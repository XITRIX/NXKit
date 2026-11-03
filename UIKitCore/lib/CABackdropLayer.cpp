#include <CABackdropLayer.h>
#include "BackdropShader.hpp"
#include "LayerCornerGeometry.hpp"
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkImageFilters.h"
#include <algorithm>
#include <cmath>

using namespace NXKit;

struct CABackdropLayer::State {
    BackdropEffect effect;
    sk_sp<SkRuntimeEffect> runtime;
    explicit State(const BackdropEffect& value)
        : effect(value), runtime(Internal::BackdropShader::compile(value)) {
        Internal::BackdropShader::validate(value, *runtime);
    }
};

CABackdropLayer::CABackdropLayer(const BackdropEffect& effect)
    : _state(std::make_shared<State>(effect)) {}
CABackdropLayer::CABackdropLayer(CABackdropLayer* source)
    : CALayer(source), _state(source->_state) {}
CABackdropLayer::~CABackdropLayer() = default;
BackdropEffect CABackdropLayer::effect() const { return _state->effect; }

void CABackdropLayer::setEffect(const BackdropEffect& effect) {
    auto next = std::make_shared<State>(effect);
    // Model and presentation copies share non-animatable shader parameters.
    *_state = std::move(*next);
    setNeedsDisplay();
}
std::shared_ptr<CALayer> CABackdropLayer::copy() {
    return new_shared<CABackdropLayer>(this);
}
void CABackdropLayer::draw(SkCanvas* canvas) {
    const auto size = bounds().size;
    if (!std::isfinite(size.width) || !std::isfinite(size.height)
        || size.width <= 0 || size.height <= 0) return;
    auto filter = Internal::BackdropShader::makeFilter(
        _state->effect, _state->runtime, size, renderingCornerRadius(), cornerCurve(), contentsScale()
    );
    if (!filter) return;
    SkPaint paint;
    paint.setImageFilter(std::move(filter));
    const auto scale = _state->runtime->findUniform("filterScale") && std::isfinite(contentsScale())
        ? std::max<NXFloat>(1, contentsScale()) : 1;
    const auto rect = SkRect::MakeWH(size.width * scale, size.height * scale);
    canvas->save();
    if (!masksToBounds() && !hasCornerConfiguration()) canvas->clipPath(
        Internal::layerCornerPath(size, renderingCornerRadius(), cornerCurve()), true
    );
    canvas->scale(1 / scale, 1 / scale);
    canvas->saveLayer(SkCanvas::SaveLayerRec(
        &rect, &paint, SkCanvas::kInitWithPrevious_SaveLayerFlag
    ));
    canvas->restore();
    canvas->restore();
}
