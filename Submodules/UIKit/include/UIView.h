#pragma once

#include "CALayer.h"
#include <YGLayout.h>
#include <UIEdgeInsets.h>
#include <UITraitEnvironment.h>
#include <UIViewContentMode.h>
#include <UIResponder.h>
#include <UIFocus.h>
#include <UIEvent.h>
#include <UINib.h>
#include <set>
#include <tinyxml2.h>

namespace NXKit {

#define REGISTER_XIB_ATTRIBUTE(strname, parcer, setter) \
if (name == #strname) { \
auto strname = parcer(value); \
if (!strname.has_value()) return false; \
setter(strname.value()); \
return true;\
}

#define REGISTER_XIB_EDGE_ATTRIBUTE(strname, parcer, setter) \
REGISTER_XIB_ATTRIBUTE(strname, parcer, setter) \
REGISTER_XIB_ATTRIBUTE(strname##Left, parcer, setter##Left) \
REGISTER_XIB_ATTRIBUTE(strname##Top, parcer, setter##Top) \
REGISTER_XIB_ATTRIBUTE(strname##Right, parcer, setter##Right) \
REGISTER_XIB_ATTRIBUTE(strname##Bottom, parcer, setter##Bottom) \
REGISTER_XIB_ATTRIBUTE(strname##Start, parcer, setter##Start) \
REGISTER_XIB_ATTRIBUTE(strname##End, parcer, setter##End) \
REGISTER_XIB_ATTRIBUTE(strname##Horizontal, parcer, setter##Horizontal) \
REGISTER_XIB_ATTRIBUTE(strname##Vertical, parcer, setter##Vertical)

class UIWindow;
class UIViewController;
class UIGestureRecognizer;
class UIView: public UIResponder, public UITraitEnvironment, public CALayerDelegate, public UIFocusItem, public enable_shared_from_this<UIView> {
public:
    std::map<std::string, std::shared_ptr<UIView>> idStorage;

    static std::shared_ptr<UIView> init() { return new_shared<UIView>(); }
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

    void setHidden(bool hidden);
    [[nodiscard]] bool isHidden() const { return _layer->isHidden(); }

    void setClipsToBounds(bool clipsToBounds) { _layer->setMasksToBounds(clipsToBounds); }
    [[nodiscard]] bool clipsToBounds() const { return _layer->masksToBounds(); }

    void setTransform(NXAffineTransform transform) { _layer->setAffineTransform(transform); }
    [[nodiscard]] NXAffineTransform transform() const { return _layer->affineTransform(); }

    void setTransform3D(NXTransform3D transform) { _layer->setTransform(transform); }
    NXTransform3D transform3D() const { return _layer->transform(); }

    void setBackgroundColor(const std::optional<UIColor>& backgroundColor) { _layer->setBackgroundColor(backgroundColor); }
    [[nodiscard]] std::optional<UIColor> backgroundColor() const { return _layer->backgroundColor(); }

    void setTintColor(std::optional<UIColor> tintColor);
    UIColor tintColor() const;
    virtual void tintColorDidChange();

    void setUserInteractionEnabled(bool isUserInteractionEnabled) { _isUserInteractionEnabled = isUserInteractionEnabled; }
    [[nodiscard]] bool isUserInteractionEnabled() const { return _isUserInteractionEnabled; }

    void setContentMode(UIViewContentMode mode);
    [[nodiscard]] UIViewContentMode contentMode() const { return _contentMode; }

    void setMask(const std::shared_ptr<UIView>& mask);
    [[nodiscard]] std::shared_ptr<UIView> mask() const { return _mask; }

    void addGestureRecognizer(const std::shared_ptr<UIGestureRecognizer>& gestureRecognizer);
    [[nodiscard]] std::vector<std::shared_ptr<UIGestureRecognizer>>* gestureRecognizers()  { return &_gestureRecognizers; }

    void addSubview(const std::shared_ptr<UIView>& view);
    void insertSubviewAt(const std::shared_ptr<UIView>& view, int index);
    void insertSubviewBelow(const std::shared_ptr<UIView>& view, const std::shared_ptr<UIView>& belowSubview);
    void removeFromSuperview();

    virtual std::shared_ptr<UIWindow> window();

    const std::vector<std::shared_ptr<UIView>>& subviews() const { return _subviews; }
    std::weak_ptr<UIView> superview() const { return _superview; }

    std::shared_ptr<CALayer> layer() const { return _layer; };

    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;

    // Layout
    UIEdgeInsets layoutMargins();
    void setLayoutMargins(UIEdgeInsets layoutMargins);

    UIEdgeInsets safeAreaInsets() { return _safeAreaInsets; }

    virtual void safeAreaInsetsDidChange() {}
    virtual void layoutMarginsDidChange() {}

    bool insetsLayoutMarginsFromSafeArea() const { return _insetsLayoutMarginsFromSafeArea; }
    void setInsetsLayoutMarginsFromSafeArea(bool insetsLayoutMarginsFromSafeArea);

    bool preservesSuperviewLayoutMargins() const { return _preservesSuperviewLayoutMargins; }
    void setPreservesSuperviewLayoutMargins(bool preservesSuperviewLayoutMargins);

    void setNeedsDisplay() { _needsDisplay = true; }
    void setNeedsLayout();// { setNeedsDisplay(); _needsLayout = true; }

    bool isDescendantOf(const std::shared_ptr<UIView>& view);

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

    static void completePendingAnimations();

    static void performWithoutAnimation(const std::function<void()>& actionsWithoutAnimation);
    
    std::shared_ptr<CABasicAnimation> actionForKey(std::string event) override;
    void updateCurrentEnvironment() override;

    virtual void draw() {}
    void display(std::shared_ptr<CALayer> layer) override;

    virtual bool applyXMLAttribute(const std::string& name, const std::string& value);
private:
    friend class UIViewController;
    friend class UIFocusSystem;
    friend class YGLayout;
    friend class UINib;
    friend bool applicationRunLoop();

    std::shared_ptr<YGLayout> _yoga;
    std::vector<std::shared_ptr<UIGestureRecognizer>> _gestureRecognizers;
    std::vector<std::shared_ptr<UIView>> _subviews;
    std::weak_ptr<UIView> _superview;
    std::shared_ptr<CALayer> _layer;
    std::shared_ptr<UIView> _mask;
    UIViewContentMode _contentMode = UIViewContentMode::scaleToFill;
    std::weak_ptr<UIViewController> _parentController;
    bool _isUserInteractionEnabled = true;
    static int _performWithoutAnimationTick;

    std::optional<UIColor> _tintColor;

    UIEdgeInsets _layoutMargins;
    UIEdgeInsets _calculatedLayoutMargins;
    UIEdgeInsets _safeAreaInsets;
    bool _insetsLayoutMarginsFromSafeArea = true;
    bool _preservesSuperviewLayoutMargins = false;

    void setSafeAreaInsets(UIEdgeInsets safeAreaInsets);
    void updateSafeAreaInsetsInChilds();
    void updateSafeAreaInsetsIfNeeded();
    void updateSafeAreaInsets();
    void updateLayoutMarginIfNeeded();
    void updateLayoutMargin();
    void setNeedsUpdateSafeAreaInsets() { _needsUpdateSafeAreaInsets = true; }
    void setNeedsUpdateLayoutMargins() { _needsUpdateLayoutMargins = true; }

    bool _needsLayout = true;
    bool _needsDisplay = true;
    bool _needsUpdateSafeAreaInsets = true;
    bool _needsUpdateLayoutMargins = true;

    static void animateIfNeeded(Timer currentTime);

    void setSuperview(const std::shared_ptr<UIView>& superview);
    bool anyCurrentlyRunningAnimationsAllowUserInteraction() const;

    std::shared_ptr<UIFocusItem> searchForFocus();

    std::shared_ptr<UIView> layoutRoot();

    static std::shared_ptr<UIView> instantiateFromXib(tinyxml2::XMLElement* element, std::map<std::string, std::shared_ptr<UIView>>* idStorage = nullptr);
    virtual void applyXMLAttributes(tinyxml2::XMLElement* element, std::map<std::string, std::shared_ptr<UIView>>* parsingIdStorage);
};

}
