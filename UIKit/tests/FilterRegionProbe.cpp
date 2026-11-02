// Keep the Skia canvas subclass in its own translation unit: the retained
// raster Skia archive is built without RTTI, while NXKit needs RTTI.
#include "include/utils/SkNoDrawCanvas.h"
#include "include/core/SkPaint.h"
#include <functional>
#include <vector>
std::vector<SkRect> captureBackdropRegions(const std::function<void(SkCanvas*)>& draw) {
    class Canvas final:public SkNoDrawCanvas {
    public:
        Canvas():SkNoDrawCanvas(1200,400){}
        std::vector<SkRect> areas;
    protected:
        SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
            if(rec.fBounds&&rec.fPaint&&rec.fPaint->getImageFilter())areas.push_back(*rec.fBounds);
            return kNoLayer_SaveLayerStrategy;
        }
    } canvas;
    draw(&canvas);return canvas.areas;
}
