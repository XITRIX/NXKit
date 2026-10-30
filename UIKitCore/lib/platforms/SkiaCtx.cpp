#include "SkiaCtx.h"
#include "GlassBackdrop.hpp"
#include "include/core/SkImage.h"
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

void SkiaCtx::readBackdrop(sk_sp<SkImage> image, BackdropCompletion completion) {
    _backdropReads.push_back({std::move(image), std::move(completion)});
}

void SkiaCtx::submitBackdropReads() {
    auto reads = std::move(_backdropReads);
    _backdropReads.clear();
    auto* context = graphiteContext();
    for (auto& read : reads) {
        if (!context || !read.image) {
            read.completion(std::nullopt);
            continue;
        }
        // Keep the prefix snapshot alive until the transfer completes. Only
        // 64 RGBA pixels cross to the CPU; there is no synchronous GPU wait.
        auto* pending = new BackdropRead(std::move(read));
        context->asyncRescaleAndReadPixels(
            pending->image.get(), SkImageInfo::MakeN32Premul(8, 8),
            SkIRect::MakeWH(pending->image->width(), pending->image->height()),
            SkImage::RescaleGamma::kSrc, SkImage::RescaleMode::kRepeatedLinear,
            [](void* opaque, std::unique_ptr<const SkImage::AsyncReadResult> pixels) {
                std::unique_ptr<BackdropRead> request(static_cast<BackdropRead*>(opaque));
                if (!pixels) {
                    request->completion(std::nullopt);
                    return;
                }
                SkPixmap pixmap(SkImageInfo::MakeN32Premul(8, 8), pixels->data(0), pixels->rowBytes(0));
                request->completion(Internal::backdropLuminance(pixmap));
            }, pending
        );
    }
}

void SkiaCtx::pollBackdropReads() {
    if (auto* context = graphiteContext()) context->checkAsyncWorkCompletion();
}

std::unique_ptr<skgpu::graphite::Recorder> SkiaCtx::createGraphiteRecorder(
        skgpu::graphite::Context* context) const {
    if (!context) {
        return nullptr;
    }

    skgpu::graphite::RecorderOptions options;
    options.fImageProvider = sk_make_sp<NXKitImageProvider>();
    return context->makeRecorder(options);
}

bool SkiaCtx::finishGraphiteWork(skgpu::graphite::Context* context,
                                 skgpu::graphite::Recorder* recorder) const {
    if (!context) {
        return false;
    }

    bool succeeded = true;
    if (recorder) {
        auto recording = recorder->snap();
        if (!recording) {
            SkDebugf("Graphite failed to snap pending shutdown work.\n");
            succeeded = false;
        } else {
            skgpu::graphite::InsertRecordingInfo insertInfo;
            insertInfo.fRecording = recording.get();
            const auto status = context->insertRecording(insertInfo);
            if (status != skgpu::graphite::InsertStatus::kSuccess) {
                SkDebugf("Graphite failed to insert pending shutdown work: %s\n",
                         status.message().c_str());
                succeeded = false;
            }
        }
    }

    if (!context->submit(skgpu::graphite::SyncToCpu::kYes)) {
        SkDebugf("Graphite failed to finish shutdown work.\n");
        succeeded = false;
    }
    return succeeded;
}

void SkiaCtx::flushAndSubmit(sk_sp<SkSurface> surface) {
    const auto cancelReads = [this] {
        auto reads = std::move(_backdropReads);
        _backdropReads.clear();
        for (const auto& read : reads) read.completion(std::nullopt);
    };
    auto* context = graphiteContext();
    auto* recorder = graphiteRecorder();
    if (!surface || !context || !recorder) {
        cancelReads();
        return;
    }

    auto recording = recorder->snap();
    if (!recording) {
        SkDebugf("Graphite failed to snap the frame recording.\n");
        cancelReads();
        return;
    }

    skgpu::graphite::InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    auto status = context->insertRecording(insertInfo);
    if (status != skgpu::graphite::InsertStatus::kSuccess) {
        SkDebugf("Graphite failed to insert the frame recording: %s\n",
                 status.message().c_str());
        cancelReads();
        return;
    }

    submitBackdropReads();
    if (!context->submit(skgpu::graphite::SyncToCpu::kNo)) {
        SkDebugf("Graphite failed to submit the frame.\n");
    }
}

void SkiaCtx::sleepForNextFrame() {
    auto _screenFrameRate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    auto frameTime = int(1000.f / _screenFrameRate - currentRunLoopStartTimer.getElapsedTimeInMilliseconds());
    if (frameTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
    }
}
