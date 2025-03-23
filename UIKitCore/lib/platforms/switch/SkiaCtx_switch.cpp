#include <platforms/switch/SkiaCtx_switch.h>

#include "include/core/SkColorSpace.h"
#include <include/core/SkGraphics.h>
#include <include/core/SkData.h>
#include <include/core/SkTypeface.h>
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/ios/GrGLMakeIOSInterface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/private/base/SkTemplates.h"

#include "include/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.h"
#include "include/ports/SkFontMgr_data.h"
#include "tools/trace/SkDebugfTracer.h"

#include <GLES3/gl3.h>


// Include the main libnx system header, for Switch development
#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

#include <tools/Logger.hpp>

extern "C" {
extern void userAppInit();
extern void userAppExit();
}

using namespace NXKit;

SkiaCtx_switch::SkiaCtx_switch() {
    SDL_Init(SDL_INIT_EVERYTHING);

    printf("SDL inited\n");

    Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    // SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    // SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, flags);

    if (!window) {
        printf("SDL window failed to create with error: %s\n", SDL_GetError());
    }

    auto context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    printf("SDL window created\n");

    SkGraphics::Init();

    // SkEventTracer* eventTracer = nullptr;
    // eventTracer = new SkDebugfTracer();
    // SkEventTracer::SetInstance(eventTracer);

    printf("SKIA inited\n");

    auto eglCtx = eglGetCurrentContext();
    if (!eglCtx) {
        printf("EGL context is empty\n");
    }

    initContext();

    printf("SKIA context created\n");

    PlFontData font;
    Result res = plGetSharedFontByType(&font, PlSharedFontType_Standard);

    auto data = SkData::MakeWithoutCopy(font.address, font.size);

    fontMgr = SkFontMgr_New_Custom_Data(SkSpan(&data, 1));
}

SkiaCtx_switch::~SkiaCtx_switch() {
}

void SkiaCtx_switch::initContext() {
    auto interface = GrGLInterfaces::MakeEGL();
    context = GrDirectContexts::MakeGL(interface);

    // GrGLint buffer;
    // interface->fFunctions.fGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
}

//float SkiaCtx_switch::getScaleFactor() {
//    return NSApplication.sharedApplication.keyWindow.backingScaleFactor;
//}


UIUserInterfaceStyle SkiaCtx_switch::getThemeMode() {
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);

    if (colorSetId == ColorSetId_Dark)
        return UIUserInterfaceStyle::dark;
    else
        return UIUserInterfaceStyle::light;
}

sk_sp<SkSurface> SkiaCtx_switch::getBackbufferSurface() {
    auto size = getSize();
    if (_size.width == size.width && _size.height == size.height && surface != nullptr) { return surface; }

    _size = size;

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(size.width, size.height,
                                                                  0, 8,
                                                                  framebuffer_info);

    SkSurfaceProps props;

    glViewport(0, 0, _size.width, _size.height);
    surface = SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                               kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                               nullptr, &props);

    return surface;
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_switch>();
}

bool SkiaCtx_switch::platformRunLoop(std::function<bool()> loop) {
    currentRunLoopStartTimer = Timer();
    return loop();
}

void SkiaCtx_switch::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

NXSize SkiaCtx_switch::getSize() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return { (NXFloat)w, (NXFloat)h };
}

float SkiaCtx_switch::getScaleFactor() {
    return 1;
}

