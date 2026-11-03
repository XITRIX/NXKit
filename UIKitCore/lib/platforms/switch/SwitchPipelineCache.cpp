#include "SwitchPipelineCache.hpp"
#include "SwitchCacheFormat.hpp"
#include "SwitchCacheFile.hpp"
#include "SwitchShaderABI.hpp"
#include "include/core/SkData.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <cerrno>

namespace NXKit::Internal {
extern const unsigned char switchBakedPipelineCache[];
extern const std::size_t switchBakedPipelineCacheSize;
extern const unsigned char switchBakedPipelineKeys[];
extern const std::size_t switchBakedPipelineKeysSize;

namespace {
using namespace SwitchCacheFormat;
constexpr std::string_view abi = NXKIT_SWITCH_SHADER_ABI;

bool bundledOnly() {
    const auto* value = std::getenv("NXKIT_SWITCH_BUNDLED_CACHE_ONLY");
    return value && std::strcmp(value, "1") == 0;
}

std::vector<uint8_t> readFile(const std::string& path) {
    auto* file = std::fopen(path.c_str(), "rb");
    // Recover the last complete file if replacement was interrupted between
    // renaming the old destination aside and installing the new one.
    if (!file) file = std::fopen((path + ".previous").c_str(), "rb");
    if (!file) return {};
    std::vector<uint8_t> bytes;
    if (std::fseek(file, 0, SEEK_END) == 0) {
        const auto length = std::ftell(file);
        if (length >= 0 && static_cast<size_t>(length) <= maximumSize + headerSize &&
            std::fseek(file, 0, SEEK_SET) == 0) {
            bytes.resize(static_cast<size_t>(length));
            if (std::fread(bytes.data(), 1, bytes.size(), file) != bytes.size()) bytes.clear();
        }
    }
    std::fclose(file);
    return bytes;
}

bool writeFile(const std::string& path, std::span<const uint8_t> bytes) {
    if (bytes.empty()) return false;
    const auto temporary = path + ".tmp";
    auto* file = std::fopen(temporary.c_str(), "wb");
    if (!file) return false;
    bool success = std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
    if (std::fflush(file) != 0) success = false;
    if (std::fclose(file) != 0) success = false;
    if (success) {
        struct FileOps {
            bool rename(const std::string& from, const std::string& to) {
                return std::rename(from.c_str(), to.c_str()) == 0;
            }
            bool remove(const std::string& file) {
                return std::remove(file.c_str()) == 0 || errno == ENOENT;
            }
            bool exists(const std::string& file) {
                std::error_code error;
                return std::filesystem::exists(file, error) && !error;
            }
        } operations;
        success = replaceSwitchCacheFile(operations, temporary, path);
    }
    if (!success) std::remove(temporary.c_str());
    return success;
}
} // namespace

SwitchPipelineCache::SwitchPipelineCache()
    : directory("sdmc:/switch/NXKit/shaders/" + std::string(abi)) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) SkDebugf("Switch shader cache directory unavailable: %s\n", error.message().c_str());
}

sk_sp<SkData> SwitchPipelineCache::load() {
    auto disk = bundledOnly() ? std::vector<uint8_t>{} : readFile(directory + "/pipeline.cache");
    auto payload = decode(disk, pipelineMagic, abi);
    hasPersistentCache = !payload.empty();
    const char* source = "SD";
    if (payload.empty()) {
        payload = decode({switchBakedPipelineCache, switchBakedPipelineCacheSize}, pipelineMagic, abi);
        source = "bundled";
    }
    SkDebugf("Switch Vulkan %s pipeline cache: %zu bytes (ABI %.12s).\n",
             source, payload.size(), abi.data());
    loadedCache = payload.empty() ? nullptr : SkData::MakeWithCopy(payload.data(), payload.size());
    return loadedCache;
}

void SwitchPipelineCache::store(const SkData& data) {
    auto bytes = encode({static_cast<const uint8_t*>(data.data()), data.size()}, pipelineMagic, abi);
    if (!writeFile(directory + "/pipeline.cache", bytes))
        SkDebugf("Switch Vulkan pipeline cache could not be saved.\n");
    else {
        hasPersistentCache = true;
        SkDebugf("Switch Vulkan saved %zu pipeline-cache bytes.\n", data.size());
    }
}

void SwitchPipelineCache::remember(sk_sp<SkData> data) {
    if (!data || !data->size() || data->size() > 1024 * 1024) return;
    std::lock_guard lock(mutex);
    if (keys.size() >= 4096 || keysBytes + data->size() > 4 * 1024 * 1024) return;
    for (const auto& key : keys)
        if (key->size() == data->size() && !std::memcmp(key->data(), data->data(), data->size())) return;
    keysBytes += data->size();
    keys.push_back(std::move(data));
}

void SwitchPipelineCache::configure(skgpu::graphite::ContextOptions& options) {
    options.fPersistentPipelineStorage = this;
    options.fPipelineCallbackContext = this;
    options.fPipelineCachingCallback = [](void* context,
            skgpu::graphite::ContextOptions::PipelineCacheOp operation, const std::string&,
            uint32_t, bool, sk_sp<SkData> data) {
        if (operation == skgpu::graphite::ContextOptions::PipelineCacheOp::kAddingPipeline)
            static_cast<SwitchPipelineCache*>(context)->remember(std::move(data));
    };
}

void SwitchPipelineCache::prewarm(skgpu::graphite::Context* context, VkFormat format) {
    using namespace skgpu::graphite;
    auto precompile = context->makePrecompileContext();
    if (!precompile) return;
    auto disk = bundledOnly() ? std::vector<uint8_t>{} : readFile(directory + "/pipeline.keys");
    auto payload = decode(disk, keysMagic, abi);
    if (payload.empty())
        payload = decode({switchBakedPipelineKeys, switchBakedPipelineKeysSize}, keysMagic, abi);
    size_t warmed = 0;
    // Validate all key lengths before creating pipelines from any of them.
    std::vector<sk_sp<SkData>> saved;
    while (!payload.empty()) {
        if (payload.size() < 8 || saved.size() >= 4096) { saved.clear(); break; }
        const auto size = read64(payload.first(8));
        payload = payload.subspan(8);
        if (!size || size > 1024 * 1024 || size > payload.size()) { saved.clear(); break; }
        saved.push_back(SkData::MakeWithCopy(payload.data(), static_cast<size_t>(size)));
        payload = payload.subspan(static_cast<size_t>(size));
    }
    size_t replayed = 0;
    for (const auto& key : saved) {
        if (bundledOnly())
            SkDebugf("Switch Vulkan replaying pipeline key %zu/%zu.\n", ++replayed, saved.size());
        if (precompile->precompile(key)) { ++warmed; remember(key); }
    }
    RenderPassProperties passes[2];
    for (auto& pass : passes)
        pass.fDstCT = format == VK_FORMAT_B8G8R8A8_UNORM ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
    passes[1].fDSFlags = DepthStencilFlags::kDepth;
    const SkBlendMode blends[] = {SkBlendMode::kSrcOver, SkBlendMode::kSrc};
    const auto shapes = static_cast<DrawTypeFlags>(DrawTypeFlags::kAnalyticRRect |
                        DrawTypeFlags::kPerEdgeAAQuad | DrawTypeFlags::kNonAAFillRect);
    PaintOptions solid;
    solid.setBlendModes(blends);
    Precompile(precompile.get(), solid, shapes, passes);
    const SkColorInfo colors[] = {SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr)};
    const SkTileMode tiles[] = {SkTileMode::kClamp};
    const sk_sp<PrecompileShader> images[] = {
        PrecompileShaders::Image(PrecompileShaders::ImageShaderFlags::kNoAlphaNoCubic, colors, tiles)};
    PaintOptions image;
    image.setShaders(images);
    image.setBlendModes(blends);
    Precompile(precompile.get(), image, shapes, passes);
    PaintOptions text;
    Precompile(precompile.get(), text,
        static_cast<DrawTypeFlags>(DrawTypeFlags::kBitmapText_Mask |
            DrawTypeFlags::kBitmapText_Color | DrawTypeFlags::kSDFText), passes);
    SkDebugf("Switch Vulkan prewarmed %zu saved pipeline keys plus common UI pipelines.\n", warmed);
}

void SwitchPipelineCache::saveKeys() {
    // An entirely warm run may not call PersistentPipelineStorage::store().
    // Install the bundled native cache on first exit even when no compilation
    // was needed, so later cache extraction does not depend on a cache miss.
    if (!hasPersistentCache && loadedCache) store(*loadedCache);
    std::vector<uint8_t> payload;
    {
        std::lock_guard lock(mutex);
        for (const auto& key : keys) {
            append64(payload, key->size());
            const auto* begin = static_cast<const uint8_t*>(key->data());
            payload.insert(payload.end(), begin, begin + key->size());
        }
    }
    if (payload.empty()) return;
    const auto bytes = encode(payload, keysMagic, abi);
    if (!writeFile(directory + "/pipeline.keys", bytes))
        SkDebugf("Switch Graphite pipeline keys could not be saved.\n");
}

} // namespace NXKit::Internal
