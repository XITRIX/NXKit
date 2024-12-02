//
// Created by Даниил Виноградов on 17.11.2024.
//

#include <SDL_syswm.h>
#include "Application.h"

#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "tools/sk_app/Window.h"
#include "include/core/SkGraphics.h"
#include "tools/window/DisplayParams.h"
#include "include/core/SkRRect.h"

#include "include/ports/SkFontMgr_empty.h"
#include "include/ports/SkFontMgr_mac_ct.h"

sk_app::Application *sk_app::Application::Create(int argc, char **argv, void *platformData) {
    return new UIApplication(argc, argv, platformData);
}

UIApplication::UIApplication(int argc, char **argv, void *platformData) {
    SkGraphics::Init();
    fWindow = sk_app::Windows::CreateNativeWindow(platformData);

//    auto paramsBuilder = skwindow::DisplayParamsBuilder::Make();
//    paramsBuilder->msaaSampleCount(1);
//    GrContextOptions grctxOpts;
////    CommonFlags::SetCtxOptions(&grctxOpts);
////    grctxOpts.fPersistentCache = &fPersistentCache;
//    grctxOpts.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
////    grctxOpts.fShaderErrorHandler = &gShaderErrorHandler;
//    grctxOpts.fSuppressPrints = true;
//    grctxOpts.fSupportBilerpFromGlyphAtlas = true;
//    paramsBuilder->grContextOptions(grctxOpts);
//    fWindow->setRequestedDisplayParams(paramsBuilder->build());

    fWindow->pushLayer(this);

    fWindow->attach(sk_app::Window::kMetal_BackendType);
//    fWindow->attach(sk_app::Window::kGraphiteMetal_BackendType);

//    mgr = SkFontMgr_New_Custom_Empty();
    mgr = SkFontMgr_New_CoreText(nullptr);

////    auto file = romfs::get("SF-Compact.ttf");
//    printf("Fonts %d\n", mgr->countFamilies());
//
//    SkString name;
//    mgr->getFamilyName(0, &name);
//    printf("Font: - %s\n", name.data());
//
//    auto family = mgr->matchFamily("SF Pro");
//
    SkFontStyle fontStyle;
//    family->getStyle(0, &fontStyle, nullptr);
    typeface = mgr->matchFamilyStyle(nullptr, fontStyle);
//    typeface = mgr->makeFromData(SkData::MakeWithoutCopy(file.data(), file.size()));
}

void UIApplication::onPaint(SkSurface *surface) {
    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

//    paint.setImageFilter(std::move(newBlurFilter));

    // Draw a rectangle with red paint
    SkRect rect = SkRect::MakeXYWH(10, 10, 512, 200);
    SkRRect rrect;
    SkVector corners[] = {{24, 36}, {36, 24}, {24, 24}, {24, 24}};
    rrect.setRectRadii(rect, corners);
    canvas->drawRRect(rrect, paint);

    // Set up a linear gradient and draw a circle
    {
        SkPoint linearPoints[] = { { 0, 0 }, { 300, 300 } };
        SkColor linearColors[] = { SK_ColorGREEN, SK_ColorBLACK };
        paint.setShader(SkGradientShader::MakeLinear(linearPoints, linearColors, nullptr, 2,
                                                     SkTileMode::kMirror));
        paint.setAntiAlias(true);

        canvas->drawCircle(200, 200, 64, paint);

        // Detach shader
        paint.setShader(nullptr);
    }

    // Draw a message with a nice black paint

    SkFont font(typeface);
    font.setSubpixel(true);
    font.setSize(49);
    paint.setColor(SK_ColorBLACK);

    canvas->save();
    static const char message[] = "Hello World ";

    // Translate and rotate
    canvas->translate(300, 300);
    fRotationAngle += 0.2f;
    if (fRotationAngle > 360) {
        fRotationAngle -= 360;
    }
    canvas->rotate(fRotationAngle);

    // Draw the text
    canvas->drawString(message, 200, 20, font, paint);
//    canvas->drawSimpleText(message, strlen(message), SkTextEncoding::kUTF8, 0, 0, font, paint);

    canvas->restore();

//    const SkRect middle = SkRect::MakeXYWH(64, 64, 128, 128);
//    // Use middle rectangle as clip mask
//    canvas->clipRect(middle, true);
//
//    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(25, 25, SkTileMode::kClamp, nullptr);
//    paint.setImageFilter(newBlurFilter);
//
//    SkCanvas::SaveLayerRec slr(&middle, &paint, SkCanvas::kInitWithPrevious_SaveLayerFlag);
//    canvas->saveLayer(slr);
//
//    // Fill the clip middle rectangle with a transparent white
//    canvas->drawColor(0x40FFFFFF);
//    canvas->restore();

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kFill_Style);
    p.setStrokeWidth(10);

//    // Draw red squares
//    p.setColor(SK_ColorRED);
//    const SkRect red = SkRect::MakeXYWH(0, 0, 128, 128);
//    canvas->drawRect(red, p);
//    const SkRect red2 = SkRect::MakeXYWH(128, 128, 128, 128);
//    canvas->drawRect(red2, p);
//
//    // Draw blue squares
//    p.setColor(SK_ColorBLUE);
////    p.setAlphaf(0.5f);
//    const SkRect blue = SkRect::MakeXYWH(128, 0, 128, 128);
//    canvas->drawRect(blue, p);
//    const SkRect blue2 = SkRect::MakeXYWH(0, 128, 128, 128);
//    canvas->drawRect(blue2, p);

    // Create middle overlay rectangle for background blur
    const SkRect middle = SkRect::MakeXYWH(64, 64, 128, 128);

    canvas->save();
    // Use middle rectangle as clip mask
    canvas->clipRect(middle, true);

    // Two blur filters, one that we're currently using and the newer one in current version of Skia.
    // Both blur filters select a tile mode for clamping the blur filter at the rectangle's edges.
    // However, the result on the CPU does NOT appear to clamp at all, while the result on GPU does!
//    sk_sp<SkImageFilter> oldBlurFilter = SkBlurImageFilter::Make(25, 25, nullptr, nullptr, SkBlurImageFilter::kClamp_TileMode);
    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(25, 25, SkTileMode::kClamp, nullptr);

    SkMatrix matrix;
    matrix.setScale(6, 6);

    SkPaint blurPaint;
    blurPaint.setAntiAlias(true);
    blurPaint.setImageFilter(
            SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(), nullptr));

//    blurPaint.setStyle(SkPaint::kFill_Style);
//    blurPaint.setStrokeWidth(10);
//    blurPaint.setImageFilter(std::move(newBlurFilter));

    // Make a separate layer using the blur filter, clipped to the middle rectangle's bounds
    SkCanvas::SaveLayerRec slr(&middle, &blurPaint, SkCanvas::kInitWithPrevious_SaveLayerFlag);
//    canvas->saveLayer(slr);

    SkCanvas::SaveLayerRec rec;
//    SkPaint paint;
    blurPaint.setBlendMode(SkBlendMode::kPlus);
    rec.fSaveLayerFlags = SkCanvas::kInitWithPrevious_SaveLayerFlag;
    rec.fPaint = &blurPaint;
    canvas->saveLayer(rec);
    blurPaint.setBlendMode(SkBlendMode::kClear);
    canvas->drawCircle(128, 128, 96, blurPaint);
    canvas->restore();

    // Fill the clip middle rectangle with a transparent white
    canvas->drawColor(0x40FFFFFF);
    canvas->restore();
    canvas->restore();
}

void UIApplication::onIdle() {
    fWindow->inval();
}

void UIApplication::onResize(int width, int height) {
    printf("Resize\n");
}

void UIApplication::onBackendCreated() {
    fWindow->show();
}

UIApplication::~UIApplication() {
    fWindow->detach();
    delete fWindow;
}
