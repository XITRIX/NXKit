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

static int getContextBluredImage(NVGcontext* vgContext, int x, int y, int w, int h, float r) {
    unsigned char* image;

    Size rootSize = Application::shared()->rootView->frame.size;

    float scaleFactor = Application::shared()->getVideoContext()->getScaleFactor();
    int winHeight = rootSize.height * scaleFactor;

    int lx = x * scaleFactor;
    int ly = winHeight - y * scaleFactor;
    int lw = w * scaleFactor;
    int lh = h * scaleFactor;
    float lr = r * scaleFactor;

    Application::shared()->getVideoContext()->getContextPixels(lx, ly, lw, lh, &image);

    if (image == NULL)
        return 0;

    int minim = 2;
    if (minim != 0) {
        shrinkImage(&image, lw, lh, minim);

        unsigned char* blured = (unsigned char*)malloc(lw*lh*4/minim);
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
//    nvgBeginPath(vgContext);
//    nvgRoundedRect(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, cornerRadius);
//    nvgFillColor(vgContext, nvgRGBA(155, 155, 155, 155));
//    nvgFill(vgContext);

    int img = getContextBluredImage(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, blurRadius);
    NVGpaint imgPaint = nvgImagePattern(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, 0, img, 1);
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, cornerRadius);
    nvgFillPaint(vgContext, imgPaint);
    nvgFill(vgContext);

    Application::shared()->flushContext();

    nvgDeleteImage(vgContext, img);
}
