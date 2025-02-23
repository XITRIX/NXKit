#pragma once

#include <UIView.h>
#include <CABlurLayer.h>

namespace NXKit {

class UIBlurView: public UIView {
public:
    UIBlurView();

    void setBlurRadius(NXFloat blurValue) { _blurLayer()->setBlurRadius(blurValue); }
    [[nodiscard]] NXFloat blurRadius() const { return _blurLayer()->blurRadius(); }

    void setBackgroundTintColor(const UIColor& backgroundTintColor) { _blurLayer()->setBackgroundTintColor(backgroundTintColor); }
    [[nodiscard]] UIColor backgroundTintColor() const { return _blurLayer()->backgroundTintColor(); }

private:
    std::shared_ptr<CABlurLayer> _blurLayer() const;
};

}
