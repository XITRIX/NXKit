#pragma once

#include "CALayer.h"
#include <YGLayout.h>
#include <UITraitEnvironment.h>
#include <UIViewContentMode.h>
#include <UIResponder.h>
#include <UIFocus.h>
#include <UIEvent.h>
#include <set>

namespace NXKit {

class UIWindow;
class UIViewController;
class UIGestureRecognizer;
class UIView: public UIResponder, public UITraitEnvironment, public CALayerDelegate, public UIFocusItem, public enable_shared_from_this<UIView> {
public:
    explicit UIView(NXRect frame = NXRect(), std::shared_ptr<CALayer> layer = new_shared<CALayer>());

    std::string tag;

    std::shared_ptr<UIResponder> next() override;

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

    void setMask(const std::shared_ptr<UIView>& mask);
    [[nodiscard]] std::shared_ptr<UIView> mask() const { return _mask; }

    void addGestureRecognizer(const std::shared_ptr<UIGestureRecognizer>& gestureRecognizer);
    [[nodiscard]] std::vector<std::shared_ptr<UIGestureRecognizer>>* gestureRecognizers()  { return &_gestureRecognizers; }

    virtual void addSubview(std::shared_ptr<UIView> view);
    void insertSubviewAt(std::shared_ptr<UIView> view, int index);
    void insertSubviewBelow(const std::shared_ptr<UIView>& view, const std::shared_ptr<UIView>& belowSubview);
    void removeFromSuperview();

    virtual std::shared_ptr<UIWindow> window();

    const std::vector<std::shared_ptr<UIView>>& subviews() const { return _subviews; }
    std::weak_ptr<UIView> superview() const { return _superview; }

    std::shared_ptr<CALayer> layer() const { return _layer; };

    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;

    // Layout
    void setNeedsDisplay() { _needsDisplay = true; }
    void setNeedsLayout();// { setNeedsDisplay(); _needsLayout = true; }

    void layoutIfNeeded();
    virtual void layoutSubviews();

    virtual NXSize sizeThatFits(NXSize size);
    void sizeToFit();

    // FlexLayout
    std::shared_ptr<YGLayout> yoga() const { return _yoga; }
    void configureLayout(const std::function<void(std::shared_ptr<YGLayout>)>& block);
    void setAutolayoutEnabled(bool enabled) { _yoga->setEnabled(enabled); }

    // Render
    void drawAndLayoutTreeIfNeeded();

    // Touch
    NXPoint convertToView(NXPoint point, const std::shared_ptr<UIView>& toView) const;
    NXPoint convertFromView(NXPoint point, const std::shared_ptr<UIView>& fromView);
    virtual std::shared_ptr<UIView> hitTest(NXPoint point, UIEvent* withEvent);
    virtual bool point(NXPoint insidePoint, UIEvent* withEvent);

    // Focus
    std::shared_ptr<UIFocusEnvironment> parentFocusEnvironment() override;
    virtual std::shared_ptr<UIFocusItem> getNextFocusItem(std::shared_ptr<UIView> current, UIFocusHeading focusHeading);
    bool isFocused() override;

    // Animations
    static std::set<std::shared_ptr<CALayer>> layersWithAnimations;
    static std::shared_ptr<CABasicAnimationPrototype> currentAnimationPrototype;

    static void animate(double duration,
                        double delay = 0.0,
                        UIViewAnimationOptions options = UIViewAnimationOptions::none,
                        const std::function<void()>& animations = [](){},
                        std::function<void(bool)> completion = [](bool res){});

    static void animate(double duration,
                        const std::function<void()>& animations,
                        std::function<void(bool)> completion = [](bool res){});

    static void animate(double duration,
                        double delay,
                        double usingSpringWithDamping,
                        double initialSpringVelocity,
                        UIViewAnimationOptions options = UIViewAnimationOptions::none,
                        const std::function<void()>& animations = [](){},
                        std::function<void(bool)> completion = [](bool res){});

    static void animateIfNeeded(Timer currentTime);
    static void completePendingAnimations();
    
    std::shared_ptr<CABasicAnimation> actionForKey(std::string event) override;
    virtual void draw() {}
    virtual void display(std::shared_ptr<CALayer> layer) override;
private:
    friend class UIViewController;
    friend class UIFocusSystem;
    friend class YGLayout;

    std::shared_ptr<YGLayout> _yoga;
    std::vector<std::shared_ptr<UIGestureRecognizer>> _gestureRecognizers;
    std::vector<std::shared_ptr<UIView>> _subviews;
    std::weak_ptr<UIView> _superview;
    std::shared_ptr<CALayer> _layer;
    std::shared_ptr<UIView> _mask;
    UIViewContentMode _contentMode = UIViewContentMode::scaleToFill;
    std::weak_ptr<UIViewController> _parentController;
    bool _isUserInteractionEnabled = true;

    bool _needsLayout = true;
    bool _needsDisplay = true;

    void setSuperview(const std::shared_ptr<UIView>& superview);
    bool anyCurrentlyRunningAnimationsAllowUserInteraction() const;

    std::shared_ptr<UIFocusItem> searchForFocus();

    std::shared_ptr<UIView> layoutRoot();
};

}
