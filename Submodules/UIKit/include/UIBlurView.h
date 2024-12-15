#pragma once

#include <UIView.h>
#include <CABlurLayer.h>

namespace NXKit {

class UIBlurView: public UIView {
public:
    UIBlurView();

    void setBlurValue(NXFloat blurValue) { _blurLayer()->setBlurValue(blurValue); }
    [[nodiscard]] NXFloat blurValue() const { return _blurLayer()->blurValue(); }

private:
    std::shared_ptr<CABlurLayer> _blurLayer() const;
};

}
