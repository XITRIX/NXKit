// The historical gesture harness uses labels without creating a renderer.
// Supply its font service when running it in the headless raster suite.
#include <SkiaCtx.h>
#define main gestureTestsMain
#include "../../../UIKit/tests/UIGestureRecognizerTests.cpp"
#undef main
#include "include/ports/SkFontMgr_mac_ct.h"
class GestureContext final:public NXKit::SkiaCtx {
public:
 GestureContext(){fontMgr=SkFontMgr_New_CoreText(nullptr);}
 NXKit::NXSize getSize()override{return {1000,1400};}
 sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}
 void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
int main(){NXKit::SkiaCtx::_main=std::make_shared<GestureContext>();return gestureTestsMain();}
