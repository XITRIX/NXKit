#pragma once
#include <UIKit.h>

// The Glass tab is a gallery. Each button pushes a dedicated scene onto the
// existing navigation stack, so Back/B returns to the same gallery position.
class GlassDemoViewController final : public NXKit::UIViewController {
public:
    GlassDemoViewController();
    void loadView() override;
    void viewSafeAreaInsetsDidChange() override;
    void viewDidLayoutSubviews() override;
private:
    std::shared_ptr<NXKit::UIScrollView> _scroll;
    std::shared_ptr<NXKit::UIView> _contentContainer;
};
