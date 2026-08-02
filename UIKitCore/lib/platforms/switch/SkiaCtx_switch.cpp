#include <platforms/switch/SkiaCtx_switch.h>

#include "include/core/SkData.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/ports/SkFontMgr_data.h"

#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/OpenGLBackend.h>
#include <webgpu/webgpu_cpp.h>

#include <EGL/egl.h>
#include <switch.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

extern "C" {
extern void nxkitEnsureSwitchRuntimeLinked();
}

using namespace NXKit;

namespace {

void reportWebGPUError(const char* source,
                       uint32_t type,
                       wgpu::StringView message) {
    const char* text = message.data ? message.data : "";
    const size_t length = message.data == nullptr
                                  ? 0
                                  : (message.length == WGPU_STRLEN
                                             ? std::strlen(message.data)
                                             : message.length);
    const int printableLength = static_cast<int>(
            std::min(length, static_cast<size_t>(std::numeric_limits<int>::max())));
    SkDebugf("%s (%u): %.*s\n", source, type, printableLength, text);
}

} // namespace

struct SkiaCtx_switch::GraphiteState {
    std::unique_ptr<dawn::native::Instance> instance;
    wgpu::Instance webgpuInstance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Surface swapchain;
    wgpu::Texture backbufferTexture;
    std::unique_ptr<skgpu::graphite::Context> context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
};

SkiaCtx_switch::SkiaCtx_switch() {
    nxkitEnsureSwitchRuntimeLinked();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SkDebugf("SDL initialization failed: %s\n", SDL_GetError());
        return;
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("Window", 1280, 720, flags);
    if (!window) {
        SkDebugf("SDL window creation failed: %s\n", SDL_GetError());
        return;
    }

    SkGraphics::Init();
    PlFontData font{};
    Result res = plGetSharedFontByType(&font, PlSharedFontType_Standard);
    if (R_SUCCEEDED(res) && font.address != nullptr && font.size > 0) {
        auto data = SkData::MakeWithoutCopy(font.address, font.size);
        auto systemFontMgr = SkFontMgr_New_Custom_Data(SkSpan(&data, 1));
        if (systemFontMgr) {
            fontMgr = std::move(systemFontMgr);
        } else {
            SkDebugf("Skia could not create the Switch system font manager.\n");
        }
    } else if (R_FAILED(res)) {
        SkDebugf("Switch system font load failed: 0x%x\n", res);
    } else {
        SkDebugf("Switch system font service returned empty data.\n");
    }

    // Font setup must precede GPU setup: UIKit performs text layout before it
    // asks the renderer for its first backbuffer.
    if (!initContext()) {
        SkDebugf("Graphite/Dawn context creation failed.\n");
        return;
    }
}

SkiaCtx_switch::~SkiaCtx_switch() {
    surface.reset();

    if (graphite) {
        if (graphite->swapchain) {
            graphite->swapchain.Unconfigure();
        }
        graphite->backbufferTexture = nullptr;
        graphite->recorder.reset();
        graphite->context.reset();
        graphite->swapchain = nullptr;
        graphite->device = nullptr;
        graphite->webgpuInstance = nullptr;
        graphite->instance.reset();
        graphite.reset();
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

bool SkiaCtx_switch::initContext() {
    dawn::native::DawnInstanceDescriptor dawnInstanceDescriptor;

    static constexpr auto kRequiredInstanceFeature =
            wgpu::InstanceFeatureName::TimedWaitAny;
    wgpu::InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.requiredFeatureCount = 1;
    instanceDescriptor.requiredFeatures = &kRequiredInstanceFeature;
    instanceDescriptor.nextInChain = &dawnInstanceDescriptor;
    graphite = std::make_unique<GraphiteState>();
    graphite->instance = std::make_unique<dawn::native::Instance>(&instanceDescriptor);
    if (!graphite->instance) {
        SkDebugf("Dawn native instance allocation failed.\n");
        return false;
    }

    DawnProcTable procs = dawn::native::GetProcs();
    dawnProcSetProcs(&procs);

    graphite->webgpuInstance = wgpu::Instance(graphite->instance->Get());
    if (!graphite->webgpuInstance) {
        SkDebugf("Dawn WebGPU instance creation failed.\n");
        return false;
    }

    dawn::native::opengl::RequestAdapterOptionsGetGLProc glOptions;
    glOptions.getProc = reinterpret_cast<dawn::native::opengl::EGLGetProcProc>(eglGetProcAddress);
    glOptions.display = EGL_NO_DISPLAY;

    wgpu::RequestAdapterOptions adapterOptions{};
    adapterOptions.nextInChain = &glOptions;
    adapterOptions.backendType = wgpu::BackendType::OpenGLES;
    adapterOptions.featureLevel = wgpu::FeatureLevel::Compatibility;

    std::vector<dawn::native::Adapter> adapters =
            graphite->instance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        SkDebugf("Dawn found no OpenGL ES compatibility adapter.\n");
        return false;
    }

    graphite->adapter = wgpu::Adapter(adapters.front().Get());

    wgpu::Limits limits{};
    if (!graphite->adapter.GetLimits(&limits)) {
        SkDebugf("Dawn could not query adapter limits.\n");
        return false;
    }

    wgpu::DeviceDescriptor deviceDescriptor{};
    deviceDescriptor.requiredLimits = &limits;
    deviceDescriptor.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                if (reason != wgpu::DeviceLostReason::Destroyed) {
                    reportWebGPUError("Dawn device lost", static_cast<uint32_t>(reason), message);
                }
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                reportWebGPUError("Dawn uncaptured error", static_cast<uint32_t>(type), message);
            });
    graphite->device = graphite->adapter.CreateDevice(&deviceDescriptor);
    if (!graphite->device) {
        SkDebugf("Dawn OpenGL ES device creation failed.\n");
        return false;
    }

    wgpu::SurfaceSourceAndroidNativeWindow nativeWindowDescriptor{};
    nativeWindowDescriptor.window = nwindowGetDefault();

    wgpu::SurfaceDescriptor surfaceDescriptor{};
    surfaceDescriptor.nextInChain = &nativeWindowDescriptor;
    graphite->swapchain = graphite->webgpuInstance.CreateSurface(&surfaceDescriptor);
    if (!graphite->swapchain || !configureSurface()) {
        SkDebugf("Dawn Switch presentation surface creation/configuration failed.\n");
        return false;
    }

    skgpu::graphite::DawnBackendContext backendContext{};
    backendContext.fInstance = graphite->webgpuInstance;
    backendContext.fDevice = graphite->device;
    backendContext.fQueue = graphite->device.GetQueue();

    skgpu::graphite::ContextOptions contextOptions{};
    contextOptions.fInternalMultisampleCount = skgpu::graphite::SampleCount::k1;
    contextOptions.fRequireOrderedRecordings = true;
    graphite->context =
            skgpu::graphite::ContextFactory::MakeDawn(backendContext, contextOptions);
    if (!graphite->context) {
        SkDebugf("Skia Graphite/Dawn context creation failed.\n");
        return false;
    }

    graphite->recorder = createGraphiteRecorder(graphite->context.get());
    if (!graphite->recorder) {
        SkDebugf("Skia Graphite recorder creation failed.\n");
        return false;
    }

    return true;
}

bool SkiaCtx_switch::configureSurface() {
    if (!window || !graphite || !graphite->adapter || !graphite->device ||
        !graphite->swapchain) {
        SkDebugf("Dawn surface configuration prerequisites are missing.\n");
        return false;
    }

    const auto size = getSize();
    if (size.width <= 0 || size.height <= 0) {
        SkDebugf("Dawn surface size is invalid: %dx%d.\n",
                 static_cast<int>(size.width),
                 static_cast<int>(size.height));
        return false;
    }

    wgpu::SurfaceCapabilities capabilities{};
    if (!graphite->swapchain.GetCapabilities(graphite->adapter, &capabilities) ||
        capabilities.formatCount == 0 || capabilities.presentModeCount == 0) {
        SkDebugf("Dawn surface capabilities are unavailable.\n");
        return false;
    }

    _size = size;
    wgpu::SurfaceConfiguration configuration{};
    configuration.device = graphite->device;
    configuration.format = capabilities.formats[0];
    configuration.usage = wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::TextureBinding |
                          wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;
    configuration.width = static_cast<uint32_t>(size.width);
    configuration.height = static_cast<uint32_t>(size.height);
    configuration.presentMode = capabilities.presentModes[0];
    if (capabilities.alphaModeCount > 0) {
        configuration.alphaMode = capabilities.alphaModes[0];
    }
    graphite->swapchain.Configure(&configuration);
    graphite->webgpuInstance.ProcessEvents();
    return true;
}

skgpu::graphite::Context* SkiaCtx_switch::graphiteContext() {
    return graphite ? graphite->context.get() : nullptr;
}

skgpu::graphite::Recorder* SkiaCtx_switch::graphiteRecorder() {
    return graphite ? graphite->recorder.get() : nullptr;
}

UIUserInterfaceStyle SkiaCtx_switch::getThemeMode() {
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    return colorSetId == ColorSetId_Dark ? UIUserInterfaceStyle::dark
                                         : UIUserInterfaceStyle::light;
}

sk_sp<SkSurface> SkiaCtx_switch::getBackbufferSurface() {
    if (!graphite || !graphite->recorder || !graphite->swapchain) {
        return nullptr;
    }

    const auto size = getSize();
    if (size.width != _size.width || size.height != _size.height) {
        surface.reset();
        graphite->backbufferTexture = nullptr;
        if (!configureSurface()) {
            return nullptr;
        }
    }

    if (surface) {
        return surface;
    }

    wgpu::SurfaceTexture surfaceTexture{};
    graphite->swapchain.GetCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return nullptr;
    }

    graphite->backbufferTexture = std::move(surfaceTexture.texture);
    auto backendTexture =
            skgpu::graphite::BackendTextures::MakeDawn(graphite->backbufferTexture.Get());
    SkSurfaceProps properties{};
    surface = SkSurfaces::WrapBackendTexture(graphite->recorder.get(),
                                             backendTexture,
                                             nullptr,
                                             &properties);
    if (!surface) {
        SkDebugf("Skia could not wrap the Dawn backbuffer texture.\n");
    }
    return surface;
}

void SkiaCtx_switch::flushAndSubmit(sk_sp<SkSurface> frameSurface) {
    if (!frameSurface || !graphite || !graphite->context || !graphite->recorder) {
        return;
    }

    auto recording = graphite->recorder->snap();
    if (!recording) {
        SkDebugf("Skia Graphite failed to snap the frame recording.\n");
        return;
    }

    skgpu::graphite::InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    const auto status = graphite->context->insertRecording(insertInfo);
    if (status != skgpu::graphite::InsertStatus::kSuccess) {
        SkDebugf("Skia Graphite failed to insert the frame recording: %s\n",
                 status.message().c_str());
        return;
    }

    if (!graphite->context->submit(skgpu::graphite::SyncToCpu::kNo)) {
        SkDebugf("Skia Graphite failed to submit the frame.\n");
        return;
    }
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_switch>();
}

bool SkiaCtx_switch::platformRunLoop(std::function<bool()> loop) {
    currentRunLoopStartTimer = Timer();
    return loop();
}

void SkiaCtx_switch::swapBuffers() {
    if (!graphite || !graphite->swapchain || !surface) {
        return;
    }

    const auto presentStatus = graphite->swapchain.Present();
    if (!presentStatus) {
        SkDebugf("Dawn failed to present the frame.\n");
    }
    surface.reset();
    graphite->backbufferTexture = nullptr;
    graphite->webgpuInstance.ProcessEvents();
}

NXSize SkiaCtx_switch::getSize() {
    int width = 0;
    int height = 0;
    if (window) {
        SDL_GetWindowSize(window, &width, &height);
    }
    return {static_cast<NXFloat>(width), static_cast<NXFloat>(height)};
}

float SkiaCtx_switch::getScaleFactor() {
    return 1;
}
