#pragma once

#include <UIKit.h>

class GlassDemoViewController final: public NXKit::UIViewController {
public:
    GlassDemoViewController();

    void loadView() override;
    void viewDidLayoutSubviews() override;

private:
    void makeDraggable(const std::shared_ptr<NXKit::BackdropEffectView>& glassView);
    void layoutSizeSlider();
    void setRegularGlassSizeProgress(NXKit::NXFloat progress);
    void clampGlassToBounds(
        const std::shared_ptr<NXKit::BackdropEffectView>& glassView
    );

    std::shared_ptr<NXKit::UIImageView> _backgroundImageView;
    std::shared_ptr<NXKit::BackdropEffectView> _clearGlassView;
    std::shared_ptr<NXKit::BackdropEffectView> _regularGlassView;
    std::shared_ptr<NXKit::UIView> _sizeSliderView;
    std::shared_ptr<NXKit::UILabel> _sizeSliderTitleLabel;
    std::shared_ptr<NXKit::UILabel> _sizeSliderValueLabel;
    std::shared_ptr<NXKit::UIView> _sizeSliderTrackView;
    std::shared_ptr<NXKit::UIView> _sizeSliderFillView;
    std::shared_ptr<NXKit::UIView> _sizeSliderThumbView;
    NXKit::NXPoint _dragOrigin;
    NXKit::NXFloat _regularGlassSizeProgress = 6.0f / 11.0f;
    bool _hasPositionedGlass = false;
};
