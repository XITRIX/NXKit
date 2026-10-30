#pragma once
// Generated from public UIKit tab-bar pixels; see selection-fill.json.
// A Demo role-specific resting surface, not LensView's caller-owned default.
#include <BackdropEffectView.h>
namespace {
class TabSelectionBackground final:public NXKit::BackdropEffectView {
public:
    TabSelectionBackground():BackdropEffectView(material(false)) {setUserInteractionEnabled(false);refresh();}
    void traitCollectionDidChange(std::shared_ptr<NXKit::UITraitCollection> previous) override {
        BackdropEffectView::traitCollectionDidChange(std::move(previous));
        refresh();
    }
private:
    void refresh() {
        const bool dark=traitCollection()&&traitCollection()->userInterfaceStyle()==NXKit::UIUserInterfaceStyle::dark;
        if(dark==_dark)return;_dark=dark;setEffect(material(dark));
    }
    bool _dark=false;
    static NXKit::BackdropEffect material(bool dark) {
        NXKit::BackdropEffect effect(R"sksl(
uniform shader content;
uniform float filterScale;
uniform float2 resolution;
uniform float4 redRow,greenRow,blueRow;
half4 main(float2 xy) {
    half4 pixel=content.eval(clamp(xy,float2(0),resolution*max(filterScale,1.0)));
    float4 inputColor=float4(pixel.rgb/max(pixel.a,.0001),1);
    half3 color=half3(clamp(float3(dot(redRow,inputColor),dot(greenRow,inputColor),dot(blueRow,inputColor)),0.0,1.0));
    return half4(color*pixel.a,pixel.a);
})sksl");
        effect.setUniform("redRow",dark?std::array<NXKit::NXFloat,4>{1.129685657f,-0.268587906f,0.009542716f,-0.068709054f}:std::array<NXKit::NXFloat,4>{1.190725046f,-0.054807616f,-0.002633461f,-0.203186910f});
        effect.setUniform("greenRow",dark?std::array<NXKit::NXFloat,4>{-0.067807323f,0.958160815f,-0.021460494f,-0.069538603f}:std::array<NXKit::NXFloat,4>{-0.006877994f,1.138118429f,0.002043534f,-0.203186910f});
        effect.setUniform("blueRow",dark?std::array<NXKit::NXFloat,4>{-0.071222788f,-0.248578552f,1.188988833f,-0.069398802f}:std::array<NXKit::NXFloat,4>{-0.008485711f,-0.064453918f,1.206223598f,-0.203186910f});
        return effect;
    }
};
}
