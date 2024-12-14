#pragma once

#include "CALayer.h"

namespace NXKit {

class UIView: public enable_shared_from_this<UIView> {
public:
    UIView();

    void setFrame(NXRect frame);
    NXRect frame() const { return _layer->getFrame(); }

    void setBounds(NXRect bounds);
    NXRect bounds() const { return _layer->bounds(); }

    void setCenter(NXPoint position);
    NXPoint center() const;

    void setAlpha(NXFloat alpha) { _layer->setOpacity(alpha); }
    NXFloat alpha() const { return _layer->opacity(); }

    void setHidden(bool hidden) { _layer->setHidden(hidden); }
    bool isHidden() const { return _layer->isHidden(); }

    void setClipsToBounds(bool clipsToBounds) { _layer->setMasksToBounds(clipsToBounds); }
    bool clipsToBounds() const { return _layer->masksToBounds(); }

    void setTransform(NXAffineTransform transform) { _layer->setAffineTransform(transform); }
    NXAffineTransform transform() const { return _layer->affineTransform(); }

    void setBackgroundColor(std::optional<UIColor> backbroundColor) { _layer->setBackgroundColor(backbroundColor); }
    std::optional<UIColor> backgroundColor() const { return _layer->backgroundColor(); }

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
