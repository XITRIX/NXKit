/*
    Copyright 2021 natinusala

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

//#include <borealis/core/application.hpp>
//#include <borealis/core/logger.hpp>
#include "ios_video.hpp"

#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
//#include <glad/glad.h>

//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/rotate_vector.hpp>
//#include <glm/mat4x4.hpp>
//#include <glm/vec3.hpp>
//#include <glm/vec4.hpp>

// nanovg implementation
#define NANOVG_GLES2_IMPLEMENTATION
#import <OpenGLES/ES2/glext.h>

#include "Application.hpp"

using namespace NXKit;

//static void glfwWindowFramebufferSizeCallback(GLFWwindow* window, int width, int height)
//{
//    if (!width || !height)
//        return;
//
//    glViewport(0, 0, width, height);
//
//
//    int wWidth, wHeight;
//    int fWidth, fHeight;
//
//    glfwGetWindowSize(window, &wWidth, &wHeight);
//    glfwGetFramebufferSize(window, &fWidth, &fHeight);
//
////    Application::shared()->onWindowResized(static_cast<unsigned>(wWidth), static_cast<unsigned>(wHeight), (float)fWidth / (float)wWidth);
//}

void IOSVideoContext::setScale(double width, double height, double scale) {
    
    static double swidth = -1;
    static double sheight = -1;

    if (swidth != width || sheight != height || scaleFactor != scale) {
        scaleFactor = scale;
        swidth = width;
        sheight = height;

        Application::shared()->onWindowResized(static_cast<unsigned>(width), static_cast<unsigned>(height), (float) scale);
    }
}

IOSVideoContext::IOSVideoContext(NVGcontext* nvgContext):
    nvgContext(nvgContext)
{ }

float IOSVideoContext::getScaleFactor() {
    return scaleFactor;
}

void IOSVideoContext::beginFrame()
{ }

void IOSVideoContext::endFrame()
{ }

void IOSVideoContext::clear(NVGcolor color)
{
    glClearColor(
                 color.r,
                 color.g,
                 color.b,
                 1.0f);

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void IOSVideoContext::resetState()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
}

bool IOSVideoContext::mainLoopInteraction() {
//    bool isActive;
//    do {
//        isActive = !glfwGetWindowAttrib(getGLFWWindow(), GLFW_ICONIFIED);
//
//        if (isActive)
//            glfwPollEvents();
//        else
//            glfwWaitEvents();
//    } while (!isActive);
//
//    bool platform = true;
//
//    return !glfwWindowShouldClose(getGLFWWindow()) || !platform;
    return true;
}

void IOSVideoContext::disableScreenDimming(bool disable)
{
}

void IOSVideoContext::applicationLoop() {
    Application::shared()->mainLoop();
}

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

//    if (this->nvgContext)
//        nvgDeleteGL3(this->nvgContext);
//
//    glfwDestroyWindow(this->window);
//    glfwTerminate();
}

NVGcontext* IOSVideoContext::getNVGContext()
{
    return this->nvgContext;
}
