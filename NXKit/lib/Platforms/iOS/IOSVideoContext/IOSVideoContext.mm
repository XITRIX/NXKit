/*
    Copyright 2022 XITRIX

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "IOSVideoContext.hpp"

// nanovg implementation
#define NANOVG_GLES2_IMPLEMENTATION
#import <OpenGLES/ES2/glext.h>
#include <nanovg-gl/nanovg_gl.h>

#include <Core/Application/Application.hpp>

NXKit::UIUserInterfaceStyle styleFrom(UIUserInterfaceStyle style) {
    switch (style) {
        case UIUserInterfaceStyleUnspecified:
            return NXKit::UIUserInterfaceStyle::unspecified;
        case UIUserInterfaceStyleDark:
            return NXKit::UIUserInterfaceStyle::dark;
        case UIUserInterfaceStyleLight:
            return NXKit::UIUserInterfaceStyle::light;
    }
}

namespace NXKit {

IOSVideoContext::IOSVideoContext(GLViewController *controller): controller(controller)
{
    // Initialize nanovg
    this->nvgContext = nvgCreateGLES2(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
    if (!this->nvgContext)
    {
        //        Logger::error("glfw: unable to init nanovg");
        return;
    }
}

float IOSVideoContext::getScaleFactor() {
    return controller.view.window.screen.scale;
}

void IOSVideoContext::beginFrame()
{}

void IOSVideoContext::endFrame()
{ }

void IOSVideoContext::clear(NVGcolor color)
{
    glClearColor(
                 color.r,
                 color.g,
                 color.b,
                 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void IOSVideoContext::resetState()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
}

bool IOSVideoContext::mainLoopInteraction()
{
    return true;
}

void IOSVideoContext::disableScreenDimming(bool disable)
{ }

static void flipHorizontal(unsigned char* image, int w, int h, int stride) {
    int i = 0, j = h-1, k;
    while (i < j) {
        unsigned char* ri = &image[i * stride];
        unsigned char* rj = &image[j * stride];
        for (k = 0; k < w*4; k++) {
            unsigned char t = ri[k];
            ri[k] = rj[k];
            rj[k] = t;
        }
        i++;
        j--;
    }
}

void IOSVideoContext::getContextPixels(int x, int y, int w, int h, unsigned char** pixels)
{
    Application::shared()->flushContext();

    *pixels = (unsigned char*) malloc(static_cast<size_t>(w * h * 4));

    glReadPixels(x, y - h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
    flipHorizontal(*pixels, w, h, w*4);
}

IOSVideoContext::~IOSVideoContext()
{
    if (this->nvgContext)
        nvgDeleteGLES2(this->nvgContext);
}

NVGcontext* IOSVideoContext::getNVGContext()
{
    return this->nvgContext;
}

UIUserInterfaceStyle IOSVideoContext::getUserInterfaceStyle()
{
    return styleFrom(controller.traitCollection.userInterfaceStyle);
}

}
