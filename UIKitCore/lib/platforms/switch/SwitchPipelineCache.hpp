#pragma once

#include "include/gpu/graphite/PersistentPipelineStorage.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include <mutex>
#include <string>
#include <vector>

namespace skgpu::graphite { class Context; struct ContextOptions; }

namespace NXKit::Internal {

class SwitchPipelineCache final : public skgpu::graphite::PersistentPipelineStorage {
public:
    SwitchPipelineCache();
    void configure(skgpu::graphite::ContextOptions&);
    void prewarm(skgpu::graphite::Context*, VkFormat);
    void saveKeys();
    sk_sp<SkData> load() override;
    void store(const SkData&) override;
private:
    std::string directory;
    std::mutex mutex;
    std::vector<sk_sp<SkData>> keys;
    size_t keysBytes = 0;
    sk_sp<SkData> loadedCache;
    bool hasPersistentCache = false;
    void remember(sk_sp<SkData>);
};

} // namespace NXKit::Internal
