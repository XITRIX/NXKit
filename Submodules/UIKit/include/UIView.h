#pragma once

#include "CALayer.h"

namespace NXKit {

class UIView: public enable_shared_from_this<UIView> {
public:
    UIView();

    virtual void addSubview(std::shared_ptr<UIView> view);
    void insertSubviewAt(std::shared_ptr<UIView> view, int index);
    void insertSubviewBelow(std::shared_ptr<UIView> view, std::shared_ptr<UIView> belowSubview);
    void removeFromSuperview();

//    virtual std::shared_ptr<UIWindow> window();

    const std::vector<std::shared_ptr<UIView>>& subviews() const { return _subviews; }
    std::weak_ptr<UIView> superview() const { return _superview; }

    std::shared_ptr<CALayer> layer() const { return _layer; };
private:
    std::vector<std::shared_ptr<UIView>> _subviews;
    std::weak_ptr<UIView> _superview;
    std::shared_ptr<CALayer> _layer;

    void setSuperview(std::shared_ptr<UIView> superview);
};

}