#include "NativeGlassBlur.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace NXKit::Internal {
std::string_view nativeGlassBlurShaderSource(){return R"sksl(
float2 nativeSampleXY(float2 origin,float2 target) {
    float2 offset=target-origin;
    return (origin+offset*min(1.0,maximumSampleRadius/max(length(offset),.0001)))*max(filterScale,1.0);
}
float2 nativeSamplingWorld(float2 xy) { return xy; }
float nativeRadiusLOD(float radius) {
    float r=max(radius,0)*1.6*max(filterScale,1)*backdropPyramidInfo.z*nativePyramidStretch;
    float minRadius=nativeMinimumBlurRadius*1.6*max(filterScale,1)*backdropPyramidInfo.z*nativePyramidStretch;
    float base=minRadius>=2?1:0;
    r*=base>.5?.5:1;
    return base+max(0.0,log2(r<2 ? 1+r*.5 : r));
}
float2 nativePyramidPosition(float2 worldXY,float level) {
    float divisor=exp2(level),width=backdropPyramidInfo.x,height=backdropPyramidInfo.y;
    float2 tile=float2(1,1);
    if(level>.5) {
        tile.x=width+3;
        // Integer LODs: sum(height/2^i + 2, i=1...level-1).
        // Pyramid dimensions are aligned to 2^lastLevel, so this closed form
        // is exact and avoids eleven conditional additions per texture sample.
        tile.y+=height*(1-2/divisor)+2*(level-1);
    }
    float2 size=float2(width,height)/divisor;
    float2 p=(worldXY*backdropPyramidInfo.z-backdropPyramidRect.xy)/divisor;
    return tile+clamp(p,float2(.5),max(size-.5,float2(.5)));
}
half4 nativePyramidAt(float2 worldXY,float lod) {
    lod=clamp(lod,0,backdropPyramidInfo.w);
    float first=floor(lod),second=min(first+1,backdropPyramidInfo.w);
    half4 low=content.eval(nativePyramidPosition(worldXY,first));
    if(fract(lod)==0 || first==second)return low;
    return mix(low,
        content.eval(nativePyramidPosition(worldXY,second)),half(fract(lod)));
}
half4 nativeAtRadius(float2 origin,float2 target,float radius) {
    float2 sampleXY=nativeSampleXY(origin,target);
    if(backdropPyramidInfo.w<0)
        return radius>0 ? content.eval(sampleXY) : sharpContent.eval(sampleXY);
    return nativePyramidAt(nativeSamplingWorld(sampleXY),nativeRadiusLOD(radius));
}
half4 nativeSharp(float2 origin,float2 target) { return nativeAtRadius(origin,target,0); }
half4 nativeBlur(float2 origin,float2 target) { return nativeAtRadius(origin,target,nativeBlurRadii.x); }
half4 nativeBleed(float2 origin,float2 target) { return nativeAtRadius(origin,target,nativeBlurRadii.z); }
half4 nativeShadow(float2 origin,float2 target) { return nativeAtRadius(origin,target,nativeBlurRadii.w); }
)sksl";}

const std::vector<std::array<NXFloat,3>>& nativeGlassPyramidTaps(bool compute) {
    // Numeric topology and coefficients independently recovered from native AIR.
    static const std::vector<std::array<NXFloat,3>> first=[] {
        constexpr NXFloat near=1.9600850343704224f,far=3.9206759929656982f;
        std::vector<std::array<NXFloat,3>> result{{0,0,.10549700260162354f}};
        for(float sign:{-1.f,1.f}) {
            result.push_back({sign*near,0,.09018599987030029f});result.push_back({0,sign*near,.09018599987030029f});
            result.push_back({sign*far,0,.05634099990129471f});result.push_back({0,sign*far,.05634099990129471f});
            for(float other:{-1.f,1.f})result.push_back({sign*near,other*near,.07709799706935883f});
        }
        return result;
    }();
    static const std::vector<std::array<NXFloat,3>> subsequent=[] {
        std::vector<std::array<NXFloat,3>> result{{0,0,.10546875f}};
        for(float sign:{-1.f,1.f}) {
            result.push_back({sign*2,0,.0902099609375f});result.push_back({0,sign*2,.0902099609375f});
            result.push_back({sign*4,0,.05633544921875f});result.push_back({0,sign*4,.05633544921875f});
            for(float other:{-1.f,1.f})result.push_back({sign*2,other*2,.07708740234375f});
        }
        return result;
    }();
    return compute?subsequent:first;
}
NXFloat nativeGlassBlurLOD(NXFloat radius,NXFloat displayScale,NXFloat captureScale,NXFloat minimumRadius) {
    if(!std::isfinite(radius)||radius<0||!std::isfinite(displayScale)||displayScale<=0
        ||!std::isfinite(captureScale)||captureScale<=0||captureScale>1
        ||!std::isfinite(minimumRadius)||minimumRadius<0)
        throw std::invalid_argument("Native blur LOD requires finite positive scales and a nonnegative radius");
    const auto base=double(minimumRadius)*1.6*double(displayScale)*double(captureScale)>=2?1:0;
    const auto r=double(radius)*1.6*double(displayScale)*double(captureScale)*(base?.5:1);
    return base+std::max(0.0,std::log2(r<2?1+r*.5:r));
}
}
