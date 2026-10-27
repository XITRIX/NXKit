#include "CAGlassContainerLayer.hpp"
#include "CABackdropEffectLayer.hpp"
#include "include/effects/SkImageFilters.h"
#include <algorithm>
#include <cmath>
#include <regex>
#include <map>
#include <stdexcept>

using namespace NXKit;
namespace {
thread_local std::vector<const CABackdropEffectLayer*> groupedLayers;
struct MaterialUniform { std::string type, name; int count; };
const std::vector<MaterialUniform>& materialUniforms() {
    static const auto uniforms = [] {
        std::vector<MaterialUniform> result;
        const auto source = BackdropEffect::glass().shaderSource();
        const std::regex declaration("uniform (float[234]?) ([A-Za-z0-9_]+);");
        for (auto i = std::sregex_iterator(source.begin(), source.end(), declaration); i != std::sregex_iterator(); ++i) {
            const std::string type = (*i)[1], name = (*i)[2];
            if (name == "contentScale" || name == "filterScale" || name == "externalShapeCoverage" || name == "maximumSampleRadius") continue;
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
std::string shaderSource(size_t count) {
    const auto n = std::to_string(count);
    auto source = BackdropEffect::glass().shaderSource();
    for (const auto& u : materialUniforms()) {
        replaceAll(source, "uniform " + u.type + " " + u.name + ";",
            "uniform " + u.type + " material_" + u.name + "[" + n + "];\n" + u.type + " " + u.name + ";");
    }
    std::string declarations = "uniform float4 inverseX[" + n + "], inverseY[" + n + "], geometry[" + n + "];\n"
        "uniform float itemAlpha[" + n + "], itemShadow[" + n + "];\nuniform float mergeSpacing;\nfloat weights[" + n + "], distances[" + n + "];\nfloat2 mergedNormal;\n";
    std::string backdrop = "half4 groupBackdrop(float2 xy) { half4 color = half4(0);\n";
    for (size_t i = 0; i < count; ++i) {
        declarations += "uniform shader backdrop" + std::to_string(i) + ";\n";
        backdrop += "color += half(weights[" + std::to_string(i) + "]) * backdrop" + std::to_string(i) + ".eval(xy);\n";
    }
    backdrop += "return color; }\n";
    source.insert(source.find("const float SMOOTH_EDGE_PX"), declarations + backdrop);
    replaceAll(source, "return content.eval(filterXY);", "return sharpContent.eval(filterXY);");
    replaceAll(source, "content.eval(", "groupBackdrop(");
    replaceAll(source, "float2 normal = lensNormalDirection(p, halfDim, r);", "float2 normal = mergedNormal;");
    // Skia requires constant indices for uniform arrays on portable backends.
    // Generate straight-line reductions; geometry and materials remain uniforms.
    const std::string itemFunction = R"SKSL(
float3 itemDistanceINDEX(float2 xy) {
    float4 g = geometry[INDEX];
    float2 p = float2(dot(inverseX[INDEX].xyz, float3(xy, 1)), dot(inverseY[INDEX].xyz, float3(xy, 1))) - g.xy * 0.5;
    cornerCurve = g.w;
    float r = min(g.z, min(g.x, g.y) * 0.5);
    float d = boxRoundedSDF(p, g.xy * 0.5, r);
    float2 n = lensNormalDirection(p, g.xy * 0.5, r);
    float2 gradient = float2(inverseX[INDEX].x * n.x + inverseY[INDEX].x * n.y,
                            inverseX[INDEX].y * n.x + inverseY[INDEX].y * n.y);
    float magnitude = max(length(gradient), 0.0001);
    return float3(d / magnitude, gradient / magnitude);
}
)SKSL";
    std::string helpers;
    for (size_t i = 0; i < count; ++i) {
        auto function = itemFunction;
        replaceAll(function, "INDEX", std::to_string(i));
        helpers += function;
    }
    helpers += "float mergeGeometry(float2 xy) { float3 first = itemDistance0(xy); float d = first.x; mergedNormal = first.yz; weights[0] = 1; distances[0] = d;\n";
    for (size_t i = 1; i < count; ++i) {
        const auto index = std::to_string(i);
        helpers += "{ float3 item = itemDistance" + index + "(xy); float other = item.x; distances[" + index + "] = other;\n"
            // Native contour measurements show a smaller blend for parallel
            // normals. In particular, coincident flat edges must not bulge.
            "float k = mergeSpacing * clamp(0.5 - 0.5 * dot(item.yz, mergedNormal / max(length(mergedNormal), 0.0001)), 0, 1);\n"
            "float h = k > 0.0001 ? clamp(0.5 + 0.5 * (other - d) / k, 0, 1) : (d <= other ? 1 : 0);\n";
        for (size_t j = 0; j < i; ++j) helpers += "weights[" + std::to_string(j) + "] *= h;\n";
        helpers += "weights[" + index + "] = 1 - h; mergedNormal = mix(item.yz, mergedNormal, h);\n"
            "d = mix(other, d, h) - k * h * (1 - h); }\n";
    }
    // Geometry weights fill the bridge outside the individual shapes. Inside
    // each item UIKit gives the front material ownership, with a narrow seam.
    // The captured color handoff extends about six points ahead of the front
    // shape, over an eight-point band; its darker internal optics are approximate.
    // Material order must follow the same painter order as foreground content.
    for (size_t i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        helpers += "{ float cover = clamp(1.25 - distances[" + index + "] / 8.0, 0, 1);\n";
        for (size_t j = 0; j < count; ++j) helpers += "weights[" + std::to_string(j) + "] *= 1-cover;\n";
        helpers += "weights[" + index + "] += cover; }\n";
    }
    helpers += "mergedNormal /= max(length(mergedNormal), 0.0001); return d; }\n";
    source.insert(source.find("float toBrightness"), helpers);
    std::string setup = "float2 xy = filterXY / max(filterScale, 1.0);\n"
        "float sceneDistance = mergeGeometry(xy);\nfloat materialAlpha = 0, materialShadow = 0;\n";
    for (const auto& u : materialUniforms()) {
        setup += u.name + " = " + u.type + "(0);\n";
        for (size_t i = 0; i < count; ++i) setup += u.name + " += weights[" + std::to_string(i) + "] * material_" + u.name + "[" + std::to_string(i) + "];\n";
    }
    for (size_t i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        setup += "materialAlpha += weights[" + index + "] * itemAlpha[" + index + "];\n";
        setup += "materialShadow += weights[" + index + "] * itemShadow[" + index + "];\n";
    }
    setup += "half4 original = sharpContent.eval(filterXY);\n"
        "float shadowDistance = max(sceneDistance - mergedNormal.y * 2, 0);\n"
        "float shadow = materialShadow * (0.0294 * exp(-0.5 * pow(shadowDistance / 5.12, 2)) + "
        "0.0098 * exp(-0.5 * pow(max(sceneDistance - mergedNormal.y, 0) / 1.65, 2)));\n"
        "half4 exterior = half4(original.rgb * half(1-shadow), original.a);\n";
    replaceAll(source, "float2 xy = filterXY / max(filterScale, 1.0);", setup);
    replaceAll(source, "return sharpContent.eval(filterXY);", "return exterior;");
    replaceAll(source, "float sdf = boxRoundedSDF(p, halfDim, r);", "float sdf = sceneDistance;");
    replaceAll(source, "return referenceGlass(xy, halfDim, r, p, sdf);", "half4 material = mix(original, referenceGlass(xy, halfDim, r, p, sdf), half(materialAlpha)); "
        "float coverage = clamp(0.5 - sdf * max(contentScale, 1), 0, 1); "
        "return half4(material.rgb - original.rgb * half(shadow * (1-coverage)), material.a);");
    return source;
}
}

struct CAGlassContainerLayer::Item {
    std::shared_ptr<CABackdropEffectLayer> layer;
    SkMatrix matrix, inverse;
    NXFloat alpha;
    BackdropEffect material;
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
            glass->_glassMaterial->_backdropEffect(size, glass->_glassTraits)});
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
    if (items.empty()) return;
    if (!_runtime || _runtimeCount != items.size()) {
        // Reuse the program across containers and membership changes. All
        // current geometry, optics and traits are supplied afresh as uniforms.
        static thread_local std::map<size_t, sk_sp<SkRuntimeEffect>> programs;
        if (const auto cached = programs.find(items.size()); cached != programs.end()) _runtime = cached->second;
        else {
            auto compiled = SkRuntimeEffect::MakeForShader(SkString(shaderSource(items.size())));
            if (!compiled.effect) throw std::runtime_error("Glass container shader: " + std::string(compiled.errorText.c_str()));
            _runtime = std::move(compiled.effect);
            if (programs.size() >= 8) programs.erase(programs.begin());
            programs[items.size()] = _runtime;
        }
        _runtimeCount = items.size();
    }
    SkRuntimeEffectBuilder builder(_runtime);
    NXFloat scale = std::isfinite(contentsScale()) ? std::max(1.0f, contentsScale()) : 1;
    NXFloat maximumRadius = 0;
    std::vector<float> inverseX, inverseY, geometry, alphas, shadows;
    SkRect area = SkRect::MakeEmpty();
    std::vector<std::string> childNames;
    std::vector<sk_sp<SkImageFilter>> inputs;
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        const auto& m = item.inverse;
        const auto& layer = item.layer;
        const auto size = layer->bounds().size;
        inverseX.insert(inverseX.end(), {m.getScaleX(), m.getSkewX(), m.getTranslateX(), 0});
        inverseY.insert(inverseY.end(), {m.getSkewY(), m.getScaleY(), m.getTranslateY(), 0});
        geometry.insert(geometry.end(), {size.width, size.height,
            std::isfinite(layer->cornerRadius()) ? std::max(0.0f, layer->cornerRadius()) : 0,
            layer->cornerCurve() == CALayerCornerCurve::continuous ? 1.0f : 0.0f});
        alphas.push_back(item.alpha);
        const auto appearance = item.material.uniform("backdropLuma");
        shadows.push_back(!layer->shadowColor() && appearance && appearance->front() > .5f
            ? std::max(0.0f, layer->shadowOpacity()) * item.alpha : 0);
        area.join(item.matrix.mapRect(SkRect::MakeWH(size.width, size.height)));
        maximumRadius = std::max(maximumRadius, item.material.maximumSampleRadius());
        const auto blur = item.material.backdropBlurRadius() * scale;
        inputs.push_back(blur > 0 ? SkImageFilters::Blur(blur, blur, SkTileMode::kClamp, nullptr) : nullptr);
        childNames.push_back("backdrop" + std::to_string(i));
        canvas->save(); canvas->concat(item.matrix); layer->observeBackdrop(canvas); canvas->restore();
    }
    for (const auto& u : materialUniforms()) {
        std::vector<float> values;
        for (const auto& item : items) {
            const auto size = item.layer->bounds().size;
            const auto center = item.matrix.mapPoint({size.width * 0.5f, size.height * 0.5f});
            if (u.name == "lensCenter") values.insert(values.end(), {center.x(), center.y()});
            else if (u.name == "lensSize" || u.name == "resolution") values.insert(values.end(), {size.width, size.height});
            else if (u.name == "cornerRadius") values.push_back(std::isfinite(item.layer->cornerRadius()) ? std::max(0.0f, item.layer->cornerRadius()) : 0);
            else if (u.name == "cornerCurve") values.push_back(item.layer->cornerCurve() == CALayerCornerCurve::continuous ? 1 : 0);
            else if (u.name == "interactionIntensity") values.push_back(item.layer->_interaction->glow());
            else if (u.name == "interactionLocation") {
                const auto location = item.matrix.mapPoint({item.layer->_interaction->location.x, item.layer->_interaction->location.y});
                values.insert(values.end(), {location.x(), location.y()});
            } else {
                const auto uniform = item.material.uniform(u.name);
                if (!uniform || uniform->size() != u.count) throw std::logic_error("Missing glass material uniform " + u.name);
                values.insert(values.end(), uniform->begin(), uniform->end());
            }
        }
        builder.uniform("material_" + u.name).set(values.data(), values.size());
    }
    builder.uniform("inverseX").set(inverseX.data(), inverseX.size());
    builder.uniform("inverseY").set(inverseY.data(), inverseY.size());
    builder.uniform("geometry").set(geometry.data(), geometry.size());
    builder.uniform("itemAlpha").set(alphas.data(), alphas.size());
    builder.uniform("itemShadow").set(shadows.data(), shadows.size());
    builder.uniform("mergeSpacing") = _effect.spacing();
    builder.uniform("contentScale") = scale;
    builder.uniform("filterScale") = scale;
    builder.uniform("externalShapeCoverage") = 0.0f;
    builder.uniform("maximumSampleRadius") = maximumRadius;
    childNames.push_back("sharpContent"); inputs.push_back(nullptr);
    std::vector<std::string_view> names(childNames.begin(), childNames.end());
    auto filter = SkImageFilters::RuntimeShader(builder, maximumRadius * scale, names.data(), inputs.data(), inputs.size());
    if (!filter) return;
    // Polynomial unions can extend beyond the input rectangles when several
    // shapes overlap. Include that bound and the unified elevation shadow.
    const float margin = _effect.spacing() * .25f * (items.size() - 1) + 16;
    area.outset(margin, margin);
    if (!area.isFinite()) return;
    area = SkRect::MakeLTRB(area.left() * scale, area.top() * scale, area.right() * scale, area.bottom() * scale);
    SkPaint paint; paint.setImageFilter(std::move(filter));
    canvas->save(); canvas->scale(1 / scale, 1 / scale);
    // saveLayer bounds are only an allocation hint. A real clip keeps each
    // runtime filter local, rather than shading the entire window per group.
    canvas->clipRect(area);
    canvas->saveLayer(SkCanvas::SaveLayerRec(&area, &paint, SkCanvas::kInitWithPrevious_SaveLayerFlag));
    canvas->restore(); canvas->restore();
    for (const auto& item : items) _renderedItems.push_back(item.layer.get());
}
