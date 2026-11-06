#pragma once
// Actual UIKit _UITabSelectionView CABackdropLayer filter settings; provenance
// is retained in docs/liquid-glass-lens/native-settings/control-materials.
// UITabBar resting selection surface, independent of LensView's caller-owned default.
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
        effect.setUniform("redRow",dark?std::array<NXKit::NXFloat,4>{1.129799962f,-.2360999882f,-.02369999886f,-.0700000003f}:std::array<NXKit::NXFloat,4>{1.185099959f,-.0502000004f,-.00499999989f,-.1999999881f});
        effect.setUniform("greenRow",dark?std::array<NXKit::NXFloat,4>{-.07009999454f,.9639999866f,-.02379999869f,-.0700000003f}:std::array<NXKit::NXFloat,4>{-.01489999983f,1.149899959f,-.00509999972f,-.1999999881f});
        effect.setUniform("blueRow",dark?std::array<NXKit::NXFloat,4>{-.07019999623f,-.2360000014f,1.176199913f,-.0700000003f}:std::array<NXKit::NXFloat,4>{-.01489999983f,-.04999999702f,1.194899917f,-.1999999881f});
        // Native _UITabSelectionView's resting CABackdropLayer: Gaussian
        // inputRadius 2 before the color matrix, captured at quarter scale.
        effect.setBackdropBlurRadius(2);
        effect.setBackdropScale(.25f);
        return effect;
    }
};
}
