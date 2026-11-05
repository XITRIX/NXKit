#import "platforms/apple/MetalPipelineDescription.hpp"
#include <iostream>
using namespace NXKit::Internal;
int main(){@autoreleasepool {
    // Every vertex format emitted by Graphite's MtlGraphicsPipeline translator.
    // Large-title text first exercised UShort4Normalized in the live harvest.
    const MTLVertexFormat formats[]={MTLVertexFormatFloat,MTLVertexFormatFloat2,MTLVertexFormatFloat3,MTLVertexFormatFloat4,
        MTLVertexFormatHalf,MTLVertexFormatHalf2,MTLVertexFormatHalf4,MTLVertexFormatInt2,MTLVertexFormatInt3,MTLVertexFormatInt4,
        MTLVertexFormatUInt2,MTLVertexFormatChar,MTLVertexFormatChar2,MTLVertexFormatChar4,MTLVertexFormatUChar,
        MTLVertexFormatUChar2,MTLVertexFormatUChar4,MTLVertexFormatUCharNormalized,MTLVertexFormatUChar4Normalized,
        MTLVertexFormatShort2,MTLVertexFormatShort4,MTLVertexFormatUShort2,MTLVertexFormatUShort2Normalized,
        MTLVertexFormatInt,MTLVertexFormatUInt,MTLVertexFormatUShortNormalized,MTLVertexFormatUShort4Normalized};
    for(auto format:formats)if(metalEnum<MTLVertexFormat,metalVertexFormat>(metalVertexFormat(format))!=format)return 1;
    if(![metalVertexFormat(MTLVertexFormatUShort4Normalized)isEqualToString:@"UShort4Normalized"])return 1;
    try{metalEnum<MTLVertexFormat,metalVertexFormat>(@"UnknownFormat");return 1;}catch(const std::runtime_error&){}
    std::cout<<"Metal pipeline vertex formats passed (27 formats)\n";return 0;
}}
