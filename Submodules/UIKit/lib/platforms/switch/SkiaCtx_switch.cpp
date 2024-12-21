#include <platforms/switch/SkiaCtx_switch.h>

#include "include/core/SkColorSpace.h"
#include <include/core/SkGraphics.h>
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
#include "include/ports/SkFontMgr_empty.h"

#include <GLES3/gl3.h>


// Include the main libnx system header, for Switch development
#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

using namespace NXKit;

static EGLDisplay s_display;
static EGLContext s_context;
static EGLSurface s_surface;

static bool initEgl(NWindow* win)
{
    // Connect to the EGL default display
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!s_display)
    {
        // LTRACEF("Could not connect to display! error: %d", eglGetError());
        goto _fail0;
    }

    // Initialize the EGL display connection
    eglInitialize(s_display, nullptr, nullptr);

    // Select OpenGL (Core) as the desired graphics API
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    {
        // LTRACEF("Could not set API! error: %d", eglGetError());
        goto _fail1;
    }

    // Get an appropriate EGL framebuffer configuration
    EGLConfig config;
    EGLint numConfigs;
    static const EGLint framebufferAttributeList[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(s_display, framebufferAttributeList, &config, 1, &numConfigs);
    if (numConfigs == 0)
    {
        // LTRACEF("No config found! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL window surface
    s_surface = eglCreateWindowSurface(s_display, config, win, nullptr);
    if (!s_surface)
    {
        // LTRACEF("Surface creation failed! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL rendering context
    static const EGLint contextAttributeList[] =
    {
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_NONE
    };
    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!s_context)
    {
        // LTRACEF("Context creation failed! error: %d", eglGetError());
        goto _fail2;
    }

    // Connect the context to the surface
    eglMakeCurrent(s_display, s_surface, s_surface, s_context);
    return true;

_fail2:
    eglDestroySurface(s_display, s_surface);
    s_surface = nullptr;
_fail1:
    eglTerminate(s_display);
    s_display = nullptr;
_fail0:
    return false;
}

static void deinitEgl()
{
    if (s_display)
    {
        eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s_context)
        {
            eglDestroyContext(s_display, s_context);
            s_context = nullptr;
        }
        if (s_surface)
        {
            eglDestroySurface(s_display, s_surface);
            s_surface = nullptr;
        }
        eglTerminate(s_display);
        s_display = nullptr;
    }
}

SkiaCtx_switch::SkiaCtx_switch() {
    if (!initEgl(nwindowGetDefault()))
        exit(1);


    SDL_Init(SDL_INIT_EVERYTHING);
    // Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    // SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    // SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // flags |= SDL_WINDOW_OPENGL;
    // window = SDL_CreateWindow("Window", 12, 12, 1280, 720, flags);
    // auto context = SDL_GL_CreateContext(window);
    // SDL_GL_MakeCurrent(window, context);

    SkGraphics::Init();
    initContext();

    // fontMgr = SkFontMgr_New_Custom_Empty();
}

void SkiaCtx_switch::initContext() {
    auto interface = GrGLInterfaces::MakeEGL();
    auto ctx = GrDirectContexts::MakeGL(interface);
}

//float SkiaCtx_switch::getScaleFactor() {
//    return NSApplication.sharedApplication.keyWindow.backingScaleFactor;
//}

sk_sp<SkSurface> SkiaCtx_switch::getBackbufferSurface() {
    auto size = getSize();
    if (_size.width == size.width && _size.height == size.height && surface != nullptr) { return surface; }

    _size = size;

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;
    auto scaleFactor = getScaleFactor();
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(size.width * scaleFactor,
                                                                  size.height * scaleFactor,
                                                                  0, 8,
                                                                  framebuffer_info);

    SkSurfaceProps props;

    glViewport(0, 0, _size.width * scaleFactor, _size.height * scaleFactor);
    surface = SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                               kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                               nullptr, &props);

    return surface;
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_switch>();
}

bool NXKit::platformRunLoop(std::function<bool()> loop) {
    return loop();
}

void SkiaCtx_switch::swapBuffers() {
    eglSwapBuffers(s_display, s_surface);
}

NXSize SkiaCtx_switch::getSize() {
    return { 1280, 720 };
}

float SkiaCtx_switch::getScaleFactor() {
    return 1;
}

