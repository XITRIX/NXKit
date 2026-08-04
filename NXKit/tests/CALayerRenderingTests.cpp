#include <BackdropEffect.h>
#include <CALayer.h>

#include "CABackdropEffectLayer.hpp"

#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/utils/SkNoDrawCanvas.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace NXKit;

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

struct SaveLayerCall {
    std::optional<SkRect> bounds;
    SkCanvas::SaveLayerFlags flags = 0;
    sk_sp<SkImageFilter> imageFilter;
};

class SaveLayerTrackingCanvas final: public SkNoDrawCanvas {
public:
    SaveLayerTrackingCanvas(int width, int height): SkNoDrawCanvas(width, height) {}

    std::vector<SaveLayerCall> calls;

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& record) override {
        calls.push_back({
            record.fBounds ? std::optional(*record.fBounds) : std::nullopt,
            record.fSaveLayerFlags,
            record.fPaint ? record.fPaint->refImageFilter() : nullptr,
        });
        return SkNoDrawCanvas::getSaveLayerStrategy(record);
    }
};

void testMaskLayersUseLocalBounds() {
    auto layer = new_shared<CALayer>();
    layer->setBounds(NXRect(7, 9, 80, 40));
    layer->setPosition({120, 90});

    auto mask = new_shared<CALayer>();
    mask->setBounds(NXRect(0, 0, 80, 40));
    mask->setPosition({40, 20});
    layer->setMask(mask);

    SaveLayerTrackingCanvas canvas(640, 480);
    layer->skiaRender(&canvas);

    expect(canvas.calls.size() == 2,
           "a masked layer uses one content layer and one destination-in layer");
    const auto expectedBounds = SkRect::MakeWH(80, 40);
    for (const auto& call : canvas.calls) {
        expect(call.bounds.has_value(), "masked offscreen layers have explicit bounds");
        expect(call.bounds && *call.bounds == expectedBounds,
               "masked offscreen layers are bounded in layer-local coordinates");
    }
    if (!canvas.calls.empty()) {
        expect((canvas.calls.front().flags
                    & SkCanvas::kInitWithPrevious_SaveLayerFlag) != 0,
               "masked content retains the existing backdrop");
    }
}

void testBackdropFilterCacheTracksGeometry() {
    auto layer = new_shared<CABackdropEffectLayer>(BackdropEffect::glass());
    layer->setBounds(NXRect(0, 0, 340, 140));
    layer->setCornerRadius(36);

    SaveLayerTrackingCanvas canvas(640, 480);
    layer->draw(&canvas);
    expect(canvas.calls.size() == 1 && canvas.calls.back().imageFilter,
           "backdrop drawing creates a runtime image filter");
    if (canvas.calls.empty() || !canvas.calls.back().imageFilter) return;
    const auto initialFilter = canvas.calls.back().imageFilter;

    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() == initialFilter.get(),
           "unchanged backdrop geometry reuses its image filter");

    layer->setPosition({100, 50});
    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() == initialFilter.get(),
           "position-only changes reuse the backdrop image filter");

    layer->setBounds(NXRect(0, 0, 300, 140));
    layer->draw(&canvas);
    const auto resizedFilter = canvas.calls.back().imageFilter;
    expect(resizedFilter.get() != initialFilter.get(),
           "backdrop size changes rebuild the image filter");

    layer->setCornerRadius(24);
    layer->draw(&canvas);
    const auto roundedFilter = canvas.calls.back().imageFilter;
    expect(roundedFilter.get() != resizedFilter.get(),
           "backdrop corner-radius changes rebuild the image filter");

    layer->setContentsScale(2);
    layer->draw(&canvas);
    const auto scaledFilter = canvas.calls.back().imageFilter;
    expect(scaledFilter.get() != roundedFilter.get(),
           "backdrop content-scale changes rebuild the image filter");

    auto changedEffect = BackdropEffect::glass();
    changedEffect.setUniform("saturation", 0.9f);
    layer->setEffect(changedEffect);
    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() != scaledFilter.get(),
           "backdrop effect changes rebuild the image filter");
}

} // namespace

int main() {
    testMaskLayersUseLocalBounds();
    testBackdropFilterCacheTracksGeometry();

    if (failures != 0) {
        std::cerr << failures << " layer-rendering assertion(s) failed\n";
        return 1;
    }
    std::cout << "CALayer rendering tests passed\n";
    return 0;
}
