#pragma once
#include <Geometry.h>
#include "RenderPerformance.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include <cmath>
#include <cstdlib>

namespace NXKit::Internal {
inline bool boundedLensForegroundEnabled() {
    static const bool value=[] {const auto flag=std::getenv("NXKIT_UNBOUNDED_LENS");return !flag||flag[0]!='1';}();
    return value;
}
// Only for settled lens output: its destination reads are point samples and
// its caller guarantees that all output is contained in the live lens bounds.
class LensForegroundComposition {
public:
    LensForegroundComposition(SkCanvas* canvas,NXSize size,bool enabled):canvas(canvas),saveCount(canvas->getSaveCount()) {
        const auto* device=SkCanvasPriv::TopDevice(canvas);SkMatrix inverse;
        const auto matrix=canvas->getTotalMatrix();
        if(!enabled||!boundedLensForegroundEnabled()||!device||!device->isClipRect()||device->isClipAntiAliased()
           ||device->imageInfo().alphaType()!=kPremul_SkAlphaType||device->surfaceProps().pixelGeometry()!=kUnknown_SkPixelGeometry
           ||!matrix.isScaleTranslate()||matrix.getScaleX()<=0||matrix.getScaleX()!=matrix.getScaleY()||!matrix.invert(&inverse))return;
        auto area=SkRect::MakeWH(size.width,size.height);
        area.outset(2*(std::abs(inverse.getScaleX())+std::abs(inverse.getSkewX())),
                    2*(std::abs(inverse.getScaleY())+std::abs(inverse.getSkewY())));
        const auto toDevice=SkMatrix::Concat(device->globalToDevice().asM33(),matrix);
        if(!area.isFinite()||toDevice.mapRect(area).roundOut().contains(device->bounds()))return;
        canvas->save();canvas->clipRect(area);
        SkPaint replace;replace.setBlendMode(SkBlendMode::kSrc);
        canvas->saveLayer(SkCanvas::SaveLayerRec(&area,&replace,SkCanvas::kInitWithPrevious_SaveLayerFlag));
        active=true;if(auto* metrics=RenderPerformance::current)++metrics->boundedLenses;
    }
    bool isActive() const {return active;}
    ~LensForegroundComposition(){if(active)canvas->restoreToCount(saveCount);}
    LensForegroundComposition(const LensForegroundComposition&)=delete;
    LensForegroundComposition& operator=(const LensForegroundComposition&)=delete;
private:
    SkCanvas* canvas;
    int saveCount;
    bool active=false;
};
}
