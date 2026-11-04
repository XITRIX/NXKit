#include <SkiaCtx.h>
#include <iostream>
#include <string_view>
using namespace NXKit;
namespace {
class Context final : public SkiaCtx {
public:
    int wait() { currentRunLoopStartTimer=Timer(); return millisecondsUntilNextFrame(); }
    NXSize getSize() override { return {100,100}; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
}
int main(int argc,char** argv) {
    const bool uncapped=argc==2&&std::string_view(argv[1])=="--uncapped";
    auto context=std::make_shared<Context>();
    for(int rate : {1,30,60,120,-1}) {
        context->setTargetFrameRate(rate);
        const auto wait=context->wait();
        if(uncapped ? wait!=0 : wait<=0) {
            std::cerr<<"Unexpected frame wait "<<wait<<" for "<<rate<<'\n';return 1;
        }
    }
    std::cout<<(uncapped?"Uncapped":"Paced")<<" frame wait tests passed\n";
}
