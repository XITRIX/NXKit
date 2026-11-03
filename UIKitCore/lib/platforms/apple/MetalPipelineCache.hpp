#pragma once
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include <cstdint>
#include <memory>

namespace skgpu::graphite { struct ContextOptions; }
namespace NXKit::Internal {
// Private platform service. No component-kit dependency or UIKit public API.
class MetalPipelineCache final : public skgpu::graphite::MtlPipelineCache {
public:
    struct Statistics {
        uint64_t libraryHits, libraryCompiles, archiveHits, pipelineCompiles, failures;
    };
    explicit MetalPipelineCache(CFTypeRef device);
    ~MetalPipelineCache() override;
    void configure(skgpu::graphite::MtlBackendContext&, skgpu::graphite::ContextOptions&);
    Statistics statistics() const;
    void save();
    sk_cfp<CFTypeRef> loadLibrary(std::string_view) override;
    bool allowsRuntimeCompilation() const override;
    void didCompileLibrary(std::string_view, CFTypeRef) override;
    sk_cfp<CFTypeRef> makeRenderPipeline(CFTypeRef, CFTypeRef, sk_cfp<CFTypeRef>*) override;
private:
    struct State;
    std::shared_ptr<State> state;
};
}
