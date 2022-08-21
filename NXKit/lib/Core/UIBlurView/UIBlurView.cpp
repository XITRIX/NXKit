//
//  UIBlurView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/Application/Application.hpp>
#include <Core/UIBlurView/UIBlurView.hpp>
#include <Core/Utils/GaussianBlur/GaussianBlur.hpp>

namespace NXKit {

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

    Size rootSize = Application::shared()->getKeyWindow()->getFrame().size;

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

    int minim = 4;
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

UIBlurView::UIBlurView(Rect frame): UIStackView() {
    backgroundColor = UIColor(235, 235, 235).withAlphaComponent(0.6f);
//    backgroundColor = UIColor::white.withAlphaComponent(0.6f);
}

void UIBlurView::draw(NVGcontext *vgContext) {
    int img = getContextBluredImage(vgContext, getFrame().origin.x, getFrame().origin.y, getFrame().size.width, getFrame().size.height, blurRadius);
    NVGpaint imgPaint = nvgImagePattern(vgContext, 0, 0, getFrame().size.width, getFrame().size.height, 0, img, 1);
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, getFrame().size.width, getFrame().size.height, cornerRadius);
    nvgFillPaint(vgContext, imgPaint);
    nvgFill(vgContext);

    Application::shared()->flushContext();

    nvgDeleteImage(vgContext, img);
}

}
