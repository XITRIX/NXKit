#pragma once

#include <UIViewController.h>
#include <UIControl.h>

namespace NXKit { class UISlider; class UISwitch; }

/** Demo-only navigation destinations for exercising the public UIKit controls. */
class SystemControlsDemoViewController final:public NXKit::UIViewController {
public:
    enum class Page { catalog, slider, switchControl };
    explicit SystemControlsDemoViewController(Page page=Page::catalog);
    void loadView() override;
    std::shared_ptr<NXKit::UIControl> action(const std::string& identifier);
    std::shared_ptr<NXKit::UISlider> slider();
    std::shared_ptr<NXKit::UISwitch> switchControl();
private:
    Page _page;
};
