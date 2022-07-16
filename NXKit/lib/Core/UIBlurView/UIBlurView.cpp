//
//  UIBlurView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "Application.hpp"
#include "UIBlurView.hpp"
#include "GaussianBlur.hpp"

static void shrinkImage(unsigned char** in, int w, int h, int scale) {
    for (int j = 0; j < h/scale; j++) {
        for (int i = 0; i < w/scale; i++) {
            int pixelPlace = j*(w/scale) + i;
            int oldPixelPlace = j*scale*w + (i*scale);
            for (int p = 0; p < 4; p++)
                (*in)[pixelPlace*4 + p] = (*in)[oldPixelPlace*4 + p];
        }
    }
}

static int getContextBluredImage(NVGcontext* vgContext, float x, float y, float w, float h, float r) {
    unsigned char* image;

    Size rootSize = Application::shared()->getKeyWindow()->frame.size();

    float scaleFactor = Application::shared()->getVideoContext()->getScaleFactor();
    int winHeight = static_cast<int>(rootSize.height * scaleFactor);

    int lx = static_cast<int>(x * scaleFactor);
    int ly = static_cast<int>(winHeight - y * scaleFactor);
    int lw = static_cast<int>(w * scaleFactor);
    int lh = static_cast<int>(h * scaleFactor);
    float lr = r * scaleFactor;

    Application::shared()->getVideoContext()->getContextPixels(lx, ly, lw, lh, &image);

    if (image == NULL || lw <= 0 || lh <= 0)
        return 0;

    int minim = 2;
    if (minim != 0) {
        shrinkImage(&image, lw, lh, minim);

        unsigned char* blured = (unsigned char*)malloc(static_cast<size_t>(lw * lh * 4 / minim));
        fast_gaussian_blur_rgb(image, blured, lw/minim, lh/minim, 4, lr/minim);
        int img = nvgCreateImageRGBA(vgContext, lw/minim, lh/minim, 0, blured);

        free(image);
        free(blured);
        return img;
    } else {
        int img = nvgCreateImageRGBA(vgContext, lw, lh, 0, image);

        free(image);
        return img;
    }
}

UIBlurView::UIBlurView(Rect frame): UIView(frame) { }

void UIBlurView::draw(NVGcontext *vgContext) {
    int img = getContextBluredImage(vgContext, frame.origin().x, frame.origin().y, frame.size().width, frame.size().height, blurRadius);
    NVGpaint imgPaint = nvgImagePattern(vgContext, 0, 0, frame.size().width, frame.size().height, 0, img, 1);
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, frame.size().width, frame.size().height, cornerRadius);
    nvgFillPaint(vgContext, imgPaint);
    nvgFill(vgContext);

    Application::shared()->flushContext();

    nvgDeleteImage(vgContext, img);
}
