#pragma once

#include <UIKit.h>

class GlassDemoViewController final: public NXKit::UIViewController {
public:
    GlassDemoViewController();

    void loadView() override;
    void viewDidLayoutSubviews() override;

private:
    void clampGlassToBounds();

    std::shared_ptr<NXKit::UIImageView> _backgroundImageView;
    std::shared_ptr<NXKit::BackdropEffectView> _glassView;
    NXKit::NXPoint _dragOrigin;
    bool _hasPositionedGlass = false;
};
