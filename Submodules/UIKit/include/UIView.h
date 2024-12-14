#pragma once

#include "CALayer.h"
#include <UIViewContentMode.h>
#include <set>

namespace NXKit {

class UIView: public CALayerDelegate, public enable_shared_from_this<UIView> {
public:
    UIView(): UIView(NXRect()) {}
    UIView(NXRect frame);

    void setFrame(NXRect frame);
    [[nodiscard]] NXRect frame() const { return _layer->getFrame(); }

    void setBounds(NXRect bounds);
    [[nodiscard]] NXRect bounds() const { return _layer->bounds(); }

    void setCenter(NXPoint position);
    [[nodiscard]] NXPoint center() const;

    void setAlpha(NXFloat alpha) { _layer->setOpacity(alpha); }
    [[nodiscard]] NXFloat alpha() const { return _layer->opacity(); }

    void setHidden(bool hidden) { _layer->setHidden(hidden); }
    [[nodiscard]] bool isHidden() const { return _layer->isHidden(); }

    void setClipsToBounds(bool clipsToBounds) { _layer->setMasksToBounds(clipsToBounds); }
    [[nodiscard]] bool clipsToBounds() const { return _layer->masksToBounds(); }

    void setTransform(NXAffineTransform transform) { _layer->setAffineTransform(transform); }
    [[nodiscard]] NXAffineTransform transform() const { return _layer->affineTransform(); }

    void setBackgroundColor(std::optional<UIColor> backbroundColor) { _layer->setBackgroundColor(backbroundColor); }
    [[nodiscard]] std::optional<UIColor> backgroundColor() const { return _layer->backgroundColor(); }

    void setUserInteractionEnabled(bool isUserInteractionEnabled) { _isUserInteractionEnabled = isUserInteractionEnabled; }
    [[nodiscard]] bool isUserInteractionEnabled() const { return _isUserInteractionEnabled; }

    void setContentMode(UIViewContentMode mode);
    [[nodiscard]] UIViewContentMode contentMode() const { return _contentMode; }

    virtual void addSubview(std::shared_ptr<UIView> view);
    void insertSubviewAt(std::shared_ptr<UIView> view, int index);
    void insertSubviewBelow(std::shared_ptr<UIView> view, std::shared_ptr<UIView> belowSubview);
    void removeFromSuperview();

//    virtual std::shared_ptr<UIWindow> window();

    const std::vector<std::shared_ptr<UIView>>& subviews() const { return _subviews; }
    std::weak_ptr<UIView> superview() const { return _superview; }

    virtual std::shared_ptr<CALayer> initLayer();
    std::shared_ptr<CALayer> layer() const { return _layer; };


    // Animations
    static std::set<std::shared_ptr<CALayer>> layersWithAnimations;
    static std::shared_ptr<CABasicAnimationPrototype> currentAnimationPrototype;

    static void animate(double duration,
                        double delay = 0.0,
                        UIViewAnimationOptions options = UIViewAnimationOptions::none,
                        std::function<void()> animations = [](){},
                        std::function<void(bool)> completion = [](bool res){});

    static void animate(double duration,
                        std::function<void()> animations = [](){});

    static void animate(double duration,
                        double delay,
                        double usingSpringWithDamping,
                        double initialSpringVelocity,
                        UIViewAnimationOptions options = UIViewAnimationOptions::none,
                        std::function<void()> animations = [](){},
                        std::function<void(bool)> completion = [](bool res){});

    static void animateIfNeeded(Timer currentTime);
    static void completePendingAnimations();
    
    std::shared_ptr<CABasicAnimation> actionForKey(std::string event) override;
//    virtual void draw() {}
    virtual void display(std::shared_ptr<CALayer> layer) override;
private:
    std::vector<std::shared_ptr<UIView>> _subviews;
    std::weak_ptr<UIView> _superview;
    std::shared_ptr<CALayer> _layer;
    UIViewContentMode _contentMode;

    bool _isUserInteractionEnabled = true;

    void setSuperview(std::shared_ptr<UIView> superview);
    bool anyCurrentlyRunningAnimationsAllowUserInteraction();
};

}
