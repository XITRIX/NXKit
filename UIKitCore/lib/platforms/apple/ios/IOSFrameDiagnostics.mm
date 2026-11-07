#include "IOSFrameDiagnostics.hpp"
#include "../../../RenderPerformance.hpp"
#include <SkiaCtx.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <TargetConditionals.h>
#include <mach/mach.h>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <memory>
#include <string_view>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
struct FrameLog {
    std::mutex mutex;
    FILE* file = nullptr;
    NSString* path = nil;
    bool consoleDumped = false;
    bool geometryWritten = false;
    double nextWindow = 30, firstPresent = 0, lastPresent = 0, gpuWindowMs = 0;
    uint64_t windowPresents = 0, windowGPU = 0;
    uint64_t lastPipelineFailures = UINT64_MAX;
    uint64_t metalBytes=0,budgetedBytes=0,purgeableBytes=0;
    double memoryTime=0;
    double start = CACurrentMediaTime();
    const double exportAfter=[] {
        if(const auto lens=std::getenv("NXKIT_IOS_DEMO_LENS_REPLAY");lens&&lens[0]=='1') {
            const auto requested=std::getenv("NXKIT_IOS_DEMO_LENS_SECONDS");
            const auto parsed=requested?std::atof(requested):60.;
            return 12+(std::isfinite(parsed)?std::clamp(parsed,10.,600.):60.);
        }
        const auto value=std::getenv("NXKIT_IOS_DEMO_SCROLL_SECONDS");
        const auto resume=std::getenv("NXKIT_IOS_DEMO_IDLE_RESUME");
        return 34+(value?std::clamp(std::atof(value),12.0,600.0):12.0)+(resume&&resume[0]=='1'?27:0);
    }();
    FrameLog() {
        const auto enabled = std::getenv("NXKIT_FRAME_METRICS");
        if (!enabled || std::string_view(enabled) != "1") return;
        UIDevice.currentDevice.batteryMonitoringEnabled=YES;
        const auto directory = [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
        [NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil];
        path = [directory URLByAppendingPathComponent:@"frame-metrics.csv"].path;
        file = fopen(path.fileSystemRepresentation, "w");
        if (!file) return;
        // Profiling must not force hundreds of tiny writes per second. Keep
        // CPU/completion rows buffered and flush only at windows/export.
        setvbuf(file,nullptr,_IOFBF,256*1024);
        fprintf(file,"# max_hz=%ld,low_power=%d,thermal=%ld,os=%s\n",
            (long)UIScreen.mainScreen.maximumFramesPerSecond, NSProcessInfo.processInfo.lowPowerModeEnabled,
            (long)NSProcessInfo.processInfo.thermalState,NSProcessInfo.processInfo.operatingSystemVersionString.UTF8String);
        fprintf(file,"kind,frame,time_s,phase,total_ms,layout_ms,render_ms,submit_ms,acquire_ms,gpu_ms,presented,thermal,layers,bounded_masks,isolated_masks,preserved_mask_pixels,library_compiles,pipeline_compiles,archive_hits,bounded_lenses,bounded_lens_groups,cropped_lens_sources,lens_source_pixels,uniform_mask_prefixes\n");
        fflush(file);

    }
    ~FrameLog() { if(file) fclose(file); }
    bool active() const { return file && CACurrentMediaTime()-start < exportAfter+44; }
    void cpu(const NXKit::Internal::RenderPerformance& r) {
        if (!active()) return;
        std::lock_guard guard(mutex);
        if(r.presented) {
            metalBytes=r.metalAllocatedBytes;budgetedBytes=r.graphiteBudgetedBytes;
            purgeableBytes=r.graphitePurgeableBytes;memoryTime=r.memorySampleTime;
        }
        if(!geometryWritten) {
            geometryWritten=true;
            if(const auto context=NXKit::SkiaCtx::main()) {
                const auto size=context->getSize();
                fprintf(file,"# viewport_points=%.3fx%.3f,scale=%.3f\n",size.width,size.height,context->getScaleFactor());
            }
        }
        fprintf(file,"cpu,%llu,%.9f,%s,%.5f,%.5f,%.5f,%.5f,%.5f,,%d,%ld,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
            (unsigned long long)r.frame,CACurrentMediaTime(),NXKit::Internal::RenderPerformance::phase,
            r.totalMs,r.layoutMs,r.renderMs,r.submitMs,r.acquireMs,r.presented,
            (long)NSProcessInfo.processInfo.thermalState,(unsigned long long)r.layers,
            (unsigned long long)r.boundedMasks,(unsigned long long)r.isolatedMasks,(unsigned long long)r.preservedMaskPixels,
            (unsigned long long)r.metalLibraryCompiles,(unsigned long long)r.metalPipelineCompiles,(unsigned long long)r.metalArchiveHits,(unsigned long long)r.boundedLenses,(unsigned long long)r.boundedLensGroups,(unsigned long long)r.croppedLensSources,(unsigned long long)r.lensSourcePixels,(unsigned long long)r.uniformMaskPrefixes);
        // Optional fallback for wireless devices whose file service cannot
        // transfer app files. Dump after the finite Demo replay has settled,
        // outside every measured motion phase; no debugger attachment needed.
        const auto console=std::getenv("NXKIT_FRAME_METRICS_CONSOLE");
        if(CACurrentMediaTime()-start>=nextWindow) {
            fprintf(file,"# power_state,t=%.0f,battery=%ld,level=%.3f\n",nextWindow,(long)UIDevice.currentDevice.batteryState,UIDevice.currentDevice.batteryLevel);
            task_vm_info_data_t memory{};mach_msg_type_number_t count=TASK_VM_INFO_COUNT;
            const auto status=task_info(mach_task_self(),TASK_VM_INFO,reinterpret_cast<task_info_t>(&memory),&count);
            const auto footprint=status==KERN_SUCCESS&&count>=TASK_VM_INFO_REV1_COUNT?memory.phys_footprint:0;
            fprintf(file,"# memory_state,t=%.0f,footprint_bytes=%llu,metal_allocated_bytes=%llu,graphite_budgeted_bytes=%llu,graphite_purgeable_bytes=%llu,sample_age_s=%.3f\n",
                nextWindow,(unsigned long long)footprint,(unsigned long long)metalBytes,
                (unsigned long long)budgetedBytes,(unsigned long long)purgeableBytes,
                memoryTime>0?CACurrentMediaTime()-memoryTime:-1);
            fflush(file);
            if(console&&std::string_view(console)=="1")printf("NXKIT_FRAME_WINDOW seconds=%.0f fps=%.2f gpu_mean_ms=%.3f thermal=%ld library_compiles=%llu pipeline_compiles=%llu battery=%ld\n",
                nextWindow,lastPresent>firstPresent?(windowPresents-1)/(lastPresent-firstPresent):0,
                windowGPU?gpuWindowMs/windowGPU:0,(long)NSProcessInfo.processInfo.thermalState,
                (unsigned long long)r.metalLibraryCompiles,(unsigned long long)r.metalPipelineCompiles,(long)UIDevice.currentDevice.batteryState);
            fflush(stdout);nextWindow+=30;firstPresent=lastPresent=0;gpuWindowMs=0;windowPresents=windowGPU=0;
        }
        if (!consoleDumped && CACurrentMediaTime()-start>exportAfter) {
            consoleDumped=true;
            fflush(file);
            if(console&&std::string_view(console)=="1") {
                const auto data=[NSData dataWithContentsOfFile:path];
                printf("\nNXKIT_FRAME_METRICS_BEGIN\n");
                fwrite(data.bytes,1,data.length,stdout);
                printf("NXKIT_FRAME_METRICS_END\n");
                fflush(stdout);
            }
        }
    }
    void event(const char* kind,uint64_t frame,double time,double gpuMs,bool ok) {
        if (!active()) return;
        std::lock_guard guard(mutex);
        if(ok&&std::string_view(kind)=="present"&&time>0) {
            if(!windowPresents)firstPresent=time;
            firstPresent=std::min(firstPresent,time);lastPresent=std::max(lastPresent,time);++windowPresents;
        }
        if(ok&&std::string_view(kind)=="gpu"){gpuWindowMs+=gpuMs;++windowGPU;}
        fprintf(file,"%s,%llu,%.9f,,,,,,,%.5f,%d,,\n",kind,(unsigned long long)frame,time,gpuMs,ok);
    }
    void pass(uint64_t frame,size_t index,NSUInteger width,NSUInteger height,double vertexMs,double fragmentMs) {
        if(!active())return;
        std::lock_guard guard(mutex);
        fprintf(file,"# gpu-pass,%llu,%zu,%lu,%lu,%.6f,%.6f\n",(unsigned long long)frame,index,
            (unsigned long)width,(unsigned long)height,vertexMs,fragmentMs);
    }
    void program(uint64_t frame,size_t index,const std::string& label) {
        if(!active())return;
        std::lock_guard guard(mutex);
        fprintf(file,"# gpu-program,%llu,%zu,%s\n",(unsigned long long)frame,index,label.c_str());
    }
    void pipelineFailures(uint64_t failures) {
        if(!active())return;
        std::lock_guard guard(mutex);
        if(failures==lastPipelineFailures)return;
        lastPipelineFailures=failures;
        fprintf(file,"# pipeline_failures=%llu\n",(unsigned long long)failures);
    }
};
std::shared_ptr<FrameLog> log() { static auto value=std::make_shared<FrameLog>(); return value; }
id<MTLCommandBuffer> samplePasses(id<MTLCommandBuffer> buffer,uint64_t frame);
id<MTLCommandBuffer> observe(id<MTLCommandBuffer> buffer) {
    const auto metrics=NXKit::Internal::RenderPerformance::current;
    const auto writer=log();
    if (!buffer || !writer->active() || !metrics) return buffer;
    const auto frame=metrics->frame;
    [buffer addCompletedHandler:^(id<MTLCommandBuffer> completed) {
        const auto start=completed.GPUStartTime,end=completed.GPUEndTime;
        writer->event(start>0&&end>=start?"gpu":"gpu-unavailable",frame,start,
            start>0&&end>=start?1000*(end-start):0,completed.status==MTLCommandBufferStatusCompleted);
    }];
    return samplePasses(buffer,frame);
}
}

// Sparse, opt-in stage counters identify costly render passes on real devices.
// Normal frame diagnostics and normal app operation never allocate counters.
struct NXPassSamples {
    id<MTLCounterSampleBuffer> buffer;
    std::vector<std::pair<NSUInteger,NSUInteger>> sizes;
    std::vector<std::vector<std::string>> programs;
};
@interface NXPassDiagnosticEncoder : NSProxy {
    id<MTLRenderCommandEncoder> _encoder;
    std::shared_ptr<NXPassSamples> _samples;
    size_t _index;
}
- (instancetype)initWithEncoder:(id<MTLRenderCommandEncoder>)encoder samples:(std::shared_ptr<NXPassSamples>)samples index:(size_t)index;
@end
@implementation NXPassDiagnosticEncoder
- (instancetype)initWithEncoder:(id<MTLRenderCommandEncoder>)encoder samples:(std::shared_ptr<NXPassSamples>)samples index:(size_t)index {
    _encoder=encoder;_samples=std::move(samples);_index=index;return self;
}
- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector{return [(id)_encoder methodSignatureForSelector:selector];}
- (void)forwardInvocation:(NSInvocation*)invocation{[invocation invokeWithTarget:_encoder];}
- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)state {
    auto& labels=_samples->programs[_index];
    std::string label=state.label.UTF8String?:"unlabelled";
    for(auto& c:label)if(c=='\n'||c=='\r'||c==',')c=' ';
    if(std::find(labels.begin(),labels.end(),label)==labels.end())labels.push_back(std::move(label));
    [_encoder setRenderPipelineState:state];
}
@end
@interface NXPassDiagnosticBuffer : NSProxy {
    id<MTLCommandBuffer> _buffer;
    std::shared_ptr<NXPassSamples> _samples;
}
- (instancetype)initWithBuffer:(id<MTLCommandBuffer>)buffer samples:(std::shared_ptr<NXPassSamples>)samples;
@end
@implementation NXPassDiagnosticBuffer
- (instancetype)initWithBuffer:(id<MTLCommandBuffer>)buffer samples:(std::shared_ptr<NXPassSamples>)samples {
    _buffer=buffer;_samples=std::move(samples);return self;
}
- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector{return [(id)_buffer methodSignatureForSelector:selector];}
- (void)forwardInvocation:(NSInvocation*)invocation{[invocation invokeWithTarget:_buffer];}
- (id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor*)descriptor {
    const auto index=_samples->sizes.size();
    if(index>=256)return [_buffer renderCommandEncoderWithDescriptor:descriptor];
    MTLRenderPassDescriptor* copy=[descriptor copy];
    const auto attachment=copy.sampleBufferAttachments[0];
    if(attachment.sampleBuffer)return [_buffer renderCommandEncoderWithDescriptor:descriptor];
    attachment.sampleBuffer=_samples->buffer;
    attachment.startOfVertexSampleIndex=index*4;attachment.endOfVertexSampleIndex=index*4+1;
    attachment.startOfFragmentSampleIndex=index*4+2;attachment.endOfFragmentSampleIndex=index*4+3;
    auto texture=copy.colorAttachments[0].texture?:copy.stencilAttachment.texture;
    _samples->sizes.emplace_back(texture.width,texture.height);
    _samples->programs.emplace_back();
    return (id<MTLRenderCommandEncoder>)[[NXPassDiagnosticEncoder alloc]
        initWithEncoder:[_buffer renderCommandEncoderWithDescriptor:copy] samples:_samples index:index];
}
@end
namespace {
id<MTLCommandBuffer> samplePasses(id<MTLCommandBuffer> buffer,uint64_t frame) {
    const auto enabled=std::getenv("NXKIT_GPU_PASS_METRICS");
    if(!enabled||std::string_view(enabled)!="1"||frame%120)return buffer;
    const auto device=buffer.device;
    if(![device supportsCounterSampling:MTLCounterSamplingPointAtStageBoundary])return buffer;
    id<MTLCounterSet> timestamps=nil;
    for(id<MTLCounterSet> set in device.counterSets)if([set.name isEqualToString:MTLCommonCounterSetTimestamp]){timestamps=set;break;}
    if(!timestamps)return buffer;
    auto descriptor=[MTLCounterSampleBufferDescriptor new];descriptor.counterSet=timestamps;
    descriptor.sampleCount=1024;descriptor.storageMode=MTLStorageModeShared;
    auto samples=std::make_shared<NXPassSamples>();samples->buffer=[device newCounterSampleBufferWithDescriptor:descriptor error:nil];
    if(!samples->buffer)return buffer;
    const auto writer=log();
    [buffer addCompletedHandler:^(id<MTLCommandBuffer> completed) {
        if(completed.status!=MTLCommandBufferStatusCompleted||samples->sizes.empty())return;
        auto data=[samples->buffer resolveCounterRange:NSMakeRange(0,samples->sizes.size()*4)];
        if(data.length<samples->sizes.size()*4*sizeof(MTLCounterResultTimestamp))return;
        const auto values=static_cast<const MTLCounterResultTimestamp*>(data.bytes);
        for(size_t i=0;i<samples->sizes.size();++i) {
            const auto duration=[&](size_t first) {
                const auto a=values[first].timestamp,b=values[first+1].timestamp;
                return a!=MTLCounterErrorValue&&b!=MTLCounterErrorValue&&b>=a?double(b-a)/1e6:-1.0;
            };
            writer->pass(frame,i,samples->sizes[i].first,samples->sizes[i].second,duration(i*4),duration(i*4+2));
            for(const auto& label:samples->programs[i])writer->program(frame,i,label);
        }
    }];
    return (id<MTLCommandBuffer>)[[NXPassDiagnosticBuffer alloc]initWithBuffer:buffer samples:samples];
}
}

// Graphite owns this queue for the same lifetime as its native queue. Forward
// every operation except command-buffer creation, which only adds a completion
// observer. In particular, retainReferences and submission remain unchanged.
@interface NXFrameDiagnosticQueue : NSProxy {
    id<MTLCommandQueue> _queue;
}
- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue;
@end
@implementation NXFrameDiagnosticQueue
- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue { _queue=queue; return self; }
- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector { return [(id)_queue methodSignatureForSelector:selector]; }
- (void)forwardInvocation:(NSInvocation*)invocation { [invocation invokeWithTarget:_queue]; }
- (id<MTLCommandBuffer>)commandBufferWithDescriptor:(MTLCommandBufferDescriptor*)descriptor { return observe([_queue commandBufferWithDescriptor:descriptor]); }
- (id<MTLCommandBuffer>)commandBuffer { return observe([_queue commandBuffer]); }
- (id<MTLCommandBuffer>)commandBufferWithUnretainedReferences { return observe([_queue commandBufferWithUnretainedReferences]); }
@end

namespace NXKit::Internal {
void observeIOSPipelineFailures(std::uint64_t failures) {log()->pipelineFailures(failures);}
CFTypeRef makeIOSDiagnosticQueue(CFTypeRef queue) {
    if (!log()->active()) return CFRetain(queue);
    RenderPerformance::observer=[](const RenderPerformance& value){log()->cpu(value);};
    return CFBridgingRetain([[NXFrameDiagnosticQueue alloc] initWithQueue:(__bridge id<MTLCommandQueue>)queue]);
}
void observeIOSDrawable(CFTypeRef drawable) {
#if !TARGET_OS_SIMULATOR
    const auto writer=log();
    if (!writer->active() || !RenderPerformance::current) return;
    const auto frame=RenderPerformance::current->frame;
    [(__bridge id<CAMetalDrawable>)drawable addPresentedHandler:^(id<MTLDrawable> value){
        writer->event("present",frame,value.presentedTime,0,value.presentedTime>0);
    }];
#endif
}
}
