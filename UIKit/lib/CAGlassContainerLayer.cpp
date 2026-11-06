#include <UIGlassEffect.h>
#include "CAGlassContainerLayer.hpp"
#include "CABackdropEffectLayer.hpp"
#include "BackdropShader.hpp"
#include "LayerCornerGeometry.hpp"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <regex>
#include <map>
#include <tuple>
#include <stdexcept>

using namespace NXKit;
namespace {
thread_local std::vector<const CABackdropEffectLayer*> groupedLayers;
struct MaterialUniform { std::string type, name; int count; };
struct MaterialInput { const char* materialName; const char* shaderName; const char* function; };
constexpr MaterialInput materialInputs[] = {
    {"content","backdrop","groupBackdrop"},
};
const std::vector<MaterialUniform>& materialUniforms() {
    static const auto uniforms = [] {
        std::vector<MaterialUniform> result;
        const auto source = UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect().shaderSource();
        const std::regex declaration("uniform (float[234]?) ([A-Za-z0-9_]+);");
        for (auto i = std::sregex_iterator(source.begin(), source.end(), declaration); i != std::sregex_iterator(); ++i) {
            const std::string type = (*i)[1], name = (*i)[2];
            if (name == "contentScale" || name == "filterScale" || name == "externalShapeCoverage") continue;
            result.push_back({type, name, type.size() == 5 ? 1 : type.back() - '0'});
        }
        return result;
    }();
    return uniforms;
}
void replaceAll(std::string& text, const std::string& from, const std::string& to) {
    size_t at = 0;
    while ((at = text.find(from, at)) != std::string::npos) { text.replace(at, from.size(), to); at += to.size(); }
}
// Rewrite identifier tokens, rather than substrings inside other names.
std::string replaceShaderIdentifiers(const std::string& source, const std::map<std::string,std::string>& names) {
    std::string result; result.reserve(source.size());
    for (size_t i=0;i<source.size();) {
        if (std::isalpha(static_cast<unsigned char>(source[i])) || source[i]=='_') {
            size_t end=i+1;
            while(end<source.size() && (std::isalnum(static_cast<unsigned char>(source[end])) || source[end]=='_'))++end;
            const auto token=source.substr(i,end-i);const auto found=names.find(token);
            result+=found==names.end()?token:found->second;i=end;
        } else result+=source[i++];
    }
    return result;
}
std::vector<std::pair<std::string,std::string>> shaderFunctions(const std::string& source) {
    const std::regex definition("(?:^|\\n)(?:half|float|void|bool|int)[234]?[ \\t]+([A-Za-z_][A-Za-z0-9_]*)[ \\t]*\\([^;{}]*\\)[ \\t\\n]*\\{");
    std::vector<std::pair<std::string,std::string>> result;
    for(auto it=std::sregex_iterator(source.begin(),source.end(),definition);it!=std::sregex_iterator();++it) {
        const auto start=static_cast<size_t>(it->position());
        auto end=source.find('{',start)+1;int depth=1;
        while(end<source.size() && depth) {if(source[end]=='{')++depth;else if(source[end]=='}')--depth;++end;}
        if (depth) throw std::logic_error("Unterminated glass shader function");
        const std::string name=(*it)[1];if(name!="main")result.emplace_back(name,source.substr(start,end-start));
    }
    return result;
}
std::string shaderSource(size_t count, size_t first, size_t last, bool composite) {
    const auto n = std::to_string(count);
    auto source = UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect().shaderSource();
    // Shading runs in each item's actual local coordinates and with its exact
    // material. Discrete choices such as clear/regular and tintMode must never
    // pass through intermediate values at an ownership boundary.
    for (const auto& u : materialUniforms()) {
        replaceAll(source, "uniform " + u.type + " " + u.name + ";",
            "uniform " + u.type + " material_" + u.name + "[" + n + "];\n" + u.type + " " + u.name + ";");
    }
    std::string declarations = "uniform float4 inverseX[" + n + "], inverseY[" + n + "], forwardX[" + n + "], forwardY[" + n + "], geometry[" + n + "];\n"
        "uniform float4 outputBounds["+n+"];\n"
        "uniform float itemAlpha[" + n + "];\nuniform float mergeSpacing;\nfloat weights[" + n + "], distances[" + n + "];\nfloat2 mergedNormal, materialNormal;\nbool geometryBlended;\n"
        "float3 materialForwardX, materialForwardY;\nint activeMaterial;\n"
        "float2 groupXY(float2 xy) { float3 p = float3(xy / max(filterScale, 1.0), 1); return float2(dot(materialForwardX,p), dot(materialForwardY,p)) * max(filterScale,1.0); }\n";
    // Every named input samples the same original backdrop. Its blur belongs
    // to the active material, never to another member's previously drawn glass.
    std::string backdrop;
    for (const auto& input : materialInputs) {
        replaceAll(source, std::string("uniform shader ") + input.materialName + ";", "");
        replaceAll(source, std::string(input.materialName) + ".eval(", std::string(input.function) + "(");
        backdrop += std::string("half4 ") + input.function + "(float2 xy) { float2 p = backdropPyramidInfo.w < 0 ? groupXY(xy) : xy;\n";
        for (size_t i = first; i < last; ++i) {
            const auto name = std::string(input.shaderName) + std::to_string(i);
            declarations += "uniform shader " + name + ";\n";
            backdrop += "if (activeMaterial == " + std::to_string(i) + ") return " + name + ".eval(p);\n";
        }
        backdrop += "return half4(0); }\n";
    }
    if (composite) declarations += "uniform shader materialSum;\n";
    replaceAll(source, "sharpContent.eval(", "groupSharp(");
    backdrop += "half4 groupSharp(float2 xy) { return sharpContent.eval(groupXY(xy)); }\n";
    replaceAll(source,"float2 nativeSamplingWorld(float2 xy) { return xy; }",
        "float2 nativeSamplingWorld(float2 xy) { return groupXY(xy); }");
    source.insert(source.find("const float SMOOTH_EDGE_PX"), declarations + backdrop);
    // Give each shading lane direct, constant-index uniform/sampler reads.
    // Copying the entire material into mutable shader globals keeps a large
    // per-fragment state live across every branch, even for a distant member.
    // Geometry and ownership remain shared; only material evaluation is local.
    const auto materialFunctions=shaderFunctions(source);
    std::map<size_t,std::map<std::string,std::string>> materialNames;
    std::string specialized;
    if (!composite) for(size_t i=first;i<last;++i) {
        const auto index=std::to_string(i);auto& names=materialNames[i];
        for(const auto& u:materialUniforms())
            if(u.name!="interactionIntensity" && u.name!="interactionLocation")
                names[u.name]="material_"+u.name+"["+index+"]";
        names["activeMaterial"]=index;
        names["materialForwardX"]="(forwardX["+index+"].xyz)";
        names["materialForwardY"]="(forwardY["+index+"].xyz)";
        for(const auto& [name,body]:materialFunctions)names[name]=name+"_material"+index;
        for(const auto& [name,body]:materialFunctions)specialized+=replaceShaderIdentifiers(body,names)+"\n";
    }
    const std::string itemFunction = R"SKSL(
float3 itemDistanceINDEX(float2 xy) {
    float4 g = geometry[INDEX];
    float2 p = float2(dot(inverseX[INDEX].xyz, float3(xy, 1)), dot(inverseY[INDEX].xyz, float3(xy, 1))) - g.xy * 0.5;
    cornerCurve = g.w;
    cornerTop=material_cornerTop[INDEX]; cornerTopCircle=material_cornerTopCircle[INDEX];
    cornerRight=material_cornerRight[INDEX]; cornerRightCircle=material_cornerRightCircle[INDEX];
    float r = min(g.z, min(g.x, g.y) * 0.5);
    float3 surface = roundedBoxSample(p, g.xy * 0.5, r);
    float d = surface.x;
    float2 n = surface.yz;
    float2 gradient = float2(inverseX[INDEX].x * n.x + inverseY[INDEX].x * n.y,
                            inverseX[INDEX].y * n.x + inverseY[INDEX].y * n.y);
    float magnitude = max(length(gradient), 0.0001);
    return float3(d / magnitude, gradient / magnitude);
}
)SKSL";
    std::string helpers;
    for (size_t i = 0; i < count; ++i) {
        auto function = itemFunction;
        replaceAll(function, "INDEX", std::to_string(i)); helpers += function;
    }
    // Preserve the calibrated zero contour exactly. Four neighbouring field
    // evaluations supply the missing derivative of the normal-dependent blend
    // width as well as the smooth-min's non-unit gradient near a narrow neck.
    helpers += "float mergeGeometry(float2 xy, bool record) { float3 first = itemDistance0(xy); float d = first.x; mergedNormal = first.yz; if (record) { weights[0] = 1; distances[0] = d; geometryBlended = false; }\n";
    for (size_t i = 1; i < count; ++i) {
        const auto index = std::to_string(i);
        helpers += "{ float3 item = itemDistance" + index + "(xy); float other = item.x; if (record) distances[" + index + "] = other;\n"
            "float k = mergeSpacing * clamp(0.5 - 0.5 * dot(item.yz, mergedNormal / max(length(mergedNormal), 0.0001)), 0, 1);\n"
            "float h = k > 0.0001 ? clamp(0.5 + 0.5 * (other - d) / k, 0, 1) : (d <= other ? 1 : 0);\n";
        for (size_t j = 0; j < i; ++j) helpers += "if (record) weights[" + std::to_string(j) + "] *= h;\n";
        helpers += "if (record) { weights[" + index + "] = 1 - h; geometryBlended = geometryBlended || (h > 0 && h < 1); } mergedNormal = mix(item.yz, mergedNormal, h);\n"
            "d = mix(other, d, h) - k * h * (1 - h); }\n";
    }
    helpers += "return d; }\nvoid materialOwnership() {\n";
    // UIKit gives explicit regular tints ownership over neutral material,
    // independent of insertion order. Equal-priority materials retain painter
    // order. This changes color ownership only, never the merged zero contour.
    // The smooth handoff is measured in world points. Its rounded endpoints
    // avoid a visible first-derivative step at the former linear color ramp.
    for (int tintedPass = 0; tintedPass < 2; ++tintedPass) for (size_t i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        helpers += "{ float cover = smoothstep(0.0, 1.0, 1.1875 - distances[" + index + "] / 8.0);\n";
        helpers += "if (material_tintMode[" + index + "] " + (tintedPass ? "<=" : ">") + " 1.5) cover = 0;\n";
        for (size_t j = 0; j < count; ++j) helpers += "weights[" + std::to_string(j) + "] *= 1-cover;\n";
        helpers += "weights[" + index + "] += cover; }\n";
    }
    helpers += "}\n";
    source.insert(source.find("float toBrightness"), helpers);
    source.erase(source.find("half4 main(float2 filterXY)"));
    std::string main = R"SKSL(
half4 main(float2 filterXY) {
    float2 xy = filterXY / max(filterScale, 1.0);
)SKSL";
    if(!composite&&first==0&&last==count) {
        main+="bool covered=false;\n";
        for(size_t i=0;i<count;++i) {
            const auto index=std::to_string(i);
            main+="covered = covered || (xy.x>=outputBounds["+index+"].x && xy.y>=outputBounds["+index+"].y && xy.x<=outputBounds["+index+"].z && xy.y<=outputBounds["+index+"].w);\n";
        }
        main+="if(!covered) { bool blendPossible=false;\n";
        for(size_t i=0;i<count;++i) {
            const auto index=std::to_string(i);
            main+="float2 bp"+index+"=float2(dot(inverseX["+index+"].xyz,float3(xy,1)),dot(inverseY["+index+"].xyz,float3(xy,1)))-geometry["+index+"].xy*.5;\n";
            main+="float bd"+index+"=circularBoxSDF(bp"+index+",geometry["+index+"].xy*.5,min(geometry["+index+"].z,min(geometry["+index+"].x,geometry["+index+"].y)*.5))*abs(forwardX["+index+"].x);\n";
            for(size_t j=0;j<i;++j)main+="blendPossible=blendPossible || abs(bd"+index+"-bd"+std::to_string(j)+")<=mergeSpacing*"+std::to_string(count)+".0;\n";
        }
        main+="if(!blendPossible)return sharpContent.eval(filterXY); }\n";
    }
    main+=R"SKSL(
    float rawDistance = mergeGeometry(xy,true);
    float2 sceneNormal = mergedNormal / max(length(mergedNormal),0.0001);
    float sceneDistance = rawDistance;
    if (geometryBlended) {
        float epsilon = max(0.05, 0.25 / max(contentScale, 1.0));
        float dx = mergeGeometry(xy + float2(epsilon,0),false) - mergeGeometry(xy - float2(epsilon,0),false);
        float dy = mergeGeometry(xy + float2(0,epsilon),false) - mergeGeometry(xy - float2(0,epsilon),false);
        float2 gradient = float2(dx,dy) / (2*epsilon);
        float magnitude = length(gradient);
        if (magnitude > 0.0001) sceneNormal = gradient / magnitude;
        // Bound medial-axis/topology saddles without changing the field sign.
        sceneDistance = rawDistance / max(magnitude,0.05);
    }
    materialOwnership();
    half4 original = sharpContent.eval(filterXY);
    float coverage = clamp(0.5 - sceneDistance * max(contentScale,1), 0, 1);
    half4 result = original;
)SKSL";
    if (composite) {
        main += "float weight = 0;\n";
        for (size_t i=0;i<count;++i)
            main += "weight += weights["+std::to_string(i)+"] * itemAlpha["+std::to_string(i)+"];\n";
        main += "return original * half(1-weight) + materialSum.eval(filterXY); }\n";
        return source+main;
    }
    const bool partial=first!=0 || last!=count;
    if(partial) replaceAll(main,"half4 result = original;","half4 result = half4(0);");
    // Touch lighting belongs to the merged surface. Preserve source ownership
    // and transform its touch to whichever material owns the shaded point.
    main += "float groupInteraction = 0; float2 groupTouch = xy;\n";
    for (size_t i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        main += "{ float intensity = material_interactionIntensity[" + index + "] * clamp(0.5 - distances[" + index + "] * max(contentScale,1), 0, 1);\n"
            "if (intensity > groupInteraction) { groupInteraction = intensity; float3 touch = float3(material_interactionLocation[" + index + "],1); groupTouch = float2(dot(forwardX[" + index + "].xyz,touch),dot(forwardY[" + index + "].xyz,touch)); } }\n";
    }
    for (size_t i = first; i < last; ++i) {
        const auto index = std::to_string(i);
        const auto block=main.size();
        main += "if (weights[" + index + "] > 0) {\n";
        main += "float2 localXY = float2(dot(inverseX[" + index + "].xyz,float3(xy,1)),dot(inverseY[" + index + "].xyz,float3(xy,1)));\n"
            "float2 localGradient = float2(forwardX[" + index + "].x * sceneNormal.x + forwardY[" + index + "].x * sceneNormal.y, forwardX[" + index + "].y * sceneNormal.x + forwardY[" + index + "].y * sceneNormal.y);\n"
            "float localMetric = max(length(localGradient),0.0001); materialNormal = localGradient / localMetric;\n"
            "interactionIntensity = groupInteraction; interactionLocation = float2(dot(inverseX[" + index + "].xyz,float3(groupTouch,1)),dot(inverseY[" + index + "].xyz,float3(groupTouch,1)));\n"
            "float localDistance=sceneDistance/localMetric;\n"
            "if(nativeGlassOutputCoverage(localDistance)>0 || coverage>0) {\n"
            // Shift the union itself. A normal-dot-offset shortcut changes
            // shadow curvature and creates seams during joining/separation.
            "half4 exterior=half4(0); float ringDistance=localDistance;\n"
            "if(nativeRingShadow.w>0 && nativeRingShadow.y>0) {\n"
            "float2 ringOffset=float2(materialForwardX.y,materialForwardY.y)*nativeRingShadowOffset;\n"
            "ringDistance=mergeGeometry(xy-ringOffset,false)/localMetric; }\n"
            "float shadowDistance=localDistance;\n"
            "if(coverage<1 && (nativeShadowShape.w>0 || nativeShadowResponse.w>0)) {\n"
            "float2 shadowOffset=float2(dot(materialForwardX.xy,nativeShadowOffset),dot(materialForwardY.xy,nativeShadowOffset));\n"
            "shadowDistance=mergeGeometry(xy-shadowOffset,false)/localMetric; }\n"
            "half4 color=nativeGlassSurface(localXY,localDistance,materialNormal,sceneNormal,min(cornerRadius,min(lensSize.x,lensSize.y)*.5),shadowDistance,ringDistance,original,coverage,contentScale*localMetric);\n"
            "result += half(weights[" + index + "] * itemAlpha[" + index + "]) * (color-original); } }\n";
        main.replace(block,main.size()-block,replaceShaderIdentifiers(main.substr(block),materialNames.at(i)));
    }
    if(partial) {
        replaceAll(main,"(color-original)","color");
        for(size_t i=first;i<last;++i) {
            const auto weight="half(weights["+std::to_string(i)+"] * itemAlpha["+std::to_string(i)+"])";
            replaceAll(main,"result += "+weight+" * color; } }",
                "result += "+weight+" * color; } else result += "+weight+" * original; }");
        }
    }
    main += "return result; }\n";
    return source + specialized + main;
}
sk_sp<SkRuntimeEffect> groupProgram(size_t count,size_t first,size_t last,bool composite) {
    using Key=std::tuple<size_t,size_t,size_t,bool>;
    static thread_local std::map<Key,sk_sp<SkRuntimeEffect>> programs;
    const Key key{count,first,last,composite};
    if(auto it=programs.find(key);it!=programs.end()) return it->second;
    auto compiled=SkRuntimeEffect::MakeForShader(SkString(shaderSource(count,first,last,composite)));
    if(!compiled.effect) throw std::runtime_error("Glass container shader: "+std::string(compiled.errorText.c_str()));
    if(programs.size()>=64) programs.erase(programs.begin());
    programs.emplace(key,compiled.effect);
    return compiled.effect;
}

}

struct CAGlassContainerLayer::Item {
    std::shared_ptr<CABackdropEffectLayer> layer;
    SkMatrix matrix, inverse;
    NXFloat alpha;
    std::shared_ptr<const BackdropEffect> material;
};
CAGlassContainerLayer::CAGlassContainerLayer(const UIGlassContainerEffect& effect): _effect(effect) {}
CAGlassContainerLayer::CAGlassContainerLayer(CAGlassContainerLayer* layer):
    CALayer(layer), _effect(layer->_effect), _runtime(layer->_runtime), _runtimeCount(layer->_runtimeCount) {}
std::shared_ptr<CALayer> CAGlassContainerLayer::copy() { return new_shared<CAGlassContainerLayer>(this); }
void CAGlassContainerLayer::setEffect(const UIGlassContainerEffect& effect) {
    _effect = effect;
    if (auto active = presentation()) std::static_pointer_cast<CAGlassContainerLayer>(active)->_effect = effect;
    setNeedsDisplay();
}
bool CAGlassContainerLayer::contains(const CABackdropEffectLayer* layer) {
    return std::find(groupedLayers.begin(), groupedLayers.end(), layer) != groupedLayers.end();
}
void CAGlassContainerLayer::renderSublayers(SkCanvas* canvas) {
    // Traversal-scoped suppression, never persistent child flags: removal,
    // reparenting and nested containers cannot leave a material suppressed.
    const auto previousSize = groupedLayers.size();
    groupedLayers.insert(groupedLayers.end(), _renderedItems.begin(), _renderedItems.end());
    struct Restore { size_t size; ~Restore() { groupedLayers.resize(size); } } restore{previousSize};
    CALayer::renderSublayers(canvas);
    _renderedItems.clear();
}
void CAGlassContainerLayer::collect(const std::shared_ptr<CALayer>& model, const SkMatrix& parent,
                                   float alpha, std::vector<Item>& items) {
    const auto layer = model->presentationOrSelf();
    if (layer->isHidden() || layer->opacity() < 0.001f || layer->mask()
        || dynamic_cast<CAGlassContainerLayer*>(layer.get())) return;
    const auto glass = std::dynamic_pointer_cast<CABackdropEffectLayer>(layer);
    // A wrapper mask is an independent compositing boundary. Keep its usual
    // rendering rather than letting a union bridge leak outside that mask.
    if (layer->masksToBounds() && !glass) return;
    const auto transform3D = layer->transform();
    if (transform3D.m13 != 0 || transform3D.m23 != 0 || transform3D.m31 != 0 || transform3D.m32 != 0
        || transform3D.m14 != 0 || transform3D.m24 != 0 || transform3D.m34 != 0 || transform3D.m44 != 1) return;
    const auto transform = transform3D.toSkM44().asM33();
    if (transform.hasPerspective() || !transform.isFinite()) return;
    auto matrix = parent;
    matrix.preTranslate(layer->position().x, layer->position().y);
    matrix.preConcat(transform);
    const auto size = layer->bounds().size;
    matrix.preTranslate(-size.width * layer->anchorPoint().x, -size.height * layer->anchorPoint().y);
    matrix.preConcat(NXTransform3DMakeAffineTransform(layer->renderingTransform()).toSkM44().asM33());
    SkMatrix inverse;
    if (!matrix.isFinite() || !matrix.invert(&inverse)) return;
    alpha *= layer->opacity();
    if (glass && glass->_glassMaterial && std::isfinite(size.width) && std::isfinite(size.height)
        && size.width > 0 && size.height > 0) {
        items.push_back({glass, matrix, inverse, alpha,
            glass->resolvedEffect(size)});
    }
    matrix.preTranslate(-layer->bounds().origin.x, -layer->bounds().origin.y);
    auto children = layer->sublayers();
    std::stable_sort(children.begin(), children.end(), [](const auto& a, const auto& b) { return a->zPosition() < b->zPosition(); });
    for (const auto& child : children) collect(child, matrix, alpha, items);
}
void CAGlassContainerLayer::draw(SkCanvas* canvas) {
    _renderedItems.clear();
    if (canvas->getTotalMatrix().hasPerspective()) return;
    std::vector<Item> items;
    auto parent = SkMatrix::Translate(-bounds().origin.x, -bounds().origin.y);
    auto children = sublayers();
    std::stable_sort(children.begin(), children.end(), [](const auto& a, const auto& b) { return a->zPosition() < b->zPosition(); });
    for (const auto& child : children) collect(child, parent, 1, items);
    if (items.size() < 2) return;
    // Native group captures share the largest neutral regular member's
    // material context, including disconnected members. Resolve that context
    // without changing each member's actual lens bounds, radius or transform.
    NXFloat sharedMaterialSize = 0;
    for (const auto& item : items) {
        const auto& effect = *item.layer->_glassMaterial;
        if (effect.style() == UIGlassEffect::Style::regular && !effect.tintColor()) {
            const auto size = item.layer->bounds().size;
            sharedMaterialSize = std::max(sharedMaterialSize,std::min(size.width,size.height));
        }
    }
    for (auto& item : items) {
        const auto& effect = *item.layer->_glassMaterial;
        if (effect.style() == UIGlassEffect::Style::regular && !effect.tintColor())
            item.material = item.layer->resolvedEffect({sharedMaterialSize,sharedMaterialSize});
    }
    if (!_runtime || _runtimeCount != items.size()) {
        // Each material samples one packed pyramid (or Gaussian override) and
        // the original backdrop. Keep the established three-program batch
        // bound to limit generated shader size and stay below 16 samplers.
        // Larger groups share geometry but shade in batches of three, then sum
        // their weighted colors from the same original backdrop.
        _runtime=groupProgram(items.size(),0,items.size()>3?0:items.size(),items.size()>3);
        _runtimeCount = items.size();
    }
    SkRuntimeEffectBuilder builder(_runtime);
    NXFloat scale = std::isfinite(contentsScale()) ? std::max(1.0f, contentsScale()) : 1;
    NXFloat maximumRadius = 0, outputMargin = 0, samplingReach = 0;
    std::vector<float> inverseX, inverseY, forwardX, forwardY, geometry, alphas;
    SkRect area = SkRect::MakeEmpty();
    std::vector<std::string> childNames;
    std::vector<sk_sp<SkImageFilter>> inputs;
    std::map<std::tuple<NXFloat,NXFloat,NXFloat,NXFloat,NXFloat,NXFloat>,sk_sp<SkImageFilter>> kernels;
    std::map<NXFloat,sk_sp<SkImageFilter>> captures;
    std::map<NXFloat,NXFloat> pyramidRadii;
    std::map<NXFloat,NXFloat> pyramidMinima;
    std::map<NXFloat,SkRect> pyramidDomains;
    std::map<NXFloat,Internal::BackdropPyramidImage> sharedPyramids;
    std::vector<Internal::BackdropPyramidImage> itemPyramids(items.size());
    std::vector<NXFloat> itemStretch;
    SkRect pyramidDomain=SkRect::MakeEmpty(); NXFloat pyramidMargin=0;
    for(const auto& item:items) {
        const auto& m=item.matrix;
        const double a=m.getScaleX(),b=m.getSkewX(),c=m.getSkewY(),d=m.getScaleY();
        const double norm=a*a+b*b+c*c+d*d,det=a*d-b*c;
        const auto stretch=static_cast<NXFloat>(std::sqrt((norm+std::sqrt(std::max(0.0,norm*norm-4*det*det)))*.5));
        itemStretch.push_back(stretch);
        pyramidDomain.join(m.mapRect(SkRect::MakeWH(item.layer->bounds().width(),item.layer->bounds().height())));
        pyramidMargin=std::max(pyramidMargin,(item.material->maximumSampleRadius()+item.material->outputOutset())*stretch+2);
        if(item.material->backdropPyramid()) {
            auto [domain,insertedDomain]=pyramidDomains.try_emplace(item.material->backdropScale(),SkRect::MakeEmpty());
            domain->second.join(m.mapRect(SkRect::MakeWH(item.layer->bounds().width(),item.layer->bounds().height())));
            auto& maxRadius=pyramidRadii[item.material->backdropScale()];
            maxRadius=std::max(maxRadius,item.material->backdropPyramid()->maximumRadius*stretch);
            const auto minimum=item.material->uniform("nativeMinimumBlurRadius")->front()*stretch;
            auto [entry,inserted]=pyramidMinima.try_emplace(item.material->backdropScale(),minimum);
            if(!inserted)entry->second=std::min(entry->second,minimum);
        }
    }
    pyramidDomain.outset(pyramidMargin,pyramidMargin);
    pyramidDomain=SkRect::MakeLTRB(pyramidDomain.left()*scale,pyramidDomain.top()*scale,
        pyramidDomain.right()*scale,pyramidDomain.bottom()*scale);
    for(auto& [density,domain]:pyramidDomains) {
        // A material only owns pixels near its own members. Bound the atlas
        // by those members, retaining the optical reach, all polynomial-union
        // expansion, and the ownership ramp's 1.1875 * 8 point support. The
        // capture lattice remains anchored in group coordinates, so distant
        // materials no longer force an otherwise identical full-width atlas.
        const auto padding=pyramidMargin+_effect.spacing()*items.size()+1.1875f*8;
        domain.outset(padding,padding);
        domain=SkRect::MakeLTRB(domain.left()*scale,domain.top()*scale,domain.right()*scale,domain.bottom()*scale);
        domain.intersect(pyramidDomain);
    }
    sk_sp<SkImageFilter> sharedPrefix;
    if(pyramidRadii.size()>1&&Internal::BackdropShader::canFreezePyramid(canvas,scale)) {
        auto inputBounds=SkRect::MakeEmpty();
        for(const auto& [density,radius]:pyramidRadii) {
            const auto member=std::find_if(items.begin(),items.end(),[&](const auto& item) {
                return item.material->backdropPyramid()&&item.material->backdropScale()==density;
            });
            auto description=*member->material;
            description.setBackdropPyramid(radius,description.backdropPyramid()->taps,
                1.6f*pyramidMinima.at(density),description.backdropPyramid()->initialTaps);
            const auto plan=Internal::BackdropShader::makePyramid(description,pyramidDomains.at(density),scale);
            auto region=pyramidDomains.at(density);region.outset(plan.sourceReach,plan.sourceReach);inputBounds.join(region);
        }
        // All lanes see the same pre-group pixels. Copy their shared source
        // once, independently of each lane's reduction density and atlas.
        sharedPrefix=Internal::BackdropShader::freezePrefix(canvas,inputBounds,scale);
    }
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        const auto& m = item.inverse;
        const auto& layer = item.layer;
        const auto& forward = item.matrix;
        const auto size = layer->bounds().size;
        inverseX.insert(inverseX.end(), {m.getScaleX(), m.getSkewX(), m.getTranslateX(), 0});
        inverseY.insert(inverseY.end(), {m.getSkewY(), m.getScaleY(), m.getTranslateY(), 0});
        forwardX.insert(forwardX.end(), {forward.getScaleX(), forward.getSkewX(), forward.getTranslateX(), 0});
        forwardY.insert(forwardY.end(), {forward.getSkewY(), forward.getScaleY(), forward.getTranslateY(), 0});
        geometry.insert(geometry.end(), {size.width, size.height,
            std::isfinite(layer->cornerRadius()) ? std::max(0.0f, layer->cornerRadius()) : 0,
            layer->cornerCurve() == CALayerCornerCurve::continuous ? 1.0f : 0.0f});
        alphas.push_back(item.alpha);
        area.join(item.matrix.mapRect(SkRect::MakeWH(size.width, size.height)));
        const double a = forward.getScaleX(), b = forward.getSkewX(), c = forward.getSkewY(), d = forward.getScaleY();
        const double squareNorm = a*a+b*b+c*c+d*d, determinant = a*d-b*c;
        const auto stretch = std::sqrt((squareNorm + std::sqrt(std::max(0.0,squareNorm*squareNorm-4*determinant*determinant)))*.5);
        maximumRadius = std::max(maximumRadius, static_cast<NXFloat>(item.material->maximumSampleRadius() * stretch));
        outputMargin = std::max(outputMargin, static_cast<NXFloat>(item.material->outputOutset()*stretch));
        for (const auto& input : materialInputs) {
            if(item.material->backdropPyramid()) {
                const auto captureScale=item.material->backdropScale();
                auto atlas=sharedPyramids.find(captureScale);
                if(atlas==sharedPyramids.end()) {
                    auto description=*item.material;
                    description.setBackdropPyramid(pyramidRadii[captureScale],description.backdropPyramid()->taps,
                        1.6f*pyramidMinima.at(captureScale),description.backdropPyramid()->initialTaps);
                    atlas=sharedPyramids.emplace(captureScale,Internal::BackdropShader::makePyramid(description,pyramidDomains.at(captureScale),scale,canvas,sharedPrefix,true)).first;
                }
                itemPyramids[i]=atlas->second;
                maximumRadius=std::max(maximumRadius,atlas->second.sampleReach/scale);
                samplingReach=std::max(samplingReach,item.material->maximumSampleRadius()*static_cast<NXFloat>(stretch)
                    +atlas->second.sourceReach/scale);
                inputs.push_back(atlas->second.image);
                childNames.push_back(std::string(input.shaderName)+std::to_string(i));
                continue;
            }
            itemPyramids[i].info={0,0,1,-1};
            auto blurKernel = item.material->backdropBlurKernel();
            if (std::string_view(input.materialName) != item.material->backdropShaderName()) {
                const auto entry = item.material->backdropInputKernels().find(input.materialName);
                if (entry == item.material->backdropInputKernels().end())
                    throw std::logic_error(std::string("Missing grouped glass backdrop input ") + input.materialName);
                blurKernel = entry->second;
            }
            const auto scaleX = scale*std::hypot(forward.getScaleX(),forward.getSkewX());
            const auto scaleY = scale*std::hypot(forward.getSkewY(),forward.getScaleY());
            samplingReach = std::max(samplingReach,static_cast<NXFloat>(
                (item.material->maximumSampleRadius()+3*std::max(blurKernel.sigma0,blurKernel.sigma1))*stretch));
            const auto samplingScale=item.material->backdropScale();
            const auto key=std::tuple{blurKernel.sigma0,blurKernel.sigma1,blurKernel.weight,scaleX,scaleY,samplingScale};
            auto kernel=kernels.find(key);
            if(kernel==kernels.end()) {
                auto capture=captures.find(samplingScale);
                if(capture==captures.end()) capture=captures.emplace(samplingScale,Internal::BackdropShader::capture(samplingScale)).first;
                auto filter=Internal::BackdropShader::filteredInput(blurKernel,scaleX,scaleY,
                    samplingScale,capture->second);
                kernel=kernels.emplace(key,std::move(filter)).first;
            }
            inputs.push_back(kernel->second);
            childNames.push_back(std::string(input.shaderName)+std::to_string(i));
        }
        canvas->save(); canvas->concat(item.matrix); layer->observeBackdrop(canvas); canvas->restore();
    }
    for (const auto& u : materialUniforms()) {
        std::vector<float> values;
        values.reserve(items.size() * u.count);
        for (size_t itemIndex=0;itemIndex<items.size();++itemIndex) {
            const auto& item=items[itemIndex];
            const auto size = item.layer->bounds().size;
            const auto center = SkPoint::Make(size.width * 0.5f, size.height * 0.5f);
            if(u.name=="backdropPyramidRect")values.insert(values.end(),itemPyramids[itemIndex].rect.begin(),itemPyramids[itemIndex].rect.end());
            else if(u.name=="backdropPyramidInfo")values.insert(values.end(),itemPyramids[itemIndex].info.begin(),itemPyramids[itemIndex].info.end());
            else if(u.name=="nativePyramidStretch")values.push_back(itemStretch[itemIndex]);
            else if(u.name=="nativeMinimumBlurRadius" && item.material->backdropPyramid())
                values.push_back(pyramidMinima.at(item.material->backdropScale())/std::max(itemStretch[itemIndex],.000001f));
            else if (u.name == "lensCenter") values.insert(values.end(), {center.x(), center.y()});
            else if (u.name == "lensSize" || u.name == "resolution") values.insert(values.end(), {size.width, size.height});
            else if (u.name == "cornerRadius") values.push_back(std::isfinite(item.layer->cornerRadius()) ? std::max(0.0f, item.layer->cornerRadius()) : 0);
            else if (u.name == "cornerCurve") values.push_back(item.layer->cornerCurve() == CALayerCornerCurve::continuous ? 1 : 0);
            else if (auto it=std::find(Internal::layerCornerShaderNames.begin(),Internal::layerCornerShaderNames.end(),u.name);
                     it!=Internal::layerCornerShaderNames.end()) {
                const auto data=Internal::layerCornerShaderData(size,item.layer->cornerRadius());
                const auto& components=data[std::distance(Internal::layerCornerShaderNames.begin(),it)];
                values.insert(values.end(),components.begin(),components.end());
            }
            else if (u.name == "interactionIntensity") values.push_back(item.layer->_interaction->glow());
            else if (u.name == "interactionLocation") {
                const auto location = SkPoint::Make(item.layer->_interaction->location.x, item.layer->_interaction->location.y);
                values.insert(values.end(), {location.x(), location.y()});
            } else if (u.name == "maximumSampleRadius") values.push_back(item.material->maximumSampleRadius());
            else {
                const auto uniform = Internal::BackdropShader::uniformValues(*item.material, u.name);
                if (uniform.size() != u.count) throw std::logic_error("Missing glass material uniform " + u.name);
                if ((u.name == "nativeShadowShape" || u.name == "nativeShadowResponse") && uniform.size() == 4) {
                    // An explicit CALayer shadow replaces automatic material
                    // elevation. Internal shadowAmount remains independent.
                    const auto opacity = item.layer->shadowColor() ? 0.0f : std::max<NXFloat>(0,item.layer->shadowOpacity());
                    values.insert(values.end(), uniform.begin(), uniform.begin() + 3);
                    values.push_back(uniform[3] * opacity);
                } else {
                    values.insert(values.end(), uniform.begin(), uniform.end());
                }
            }
        }
        if (_runtime->findUniform("material_" + u.name))
            builder.uniform("material_" + u.name).set(values.data(), values.size());
    }
    builder.uniform("inverseX").set(inverseX.data(), inverseX.size());
    builder.uniform("inverseY").set(inverseY.data(), inverseY.size());
    builder.uniform("forwardX").set(forwardX.data(), forwardX.size());
    builder.uniform("forwardY").set(forwardY.data(), forwardY.size());
    builder.uniform("geometry").set(geometry.data(), geometry.size());
    builder.uniform("itemAlpha").set(alphas.data(), alphas.size());
    builder.uniform("mergeSpacing") = _effect.spacing();
    builder.uniform("contentScale") = scale;
    builder.uniform("filterScale") = scale;
    if(_runtime->findUniform("outputBounds")) {
        // Exact SDFs under uniform scaling admit a cheap finite-output check.
        // Pixels where any pair could blend keep the complete normal-dependent
        // union and derivative calculation; its width is not globally constant.
        bool bounded=true;
        for(const auto& item:items) {
            const auto size=item.layer->bounds().size;
            const auto r=std::min(item.layer->cornerRadius(),std::min(size.width,size.height)*.5f);
            const bool exact=item.layer->cornerCurve()==CALayerCornerCurve::circular||r<=0
                ||(size.width==size.height&&r==size.width*.5f);
            const auto& m=item.matrix;
            bounded=bounded&&exact&&m.isScaleTranslate()&&std::abs(m.getScaleX())==std::abs(m.getScaleY());
        }
        const auto padding=_effect.spacing()*.25f*(items.size()-1)+2*outputMargin+2/scale;
        std::vector<float> rectangles;rectangles.reserve(items.size()*4);
        for(const auto& item:items) {
            auto rect=bounded?item.matrix.mapRect(SkRect::MakeWH(item.layer->bounds().width(),item.layer->bounds().height())):area;
            rect.outset(padding,padding);
            rectangles.insert(rectangles.end(),{rect.left(),rect.top(),rect.right(),rect.bottom()});
        }
        builder.uniform("outputBounds").set(rectangles.data(),rectangles.size());
    }
    if (_runtime->findUniform("externalShapeCoverage")) builder.uniform("externalShapeCoverage") = 1.0f;
    auto sharpDomain=pyramidDomain;
    const auto unionOutset=_effect.spacing()*.25f*(items.size()-1)*scale;
    sharpDomain.outset(unionOutset,unionOutset);
    const auto sharpInput=SkImageFilters::Crop(sharpDomain,nullptr);
    sk_sp<SkImageFilter> filter;
    if(items.size()<=3) {
        childNames.push_back("sharpContent"); inputs.push_back(sharpInput);
        std::vector<std::string_view> names(childNames.begin(),childNames.end());
        filter=SkImageFilters::RuntimeShader(builder,maximumRadius*scale,names.data(),inputs.data(),inputs.size());
    } else {
        sk_sp<SkImageFilter> sum;
        const auto uniformData=builder.uniforms();
        constexpr size_t batchSize=3;
        for(size_t first=0;first<items.size();first+=batchSize) {
            const auto last=std::min(first+batchSize,items.size());
            auto program=groupProgram(items.size(),first,last,false);
            SkRuntimeEffectBuilder batch(program);
            for(const auto& uniform:program->uniforms()) {
                const auto* from=_runtime->findUniform(uniform.name);
                if(!from || from->sizeInBytes()!=uniform.sizeInBytes())
                    throw std::logic_error("Glass batch uniform contract mismatch");
                batch.uniform(uniform.name).set(reinterpret_cast<const float*>(
                    static_cast<const char*>(uniformData->data())+from->offset),uniform.sizeInBytes()/sizeof(float));
            }
            std::vector<std::string_view> names;
            std::vector<sk_sp<SkImageFilter>> samples;
            for(size_t i=first*std::size(materialInputs);i<last*std::size(materialInputs);++i) {
                names.push_back(childNames[i]);samples.push_back(inputs[i]);
            }
            names.push_back("sharpContent");samples.push_back(sharpInput);
            auto image=SkImageFilters::RuntimeShader(batch,maximumRadius*scale,names.data(),samples.data(),samples.size());
            sum=sum?SkImageFilters::Blend(SkBlendMode::kPlus,std::move(sum),std::move(image)):std::move(image);
        }
        // Summation is linear in premultiplied RGBA, including source alpha.
        // The final pass restores the unchanged backdrop's remaining weight.
        std::string_view names[]={"materialSum","sharpContent"};
        sk_sp<SkImageFilter> samples[]={std::move(sum),nullptr};
        filter=SkImageFilters::RuntimeShader(builder,0,names,samples,2);
    }
    if (!filter) return;
    // Polynomial unions extend beyond member bounds. Add the transformed
    // native output mask and antialias reach, not a calibrated fixed shadow.
    const float margin = _effect.spacing() * .25f * (items.size() - 1) + outputMargin + 1/scale;
    area.outset(margin, margin);
    if (!area.isFinite()) return;
    // Keep the same shared material/union program but avoid shading empty
    // space between distant items (for example Back and the trailing pair).
    // Include both native output and input sampling reach before splitting,
    // so one pass cannot sample another pass's newly painted material.
    std::vector<SkRect> regions;
    const auto reach=std::max(margin,samplingReach);
    for(const auto& item:items) {
        auto region=item.matrix.mapRect(SkRect::MakeWH(item.layer->bounds().width(),item.layer->bounds().height()));
        region.outset(reach,reach);
        if(!region.intersect(area))continue;
        for(size_t i=0;i<regions.size();) {
            if(SkRect::Intersects(region,regions[i])) {
                region.join(regions[i]);regions.erase(regions.begin()+i);i=0;
            } else ++i;
        }
        regions.push_back(region);
    }
    area = SkRect::MakeLTRB(area.left() * scale, area.top() * scale, area.right() * scale, area.bottom() * scale);
    SkPaint paint; paint.setBlendMode(SkBlendMode::kSrc); paint.setImageFilter(std::move(filter));
    canvas->save(); canvas->scale(1 / scale, 1 / scale);
    // saveLayer bounds are only an allocation hint. A real clip keeps each
    // runtime filter local, rather than shading the entire window per group.
    canvas->clipRect(area);
    for(auto region:regions) {
        region=SkRect::MakeLTRB(region.left()*scale,region.top()*scale,region.right()*scale,region.bottom()*scale);
        canvas->save();canvas->clipRect(region);
        canvas->saveLayer(SkCanvas::SaveLayerRec(&region, &paint, SkCanvas::kInitWithPrevious_SaveLayerFlag));
        canvas->restore();canvas->restore();
    }
    canvas->restore();
    for (const auto& item : items) _renderedItems.push_back(item.layer.get());
}
