#pragma once
#include <string_view>
namespace NXKit::Internal {
// Independent translation of the shipped QuartzCore SDF displacement and
// foreground AIR. No Apple shader binary is linked into the portable kit.
inline std::string_view nativeLensShaderFunctions() { return R"sksl(
float3 lensShape(float2 p,float2 size) {
    return roundedBoxSample(p-size*.5,size*.5,min(size.x,size.y)*.5);
}
float lensShoulder(float depth,float height,float offset) {
    // Native CPU packs1/height. Avoid0*infinity at a zero-height boundary.
    float t=height>0?clamp((depth-offset)/height,0,1):step(offset,depth);
    return 1.0-sqrt(max(t*(2.0-t),0));
}
float2 lensDisplaced(float2 p,float2 size,float amount,float height,float curvature,float ovalization) {
    if(amount==0 || height<=0)return p;
    float3 field=lensShape(p,size);float depth=-field.x;
    float width=max((abs(field.y)+abs(field.z))/max(contentScale,1),.0001);
    float t=clamp(depth/height,0,1);
    float flatBand=t<1?.292899996:0;
    float strength=mix(flatBand,1-sqrt(max(t*(2-t),0)),curvature);
    float endpoint=clamp((height-depth)/width+.5,0,1);
    float2 delta=p-size*.5;
    float2 radial=float2(delta.x,delta.y*size.x/max(size.y,.0001));
    float2 normal=field.yz;
    if(length(radial)>.000001) {
        float2 blended=mix(normal,normalize(radial),ovalization);
        if(length(blended)>.000001)normal=normalize(blended);
    }
    return p+normal*(amount*strength*endpoint);
}
float lensEdgeOpacity(float sdf,float4 edge) {
    float t=abs(edge.y-edge.x)>.000001?clamp((sdf-edge.x)/(edge.y-edge.x),0,1):step(edge.x,sdf);
    return 1-mix(edge.z,edge.w,t);
}
float2 lensAberration(float2 normal,float angle,float2 sourceExtent) {
    // SDF textureY and viewY differ. Transform the normal to textureY,
    // apply the AIR direction algebra, then transform the sample back.
    float c=cos(angle),s=sin(angle);
    // AIR swaps after UV normalization, so swapping point-space axes alone
    // loses the source allocation's aspect. Preserve that operation order.
    return float2(normal.y*c+normal.x*s,normal.x*c-normal.y*s)
        *sourceExtent/max(sourceExtent.yx,float2(.0001));
}
float2 lensForegroundExtent(float2 size,float margin) {
    // Portable capture-context policy inferred from independently varied
    // native margin controls. This models virtual padding without allocating
    // transparent pixels. Native allocator tile dimensions remain private.
    // Native foreground capture scale remains1 in the2x/3x controls.
    // Screen backing scale therefore must not change this optical aspect.
    return size+float2(sqrt(2)*margin);
}
)sksl"; }
} // namespace NXKit::Internal
