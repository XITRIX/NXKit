#include "MetalPipelineCache.hpp"
#include "MetalPipelineDescription.hpp"
#include "NXMetalShaderABI.hpp"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/core/SkTypes.h"
#include "include/core/SkExecutor.h"
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <CommonCrypto/CommonDigest.h>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>
#include <stdexcept>

namespace NXKit::Internal {
namespace {
class MetalExecutor final : public SkExecutor {
    dispatch_queue_t first=dispatch_queue_create("NXKit.MetalCompile.0",DISPATCH_QUEUE_SERIAL);
    dispatch_queue_t second=dispatch_queue_create("NXKit.MetalCompile.1",DISPATCH_QUEUE_SERIAL);
    std::atomic<unsigned> next{0};
public:
    // GCD retains queued blocks and their work independently. Do not sync
    // here: the last Context reference may be released on a compiler queue.
    ~MetalExecutor() override = default;
    void add(std::function<void()> work) override {
        dispatch_async(next.fetch_add(1)%2?first:second,^{@autoreleasepool{work();}});
    }
};
bool enabled(const char* key) { const auto* value=std::getenv(key);return value&&std::string_view(value)=="1"; }
NSString* fingerprint(std::string_view source) {
    unsigned char bytes[CC_SHA256_DIGEST_LENGTH];CC_SHA256(source.data(),static_cast<CC_LONG>(source.size()),bytes);
    char hex[CC_SHA256_DIGEST_LENGTH*2+1];for(size_t i=0;i<sizeof(bytes);++i)snprintf(hex+i*2,3,"%02x",bytes[i]);
    return [NSString stringWithUTF8String:hex];
}
NSString* environmentPath(const char* name) {
    const char* value=std::getenv(name);return value&&*value?[NSString stringWithUTF8String:value]:nil;
}
NSURL* child(NSURL* base,NSString* name) { return [base URLByAppendingPathComponent:name]; }
id<MTLBinaryArchive> loadArchive(id<MTLDevice> device,NSURL* url) {
    if(!url||![NSFileManager.defaultManager fileExistsAtPath:url.path])return nil;
    MTLBinaryArchiveDescriptor* descriptor=[MTLBinaryArchiveDescriptor new];descriptor.url=url;
    NSError* error=nil;id<MTLBinaryArchive> result=[device newBinaryArchiveWithDescriptor:descriptor error:&error];
    if(!result)SkDebugf("Metal ignored unavailable/incompatible archive: %s\n",error.localizedDescription.UTF8String);
    return result;
}
id<MTLFunction> archiveFunction(id<MTLLibrary> library,NSString* name,NSArray* archives) {
    MTLFunctionDescriptor* descriptor=[MTLFunctionDescriptor new];descriptor.name=name;descriptor.binaryArchives=archives;
    return [library newFunctionWithDescriptor:descriptor error:nil];
}
}
struct MetalPipelineCache::State {
    MetalExecutor executor;
    id<MTLDevice> device;
    NSArray<id<MTLBinaryArchive>>* readArchives;
    id<MTLBinaryArchive> writeArchive;
    NSURL* directory;
    NSURL* bundled;
    NSURL* harvest;
    dispatch_queue_t writer=dispatch_queue_create("NXKit.MetalArchive",DISPATCH_QUEUE_SERIAL);
    std::mutex librariesMutex;
    std::unordered_map<std::string,sk_cfp<CFTypeRef>> libraries;
    std::mutex pipelinesMutex;
    std::unordered_map<std::string,sk_cfp<CFTypeRef>> preparedPipelines;
    std::atomic<uint64_t> libraryHits{0},libraryCompiles{0},archiveHits{0},pipelineCompiles{0};
    std::atomic<uint64_t> failures{0};
    bool diagnostics=enabled("NXKIT_METAL_DIAGNOSTICS"), strict=enabled("NXKIT_METAL_REQUIRE_ARCHIVE_HIT");
    bool dirty=false,saveScheduled=false;
    NSUInteger captured=0;
    NSMutableDictionary<NSString*,NSDictionary*>* descriptions=[NSMutableDictionary new];
    NSMutableSet<NSString*>* sourceIDs=[NSMutableSet new];
    bool exportFailed=false;
    void write() {
        if(!dirty||!writeArchive)return;
        NSURL* base=harvest?:directory;if(!base)return;
        NSURL* temporary=child(base,@"pipeline.binary.metallib.tmp");NSError* error=nil;
        if([writeArchive serializeToURL:temporary error:&error]) {
            // POSIX rename atomically replaces an existing cache without a gap.
            NSURL* destination=child(base,@"pipeline.binary.metallib");
            if(std::rename(temporary.fileSystemRepresentation,destination.fileSystemRepresentation)==0)dirty=false;
        }
        if(dirty&&diagnostics)SkDebugf("Metal archive persistence failed: %s\n",error.localizedDescription.UTF8String?:"rename failed");
        if(harvest) {
            NSMutableArray* libraries=[NSMutableArray new];
            for(NSString* source in [sourceIDs.allObjects sortedArrayUsingSelector:@selector(compare:)])
                [libraries addObject:@{@"label":source,@"path":[NSString stringWithFormat:@"libraries/%@.metallib",source]}];
            NSDictionary* script=@{@"libraries":@{@"paths":libraries},@"pipelines":@{@"render_pipelines":descriptions.allValues},@"exportFailed":@(exportFailed)};
            [[NSJSONSerialization dataWithJSONObject:script options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:nil] writeToURL:child(harvest,@"pipelines.mtlp-json") atomically:YES];
        }
    }
};
MetalPipelineCache::MetalPipelineCache(CFTypeRef native):state(std::make_shared<State>()) {
    auto s=state;s->device=(__bridge id<MTLDevice>)native;
    // A missing backend hook is a link error with old prebuilt Skia archives.
    if(skgpu::graphite::MetalPipelineCacheVersion()!=1)throw std::runtime_error("Unsupported Metal cache backend ABI");
    NSString* custom=environmentPath("NXKIT_METAL_CACHE_DIR");
    NSURL* base=custom?[NSURL fileURLWithPath:custom isDirectory:YES]:[NSFileManager.defaultManager URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
    NSString* identity=[NSString stringWithFormat:@"%@/NXKit/Metal/%s/%llu-%@",NSBundle.mainBundle.bundleIdentifier?:@"NXKit",NXKIT_METAL_SHADER_ABI,s->device.registryID,NSProcessInfo.processInfo.operatingSystemVersionString];
    s->directory=child(base,identity);[NSFileManager.defaultManager createDirectoryAtURL:s->directory withIntermediateDirectories:YES attributes:nil error:nil];
    NSString* harvest=environmentPath("NXKIT_METAL_HARVEST_DIR");
    if(harvest) {
        s->harvest=[NSURL fileURLWithPath:harvest isDirectory:YES];
        NSDictionary* prior=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:child(s->harvest,@"manifest.json")]?:[NSData data] options:0 error:nil];
        if([prior[@"abi"]isEqualToString:@NXKIT_METAL_SHADER_ABI]) {
            NSDictionary* script=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:child(s->harvest,@"pipelines.mtlp-json")]?:[NSData data] options:0 error:nil];
            for(NSDictionary* description in script[@"pipelines"][@"render_pipelines"]) {
                NSData* encoded=[NSJSONSerialization dataWithJSONObject:description options:NSJSONWritingSortedKeys error:nil];
                s->descriptions[fingerprint({static_cast<const char*>(encoded.bytes),encoded.length})]=description;
            }
            for(NSDictionary* library in script[@"libraries"][@"paths"])[s->sourceIDs addObject:library[@"label"]];
            s->exportFailed=[script[@"exportFailed"]boolValue];
        }
        [NSFileManager.defaultManager createDirectoryAtURL:child(s->harvest,@"sources") withIntermediateDirectories:YES attributes:nil error:nil];
        NSDictionary* manifest=@{@"abi":@NXKIT_METAL_SHADER_ABI,@"languageVersion":@"metal2.3",@"device":s->device.name,@"os":NSProcessInfo.processInfo.operatingSystemVersionString};
        [[NSJSONSerialization dataWithJSONObject:manifest options:NSJSONWritingPrettyPrinted error:nil] writeToURL:child(s->harvest,@"manifest.json") atomically:YES];
    }
    NSString* bundled=environmentPath("NXKIT_METAL_BUNDLE_DIR");
    s->bundled=bundled?[NSURL fileURLWithPath:bundled isDirectory:YES]:[NSBundle.mainBundle.resourceURL URLByAppendingPathComponent:@"NXKitMetalShaders" isDirectory:YES];
    NSDictionary* manifest=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:child(s->bundled,@"manifest.json")]?:[NSData data] options:0 error:nil];
    if(![manifest[@"abi"]isEqualToString:@NXKIT_METAL_SHADER_ABI])s->bundled=nil;
    NSMutableArray* archives=[NSMutableArray new];
    if(auto archive=loadArchive(s->device,child(s->bundled,@"pipeline.binary.metallib")))[archives addObject:archive];
    NSURL* persisted=child(s->directory,@"pipeline.binary.metallib");
    if(!enabled("NXKIT_METAL_BUNDLED_ONLY"))if(auto archive=loadArchive(s->device,persisted))[archives addObject:archive];
    s->readArchives=archives;
    // Writer and reader archives are separate: disk harvesting never mutates
    // an archive that a rendering/compilation thread is currently reading.
    s->writeArchive=loadArchive(s->device,persisted);
    if(!s->writeArchive&&s->harvest)s->writeArchive=loadArchive(s->device,child(s->harvest,@"pipeline.binary.metallib"));
    if(!s->writeArchive)s->writeArchive=[s->device newBinaryArchiveWithDescriptor:[MTLBinaryArchiveDescriptor new] error:nil];
    // Prepare the native objects before the event loop starts. The catalog is
    // generic Metal state; Core never needs to instantiate a UIKit component.
    if(s->bundled) {
        NSDictionary* catalog=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:child(s->bundled,@"pipelines.json")]?:[NSData data] options:0 error:nil];
        auto function=[&](NSString* reference)->id<MTLFunction> {
            NSArray* parts=[reference componentsSeparatedByString:@"#"];
            if(parts.count!=2||![parts[0]hasPrefix:@"alias:"])return nil;
            NSString* hash=[parts[0]substringFromIndex:6];if(hash.length!=64)return nil;
            for(NSUInteger i=0;i<hash.length;++i)if(!std::isxdigit([hash characterAtIndex:i]))return nil;
            const auto key=std::string(hash.UTF8String);id<MTLLibrary> library=nil;
            if(auto existing=s->libraries.find(key);existing!=s->libraries.end())library=(__bridge id<MTLLibrary>)existing->second.get();
            else {library=[s->device newLibraryWithURL:child(child(s->bundled,@"libraries"),[hash stringByAppendingPathExtension:@"metallib"]) error:nil];if(library){library.label=hash;s->libraries.emplace(key,sk_ret_cfp((__bridge CFTypeRef)library));}}
            auto result=archiveFunction(library,parts[1],s->readArchives);result.label=hash;return result;
        };
        NSArray* descriptions=catalog[@"pipelines"][@"render_pipelines"];
        if(descriptions.count<=4096)for(NSDictionary* source in descriptions)try {
            auto descriptor=metalPipelineFromDescription(source,function(source[@"vertex_function"]),function(source[@"fragment_function"]));
            descriptor.binaryArchives=s->readArchives;
            id<MTLRenderPipelineState> pipeline=nil;
            // Metal can ignore FailOnBinaryArchiveMiss when no archive is
            // attached. Never mistake that path for an offline binary hit.
            if(s->readArchives.count) {
                pipeline=[s->device newRenderPipelineStateWithDescriptor:descriptor options:MTLPipelineOptionFailOnBinaryArchiveMiss reflection:nil error:nil];
            }
            if(!pipeline&&!s->strict) {
                // Driver updates may invalidate GPU binaries. Recreate known
                // catalog states now, before interaction, from minimum-OS IR.
                ++s->pipelineCompiles;
                pipeline=[s->device newRenderPipelineStateWithDescriptor:descriptor error:nil];
                if(pipeline) {
                    MTLRenderPipelineDescriptor* capture=[descriptor copy];
                    dispatch_async(s->writer,^{@autoreleasepool{
                        if(s->writeArchive&&[s->writeArchive addRenderPipelineFunctionsWithDescriptor:capture error:nil])s->dirty=true;
                    }});
                }
            }
            if(pipeline) {
                NSData* encoded=[NSJSONSerialization dataWithJSONObject:source options:NSJSONWritingSortedKeys error:nil];
                s->preparedPipelines.emplace(fingerprint({static_cast<const char*>(encoded.bytes),encoded.length}).UTF8String,sk_ret_cfp((__bridge CFTypeRef)pipeline));
            }
        }catch(const std::exception& error){if(s->diagnostics)SkDebugf("Metal skipped incompatible catalog entry: %s\n",error.what());}
        if(s->diagnostics)SkDebugf("Metal prepared %zu native pipelines from %lu archives.\n",s->preparedPipelines.size(),(unsigned long)s->readArchives.count);
    }
    // iOS may terminate a backgrounded process without running destructors.
    // Persist any launch-time recovery work independently of clean shutdown.
    dispatch_async(s->writer,^{@autoreleasepool{s->write();}});
}
MetalPipelineCache::~MetalPipelineCache() {save();}
void MetalPipelineCache::configure(skgpu::graphite::MtlBackendContext&,skgpu::graphite::ContextOptions& options) {
#if !defined(SK_GRAPHITE_USE_CALLBACK_EXECUTOR)
    options.fExecutor=&state->executor;
#endif
}
MetalPipelineCache::Statistics MetalPipelineCache::statistics()const {return {state->libraryHits.load(),state->libraryCompiles.load(),state->archiveHits.load(),state->pipelineCompiles.load(),state->failures.load()};}
bool MetalPipelineCache::allowsRuntimeCompilation() const{return !state->strict;}
void MetalPipelineCache::save() {auto s=state;dispatch_sync(s->writer,^{@autoreleasepool{s->write();}});}
sk_cfp<CFTypeRef> MetalPipelineCache::loadLibrary(std::string_view msl) {
    auto s=state;NSString* key=fingerprint(msl);std::string hash=key.UTF8String;
    std::lock_guard lock(s->librariesMutex);
    if(auto found=s->libraries.find(hash);found!=s->libraries.end()){++s->libraryHits;return found->second;}
    if(s->bundled) {
        NSURL* path=child(child(s->bundled,@"libraries"),[key stringByAppendingPathExtension:@"metallib"]);
        if([NSFileManager.defaultManager fileExistsAtPath:path.path]) {
            auto library=[s->device newLibraryWithURL:path error:nil];
            if(library){library.label=key;++s->libraryHits;auto result=sk_ret_cfp((__bridge CFTypeRef)library);s->libraries.emplace(hash,result);return result;}
        }
    }
    if(s->strict)++s->failures;
    return nullptr;
}
void MetalPipelineCache::didCompileLibrary(std::string_view msl,CFTypeRef library) {
    auto s=state;++s->libraryCompiles;NSString* key=fingerprint(msl);
    ((__bridge id<MTLLibrary>)library).label=key;
    {std::lock_guard lock(s->librariesMutex);s->libraries.emplace(key.UTF8String,sk_ret_cfp(library));}
    if(s->harvest) {
        NSData* source=[NSData dataWithBytes:msl.data() length:msl.size()];
        dispatch_async(s->writer,^{@autoreleasepool{[source writeToURL:child(child(s->harvest,@"sources"),[key stringByAppendingPathExtension:@"metal"]) atomically:YES];}});
    }
}
sk_cfp<CFTypeRef> MetalPipelineCache::makeRenderPipeline(CFTypeRef native,CFTypeRef value,sk_cfp<CFTypeRef>* failure) {
    auto s=state;id<MTLDevice> device=(__bridge id<MTLDevice>)native;
    MTLRenderPipelineDescriptor* descriptor=(__bridge MTLRenderPipelineDescriptor*)value;
    descriptor.binaryArchives=s->readArchives;
    if(s->readArchives.count) {
        std::lock_guard lock(s->librariesMutex);
        for(bool vertex:{true,false}) {
            id<MTLFunction> original=vertex?descriptor.vertexFunction:descriptor.fragmentFunction;
            if(!original)continue;
            auto library=s->libraries.find(original.label.UTF8String?:"");
            if(library==s->libraries.end())continue;
            auto function=archiveFunction((__bridge id<MTLLibrary>)library->second.get(),original.name,s->readArchives);
            if(function){function.label=original.label;if(vertex)descriptor.vertexFunction=function;else descriptor.fragmentFunction=function;}
        }
    }
    if(s->bundled)try {
        NSDictionary* description=metalPipelineDescription(descriptor);
        NSData* encoded=[NSJSONSerialization dataWithJSONObject:description options:NSJSONWritingSortedKeys error:nil];
        std::lock_guard lock(s->pipelinesMutex);
        if(auto found=s->preparedPipelines.find(fingerprint({static_cast<const char*>(encoded.bytes),encoded.length}).UTF8String);found!=s->preparedPipelines.end()){++s->archiveHits;return found->second;}
    }catch(const std::exception&){} // Custom states still use the normal archive/fallback path.
    NSError* error=nil;id<MTLRenderPipelineState> pipeline=nil;
    if(s->readArchives.count) {
        pipeline=[device newRenderPipelineStateWithDescriptor:descriptor options:MTLPipelineOptionFailOnBinaryArchiveMiss reflection:nil error:&error];
        if(pipeline){++s->archiveHits;return sk_ret_cfp((__bridge CFTypeRef)pipeline);}
    }
    if(s->strict) {
        ++s->failures;
        if(!error)error=[NSError errorWithDomain:@"NXKit.MetalArchive" code:1 userInfo:@{NSLocalizedDescriptionKey:@"Required bundled Metal pipeline was not found"}];
        if(failure)*failure=sk_ret_cfp((__bridge CFTypeRef)error);return nullptr;
    }
    ++s->pipelineCompiles;const auto start=std::chrono::steady_clock::now();
    pipeline=[device newRenderPipelineStateWithDescriptor:descriptor error:&error];
    const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    if(s->diagnostics)SkDebugf("Metal pipeline compile %.2f ms: %s\n",elapsed,descriptor.label.UTF8String?:"unnamed");
    if(!pipeline){++s->failures;if(failure)*failure=sk_ret_cfp((__bridge CFTypeRef)error);return nullptr;}
    MTLRenderPipelineDescriptor* capture=[descriptor copy];
    dispatch_async(s->writer,^{@autoreleasepool{
        if(s->harvest) {
            try {
                NSDictionary* description=metalPipelineDescription(capture);
                NSData* encoded=[NSJSONSerialization dataWithJSONObject:description options:NSJSONWritingSortedKeys error:nil];
                s->descriptions[fingerprint({static_cast<const char*>(encoded.bytes),encoded.length})]=description;
                [s->sourceIDs addObject:capture.vertexFunction.label];if(capture.fragmentFunction)[s->sourceIDs addObject:capture.fragmentFunction.label];
            }catch(const std::exception& error){s->exportFailed=true;SkDebugf("Metal harvest failed: %s\n",error.what());}
        }
        if(s->writeArchive&&s->captured<4096&&[s->writeArchive addRenderPipelineFunctionsWithDescriptor:capture error:nil]) {
            ++s->captured;s->dirty=true;
            if(!s->saveScheduled){s->saveScheduled=true;dispatch_after(dispatch_time(DISPATCH_TIME_NOW,2*NSEC_PER_SEC),s->writer,^{@autoreleasepool{s->saveScheduled=false;s->write();}});}
        }
    }});
    return sk_ret_cfp((__bridge CFTypeRef)pipeline);
}
}
