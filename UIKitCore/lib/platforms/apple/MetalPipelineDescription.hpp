#pragma once
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
// Metal translator spellings for the formats Graphite uses. Fail visibly if
// a new backend format needs to be added to the offline catalog exporter.
namespace NXKit::Internal {
inline NSString* metalPixelFormat(MTLPixelFormat value) {
#define FORMAT(x) case MTLPixelFormat##x:return @#x
    switch(value){FORMAT(Invalid);FORMAT(A8Unorm);FORMAT(R8Unorm);FORMAT(RG8Unorm);FORMAT(RGBA8Unorm);FORMAT(RGBA8Unorm_sRGB);FORMAT(BGRA8Unorm);FORMAT(BGRA8Unorm_sRGB);FORMAT(R16Float);FORMAT(RG16Float);FORMAT(RGBA16Float);FORMAT(R32Float);FORMAT(RG32Float);FORMAT(RGBA32Float);FORMAT(Depth16Unorm);FORMAT(Depth32Float);FORMAT(Stencil8);FORMAT(Depth32Float_Stencil8);default:throw std::runtime_error("Unmapped Metal pixel format in shader harvest");}
#undef FORMAT
}
inline NSString* metalVertexFormat(MTLVertexFormat value) {
#define FORMAT(x) case MTLVertexFormat##x:return @#x
    switch(value){FORMAT(Invalid);FORMAT(Float);FORMAT(Float2);FORMAT(Float3);FORMAT(Float4);FORMAT(Half);FORMAT(Half2);FORMAT(Half4);FORMAT(Int);FORMAT(Int2);FORMAT(Int3);FORMAT(Int4);FORMAT(UInt);FORMAT(UInt2);FORMAT(UInt3);FORMAT(UInt4);FORMAT(Char);FORMAT(Char2);FORMAT(Char4);FORMAT(UChar);FORMAT(UChar2);FORMAT(UChar4);FORMAT(UCharNormalized);FORMAT(UChar4Normalized);FORMAT(Short2);FORMAT(Short4);FORMAT(UShort2);FORMAT(UShort2Normalized);FORMAT(UShortNormalized);default:throw std::runtime_error("Unmapped Metal vertex format in shader harvest");}
#undef FORMAT
}
inline NSString* metalBlendFactor(MTLBlendFactor value) {
#define FACTOR(x) case MTLBlendFactor##x:return @#x
    switch(value){FACTOR(Zero);FACTOR(One);FACTOR(SourceColor);FACTOR(OneMinusSourceColor);FACTOR(SourceAlpha);FACTOR(OneMinusSourceAlpha);FACTOR(DestinationColor);FACTOR(OneMinusDestinationColor);FACTOR(DestinationAlpha);FACTOR(OneMinusDestinationAlpha);FACTOR(SourceAlphaSaturated);FACTOR(BlendColor);FACTOR(OneMinusBlendColor);FACTOR(BlendAlpha);FACTOR(OneMinusBlendAlpha);FACTOR(Source1Color);FACTOR(OneMinusSource1Color);FACTOR(Source1Alpha);FACTOR(OneMinusSource1Alpha);default:throw std::runtime_error("Unmapped Metal blend factor in shader harvest");}
#undef FACTOR
}
inline NSString* metalBlendOperation(MTLBlendOperation value) {
    switch(value){case MTLBlendOperationAdd:return @"Add";case MTLBlendOperationSubtract:return @"Subtract";case MTLBlendOperationReverseSubtract:return @"ReverseSubtract";case MTLBlendOperationMin:return @"Min";case MTLBlendOperationMax:return @"Max";}throw std::runtime_error("Unmapped Metal blend operation");
}
inline NSDictionary* metalPipelineDescription(MTLRenderPipelineDescriptor* descriptor) {
    NSString* vertex=descriptor.vertexFunction.label,*fragment=descriptor.fragmentFunction.label;
    if(vertex.length!=64||(descriptor.fragmentFunction&&fragment.length!=64))throw std::runtime_error("Metal shader harvest lacks source identifiers");
    NSMutableArray* attributes=[NSMutableArray new],*layouts=[NSMutableArray new];
    for(NSUInteger i=0;i<31;++i) {
        auto attribute=descriptor.vertexDescriptor.attributes[i];
        [attributes addObject:@{@"format":metalVertexFormat(attribute.format),@"offset":@(attribute.offset),@"buffer_index":@(attribute.bufferIndex)}];
        auto layout=descriptor.vertexDescriptor.layouts[i];
        NSString* step=layout.stepFunction==MTLVertexStepFunctionPerInstance?@"PerInstance":layout.stepFunction==MTLVertexStepFunctionConstant?@"Constant":@"PerVertex";
        [layouts addObject:@{@"step_function":step,@"step_rate":@(layout.stepRate),@"stride":@(layout.stride)}];
    }
    while(attributes.count&&[attributes.lastObject[@"format"]isEqualToString:@"Invalid"])[attributes removeLastObject];
    while(layouts.count&&[layouts.lastObject[@"stride"]unsignedIntegerValue]==0)[layouts removeLastObject];
    NSMutableArray* colors=[NSMutableArray new];
    for(NSUInteger i=0;i<8;++i){auto a=descriptor.colorAttachments[i];
        if(a.writeMask!=MTLColorWriteMaskAll&&a.writeMask!=MTLColorWriteMaskNone)throw std::runtime_error("Unmapped partial Metal color mask");
        [colors addObject:@{@"pixel_format":metalPixelFormat(a.pixelFormat),@"write_mask":a.writeMask==MTLColorWriteMaskAll?@"All":@"None",@"blending_enabled":@(a.blendingEnabled),@"rgb_blend_operation":metalBlendOperation(a.rgbBlendOperation),@"alpha_blend_operation":metalBlendOperation(a.alphaBlendOperation),@"source_rgb_blend_factor":metalBlendFactor(a.sourceRGBBlendFactor),@"destination_rgb_blend_factor":metalBlendFactor(a.destinationRGBBlendFactor),@"source_alpha_blend_factor":metalBlendFactor(a.sourceAlphaBlendFactor),@"destination_alpha_blend_factor":metalBlendFactor(a.destinationAlphaBlendFactor)}];}
    while(colors.count&&[colors.lastObject[@"pixel_format"]isEqualToString:@"Invalid"])[colors removeLastObject];
    NSMutableDictionary* result=[@{@"vertex_function":[NSString stringWithFormat:@"alias:%@#%@",vertex,descriptor.vertexFunction.name],@"vertex_descriptor":@{@"attributes":attributes,@"layouts":layouts},@"raster_sample_count":@(descriptor.rasterSampleCount),@"rasterization_enabled":@(descriptor.rasterizationEnabled),@"alpha_to_coverage_enabled":@(descriptor.alphaToCoverageEnabled),@"alpha_to_one_enabled":@(descriptor.alphaToOneEnabled),@"depth_attachment_pixel_format":metalPixelFormat(descriptor.depthAttachmentPixelFormat),@"stencil_attachment_pixel_format":metalPixelFormat(descriptor.stencilAttachmentPixelFormat),@"color_attachments":colors} mutableCopy];
    if(descriptor.fragmentFunction)result[@"fragment_function"]=[NSString stringWithFormat:@"alias:%@#%@",fragment,descriptor.fragmentFunction.name];
    return result;
}
}
namespace NXKit::Internal {
template<class Enum,NSString*(*Name)(Enum)>
inline Enum metalEnum(NSString* name) {
    // Construct the inverse once. Unknown/future SDK enum values are rejected,
    // never reinterpreted as a different pipeline state.
    static const auto values=[] {
        std::unordered_map<std::string,Enum> result;
        for(NSUInteger i=0;i<512;++i)try{result.emplace(Name(static_cast<Enum>(i)).UTF8String,static_cast<Enum>(i));}catch(const std::exception&){}
        return result;
    }();
    const auto found=values.find(name.UTF8String?:"");if(found==values.end())throw std::runtime_error("Invalid Metal pipeline catalog enum");return found->second;
}
inline MTLRenderPipelineDescriptor* metalPipelineFromDescription(NSDictionary* source,id<MTLFunction> vertex,id<MTLFunction> fragment) {
    if(!vertex)throw std::runtime_error("Metal pipeline catalog library is unavailable");
    auto result=[MTLRenderPipelineDescriptor new];result.vertexFunction=vertex;result.fragmentFunction=fragment;
    result.rasterSampleCount=[source[@"raster_sample_count"]unsignedIntegerValue];
    if(result.rasterSampleCount!=1&&result.rasterSampleCount!=2&&result.rasterSampleCount!=4&&result.rasterSampleCount!=8)throw std::runtime_error("Invalid Metal sample count");
    result.rasterizationEnabled=[source[@"rasterization_enabled"]boolValue];result.alphaToCoverageEnabled=[source[@"alpha_to_coverage_enabled"]boolValue];result.alphaToOneEnabled=[source[@"alpha_to_one_enabled"]boolValue];
    result.depthAttachmentPixelFormat=metalEnum<MTLPixelFormat,metalPixelFormat>(source[@"depth_attachment_pixel_format"]);
    result.stencilAttachmentPixelFormat=metalEnum<MTLPixelFormat,metalPixelFormat>(source[@"stencil_attachment_pixel_format"]);
    NSArray* colors=source[@"color_attachments"],*attributes=source[@"vertex_descriptor"][@"attributes"],*layouts=source[@"vertex_descriptor"][@"layouts"];
    if(colors.count>8||attributes.count>31||layouts.count>31)throw std::runtime_error("Oversized Metal pipeline catalog descriptor");
    for(NSUInteger i=0;i<colors.count;++i){NSDictionary* c=colors[i];auto a=result.colorAttachments[i];
        a.pixelFormat=metalEnum<MTLPixelFormat,metalPixelFormat>(c[@"pixel_format"]);a.writeMask=[c[@"write_mask"]isEqualToString:@"All"]?MTLColorWriteMaskAll:MTLColorWriteMaskNone;
        a.blendingEnabled=[c[@"blending_enabled"]boolValue];a.rgbBlendOperation=metalEnum<MTLBlendOperation,metalBlendOperation>(c[@"rgb_blend_operation"]);a.alphaBlendOperation=metalEnum<MTLBlendOperation,metalBlendOperation>(c[@"alpha_blend_operation"]);
        a.sourceRGBBlendFactor=metalEnum<MTLBlendFactor,metalBlendFactor>(c[@"source_rgb_blend_factor"]);a.destinationRGBBlendFactor=metalEnum<MTLBlendFactor,metalBlendFactor>(c[@"destination_rgb_blend_factor"]);a.sourceAlphaBlendFactor=metalEnum<MTLBlendFactor,metalBlendFactor>(c[@"source_alpha_blend_factor"]);a.destinationAlphaBlendFactor=metalEnum<MTLBlendFactor,metalBlendFactor>(c[@"destination_alpha_blend_factor"]);
    }
    auto descriptor=[MTLVertexDescriptor vertexDescriptor];
    for(NSUInteger i=0;i<attributes.count;++i){NSDictionary* a=attributes[i];descriptor.attributes[i].format=metalEnum<MTLVertexFormat,metalVertexFormat>(a[@"format"]);descriptor.attributes[i].offset=[a[@"offset"]unsignedIntegerValue];descriptor.attributes[i].bufferIndex=[a[@"buffer_index"]unsignedIntegerValue];if(descriptor.attributes[i].bufferIndex>=31)throw std::runtime_error("Invalid Metal vertex buffer index");}
    for(NSUInteger i=0;i<layouts.count;++i){NSDictionary* a=layouts[i];NSString* step=a[@"step_function"];if(![step isEqualToString:@"PerVertex"]&&![step isEqualToString:@"PerInstance"]&&![step isEqualToString:@"Constant"])throw std::runtime_error("Invalid Metal vertex step");descriptor.layouts[i].stepFunction=[step isEqualToString:@"PerInstance"]?MTLVertexStepFunctionPerInstance:[step isEqualToString:@"Constant"]?MTLVertexStepFunctionConstant:MTLVertexStepFunctionPerVertex;descriptor.layouts[i].stride=[a[@"stride"]unsignedIntegerValue];descriptor.layouts[i].stepRate=[a[@"step_rate"]unsignedIntegerValue];}
    result.vertexDescriptor=descriptor;return result;
}
}
