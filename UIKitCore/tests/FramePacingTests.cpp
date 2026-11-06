#include <SkiaCtx.h>
#include <iostream>
#include <string_view>
#include <cmath>
using namespace NXKit;
namespace {
class Context final : public SkiaCtx {
public:
    int wait() { return millisecondsUntilNextFrame(); }
    bool frame(double ms) { return beginScheduledFrame(at(ms)); }
    double remaining(double ms) { return remainingFrameMilliseconds(at(ms)); }
    void startNow() { beginScheduledFrame(); }
    NXSize getSize() override { return {100,100}; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    static FrameClock::time_point at(double ms) { return FrameClock::time_point{}+
        std::chrono::duration_cast<FrameClock::duration>(std::chrono::duration<double,std::milli>(ms)); }
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
}
int main(int argc,char** argv) {
    const bool uncapped=argc==2&&std::string_view(argv[1])=="--uncapped";
    for(int rate : {1,30,60,120,-1}) {
        auto context=std::make_shared<Context>();
        context->setTargetFrameRate(rate);
        context->startNow();
        const auto wait=context->wait();
        if(uncapped ? wait!=0 : wait<=0) {
            std::cerr<<"Unexpected frame wait "<<wait<<" for "<<rate<<'\n';return 1;
        }
    }
    // Model input arriving at 1kHz. Early wakes must keep the pending deadline
    // instead of rendering at the input rate or postponing the next frame.
    auto burst=std::make_shared<Context>();burst->setTargetFrameRate(120);
    int rendered=0;
    for(int ms=0;ms<=90;++ms)if(burst->frame(ms))++rendered;
    if(rendered!=(uncapped?91:11)) {
        std::cerr<<"Input burst submitted "<<rendered<<" frames\n";return 1;
    }
    auto precise=std::make_shared<Context>();precise->setTargetFrameRate(120);
    if(!precise->frame(0)||(!uncapped&&precise->frame(4))||!precise->frame(1000./120))return 1;
    if(!uncapped&&std::abs(precise->remaining(9)-(2000./120-9))>.00001)return 1;
    // A 0.35ms scheduler overshoot must not accumulate into a slower clock.
    auto jitter=std::make_shared<Context>();jitter->setTargetFrameRate(120);jitter->frame(0);
    if(!jitter->frame(1000./120+.35))return 1;
    if(!uncapped&&std::abs(jitter->remaining(9)-(2000./120-9))>.00001) {
        std::cerr<<"Scheduler overshoot shifted the next display interval\n";return 1;
    }
    double now=0;int count=0;
    auto sustained=std::make_shared<Context>();sustained->setTargetFrameRate(120);
    while(now<1000) {
        if(sustained->frame(now))++count;
        now+=uncapped?1:sustained->remaining(now)+.35;
    }
    if(count!=(uncapped?1000:120)) {
        std::cerr<<"Scheduler jitter submitted "<<count<<" frames per second\n";return 1;
    }
    // An active touch changes an idle 30Hz deadline immediately to 120Hz.
    auto wake=std::make_shared<Context>();wake->setTargetFrameRate(30);wake->frame(0);
    wake->setTargetFrameRate(120);if(!wake->frame(9))return 1;
    // Slow GPU work uses the current position once; never replay missed frames.
    if(!wake->frame(100)||(!uncapped&&wake->frame(101)))return 1;
    std::cout<<(uncapped?"Uncapped":"Paced")<<" frame wait tests passed\n";
}
