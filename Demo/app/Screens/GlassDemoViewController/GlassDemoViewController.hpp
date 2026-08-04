#pragma once

#include <UIKit.h>

class GlassDemoViewController final: public NXKit::UIViewController {
public:
    GlassDemoViewController();

    void loadView() override;
    void viewDidLayoutSubviews() override;

private:
    void makeDraggable(const std::shared_ptr<NXKit::BackdropEffectView>& glassView);
    void clampGlassToBounds(
        const std::shared_ptr<NXKit::BackdropEffectView>& glassView
    );

    std::shared_ptr<NXKit::UIImageView> _backgroundImageView;
    std::shared_ptr<NXKit::BackdropEffectView> _clearGlassView;
    std::shared_ptr<NXKit::BackdropEffectView> _regularGlassView;
    NXKit::NXPoint _dragOrigin;
    bool _hasPositionedGlass = false;
};
