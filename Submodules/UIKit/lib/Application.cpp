#include "Application.h"

#include <SDL2/SDL_syswm.h>

#include <include/core/SkGraphics.h>
#include <include/core/SkSurface.h>
#include <include/core/SkRRect.h>
#include <include/core/SkCanvas.h>

// SKIA METAL GPU
#include "include/gpu/ganesh/GrBackendSurface.h"

#include "platforms/SkiaInit.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

Application* Application::shared = nullptr;

int Application::resizingEventWatcher(void* data, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
        if (win == (SDL_Window*)data) {
            Application::shared->render();
        }
    }
    return 0;
}

Application::Application() {
    shared = this;

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Window", 12, 12, 1280, 720, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    SkGraphics::Init();
    skiaWindow = skiaMakeWindow(window);

    SDL_AddEventWatch(resizingEventWatcher, window);

    while(platformRunLoop([this]() { return runLoop(); }));
}

Application::~Application() {
    skiaWindow = nullptr;
}

bool Application::runLoop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    render();
    return true;
}

void Application::render() {
    auto surface = skiaWindow->getBackbufferSurface();
    if (!surface) return;

    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

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

    // Create middle overlay rectangle for background blur
    const SkRect middle = SkRect::MakeXYWH(64, 64, 128, 128);

    canvas->save();
//    // Use middle rectangle as clip mask
//    canvas->clipRect(middle, false);
    canvas->clipRect(SkRect::MakeWH(190.5f, 110.5f), true);

    SkPaint redPaint;
    redPaint.setAntiAlias(true);
    redPaint.setColor(SK_ColorGREEN);
    canvas->drawCircle(190, 71, 40, redPaint);

    // Two blur filters, one that we're currently using and the newer one in current version of Skia.
    // Both blur filters select a tile mode for clamping the blur filter at the rectangle's edges.
    // However, the result on the CPU does NOT appear to clamp at all, while the result on GPU does!
//    sk_sp<SkImageFilter> oldBlurFilter = SkBlurImageFilter::Make(25, 25, nullptr, nullptr, SkBlurImageFilter::kClamp_TileMode);
    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(20, 20, SkTileMode::kClamp, nullptr);
//
//    SkMatrix matrix;
//    matrix.setScale(6, 6);
//
    SkPaint blurPaint;
    blurPaint.setAntiAlias(true);

    blurPaint.setStyle(SkPaint::kFill_Style);
    blurPaint.setStrokeWidth(10);
    blurPaint.setImageFilter(std::move(newBlurFilter));
//
    canvas->save();
//    // Make a separate layer using the blur filter, clipped to the middle rectangle's bounds
    SkCanvas::SaveLayerRec slr(&middle, &blurPaint, SkCanvas::kInitWithPrevious_SaveLayerFlag);
    canvas->saveLayer(slr);
    canvas->restore();

//
//    // Fill the clip middle rectangle with a transparent white
    canvas->drawColor(0x40FFFFFF);
    canvas->restore();
    canvas->restore();

// -----------------------

//    // Create middle overlay rectangle for background blur
//    canvas->save();
//    // Use middle rectangle as clip mask
//    canvas->clipRect(middle, true);
//
//    // Two blur filters, one that we're currently using and the newer one in current version of Skia.
//    // Both blur filters select a tile mode for clamping the blur filter at the rectangle's edges.
//    // However, the result on the CPU does NOT appear to clamp at all, while the result on GPU does!
////    sk_sp<SkImageFilter> oldBlurFilter = SkBlurImageFilter::Make(25, 25, nullptr, nullptr, SkBlurImageFilter::kClamp_TileMode);
//    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(25, 25, SkTileMode::kClamp, nullptr);
//
//    SkPaint p;
//    p.setAntiAlias(true);
//    p.setStyle(SkPaint::kFill_Style);
//    p.setStrokeWidth(10);
//    p.setImageFilter(std::move(newBlurFilter));
//
//    // Make a separate layer using the blur filter, clipped to the middle rectangle's bounds
//    SkCanvas::SaveLayerRec slr(&middle, &p, SkCanvas::kInitWithPrevious_SaveLayerFlag);
//    canvas->saveLayer(slr);
//
//    // Fill the clip middle rectangle with a transparent white
//    canvas->drawColor(0x40FFFFFF);
//    canvas->restore();
//    canvas->restore();

// -----------------------

    if (auto dContext = skiaWindow->directContext()) {
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }

    skiaWindow->swapBuffers();
}
