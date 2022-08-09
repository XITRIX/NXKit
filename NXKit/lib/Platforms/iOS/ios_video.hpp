//
//  ios_video.h
//  NXKit
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#pragma once

#include "video.hpp"

#import <OpenGLES/ES2/glext.h>

#include <string>

// GLFW Video Context
class IOSVideoContext : public NXKit::VideoContext
{
  public:
    IOSVideoContext(NVGcontext* nvgContext);
    ~IOSVideoContext();

    NVGcontext* getNVGContext() override;

    void clear(NVGcolor color) override;
    void beginFrame() override;
    void endFrame() override;
    void resetState() override;
    void disableScreenDimming(bool disable) override;
    void getContextPixels(int x, int y, int w, int h, unsigned char** pixels) override;
    float getScaleFactor() override;
    bool mainLoopInteraction() override;

    void setScale(double width, double height, double scale);
    void applicationLoop();
  private:
    double scaleFactor = 1;
    NVGcontext* nvgContext = nullptr;
};
