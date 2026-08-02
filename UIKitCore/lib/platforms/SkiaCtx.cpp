#include "SkiaCtx.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include <cstdint>
#include <list>
#include <thread>
#include <unordered_map>

using namespace NXKit;

std::shared_ptr<SkiaCtx> SkiaCtx::_main;

namespace {

class NXKitImageProvider final : public skgpu::graphite::ImageProvider {
public:
    sk_sp<SkImage> findOrCreate(skgpu::graphite::Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties requiredProperties) override {
        if (!recorder || !image) {
            return nullptr;
        }

        const uint64_t key = (static_cast<uint64_t>(image->uniqueID()) << 1) |
                             static_cast<uint64_t>(requiredProperties.fMipmapped);
        if (auto found = cache.find(key); found != cache.end()) {
            recency.splice(recency.begin(), recency, found->second.recency);
            return found->second.image;
        }

        auto graphiteImage =
                SkImages::TextureFromImage(recorder, image, requiredProperties);
        if (!graphiteImage) {
            return nullptr;
        }

        recency.push_front(key);
        cache.emplace(key, Entry{graphiteImage, recency.begin()});
        if (cache.size() > kMaximumCachedImages) {
            const uint64_t expiredKey = recency.back();
            recency.pop_back();
            cache.erase(expiredKey);
        }
        return graphiteImage;
    }

    void clear() {
        // Graphite-backed images retain texture proxies owned by the recorder's
        // resource provider. Release them while the recorder and context are
        // still alive instead of from ImageProvider's member destructor.
        cache.clear();
        recency.clear();
    }

private:
    static constexpr size_t kMaximumCachedImages = 256;

    struct Entry {
        sk_sp<SkImage> image;
        std::list<uint64_t>::iterator recency;
    };

    std::list<uint64_t> recency;
    std::unordered_map<uint64_t, Entry> cache;
};

} // namespace

std::unique_ptr<skgpu::graphite::Recorder> SkiaCtx::createGraphiteRecorder(
        skgpu::graphite::Context* context) const {
    if (!context) {
        return nullptr;
    }

    skgpu::graphite::RecorderOptions options;
    options.fImageProvider = sk_make_sp<NXKitImageProvider>();
    return context->makeRecorder(options);
}

void SkiaCtx::clearGraphiteImageCache(skgpu::graphite::Recorder* recorder) const {
    if (!recorder) {
        return;
    }

    auto* imageProvider =
            dynamic_cast<NXKitImageProvider*>(recorder->clientImageProvider());
    if (imageProvider) {
        imageProvider->clear();
    }
}

SkString SkiaCtx::getDefaultFontFamilyName() const {
    SkString familyName;
    if (!fontMgr) {
        return familyName;
    }

    if (auto typeface = fontMgr->matchFamilyStyle(nullptr, SkFontStyle())) {
        typeface->getFamilyName(&familyName);
        return familyName;
    }

    if (fontMgr->countFamilies() > 0) {
        fontMgr->getFamilyName(0, &familyName);
    }
    return familyName;
}

sk_sp<SkTypeface> SkiaCtx::getDefaultTypeface(const SkFontStyle& style) const {
    if (!fontMgr) {
        return nullptr;
    }

    if (auto typeface = fontMgr->matchFamilyStyle(nullptr, style)) {
        return typeface;
    }

    // Custom-data managers enumerate their fonts but may not expose a
    // null-named default family. The first enumerated family is the manager's
    // own fallback in that case, so resolve it directly through its style set.
    if (fontMgr->countFamilies() > 0) {
        auto styles = fontMgr->createStyleSet(0);
        if (styles && styles->count() > 0) {
            return styles->matchStyle(style);
        }
    }
    return nullptr;
}

void SkiaCtx::flushAndSubmit(sk_sp<SkSurface> surface) {
    auto* context = graphiteContext();
    auto* recorder = graphiteRecorder();
    if (!surface || !context || !recorder) {
        return;
    }

    auto recording = recorder->snap();
    if (!recording) {
        SkDebugf("Graphite failed to snap the frame recording.\n");
        return;
    }

    skgpu::graphite::InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    auto status = context->insertRecording(insertInfo);
    if (status != skgpu::graphite::InsertStatus::kSuccess) {
        SkDebugf("Graphite failed to insert the frame recording: %s\n",
                 status.message().c_str());
        return;
    }

    if (!context->submit(skgpu::graphite::SyncToCpu::kNo)) {
        SkDebugf("Graphite failed to submit the frame.\n");
    }
}

void SkiaCtx::sleepForNextFrame() {
    auto _screenFrameRate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    auto frameTime = int(1000.f / _screenFrameRate - currentRunLoopStartTimer.getElapsedTimeInMilliseconds());
    printf("Frame time: %d\n", frameTime);
    if (frameTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
    }
}
