#pragma once
#include "GlassScreenView.hpp"
class GlassScreenViewController final : public NXKit::UIViewController {
public:
    GlassScreenViewController() { setTitle("iOS screen layout"); }
    void loadView() override;
    void willMoveToParent(const std::shared_ptr<NXKit::UIViewController>& parent) override;
};
