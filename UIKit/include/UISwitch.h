#pragma once

#include <UIControl.h>

namespace NXKit {
class LensView;
class UIImage;

enum class UISwitchStyle { automatic = 0, checkbox = 1, sliding = 2 };

/** Apple-styled on/off control. Programmatic changes never send valueChanged.
    The portable implementation resolves all preferred styles to sliding,
    matching UIKit outside the Catalyst Mac idiom. Catalyst-only title and
    checkbox presentation are intentionally outside this API subset. */
class UISwitch : public UIControl {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UISwitch>(); }
    explicit UISwitch(NXRect frame = {}, std::optional<UIAction> primaryAction = std::nullopt);
    ~UISwitch() override;

    [[nodiscard]] bool isOn() const { return _on; }
    void setOn(bool on, bool animated = false);
    [[nodiscard]] const std::optional<UIColor>& onTintColor() const { return _onTintColor; }
    void setOnTintColor(std::optional<UIColor> color);
    [[nodiscard]] const std::optional<UIColor>& thumbTintColor() const { return _thumbTintColor; }
    void setThumbTintColor(std::optional<UIColor> color);
    [[nodiscard]] std::shared_ptr<UIImage> onImage() const { return _onImage; }
    void setOnImage(std::shared_ptr<UIImage> image);
    [[nodiscard]] std::shared_ptr<UIImage> offImage() const { return _offImage; }
    void setOffImage(std::shared_ptr<UIImage> image);
    [[nodiscard]] UISwitchStyle style() const { return UISwitchStyle::sliding; }
    [[nodiscard]] UISwitchStyle preferredStyle() const { return _preferredStyle; }
    void setPreferredStyle(UISwitchStyle style);

    NXSize sizeThatFits(NXSize size) override;
    void sizeToFit() override;
    void setFrame(NXRect frame) override;
    void setBounds(NXRect bounds) override;
    bool point(NXPoint point, UIEvent* event) override;
    std::shared_ptr<UIView> hitTest(NXPoint point, UIEvent* event) override;
    bool beginTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) override;
    bool continueTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) override;
    void endTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) override;
    void cancelTracking(const std::shared_ptr<UIEvent>& event = nullptr) override;
    void layoutSubviews() override;
    void tintColorDidChange() override;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) override;
    /** Portable activation hook until the platform accessibility bridge exists. */
    virtual bool accessibilityActivate();
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;
protected:
    void controlStateDidChange() override;
    bool requiresExclusiveTouchTracking() const override { return true; }
private:
    friend class UISwitchTestHarness;
    friend class UISwitchGestureRecognizer;
    bool _on = false;
    UISwitchStyle _preferredStyle = UISwitchStyle::automatic;
    std::optional<UIColor> _onTintColor, _thumbTintColor;
    std::shared_ptr<UIImage> _onImage, _offImage;
    std::shared_ptr<UIView> _visuals, _track;
    std::shared_ptr<LensView> _lens;
    std::weak_ptr<UIGestureRecognizer> _switchGesture;
    NXPoint _dragOrigin;
    NXFloat _trackingStartPosition = 0, _position = 0;
    bool _dragged = false, _acceptedContact = false;
    void updateAppearance();
    void updateGeometry();
    void setPressed(bool pressed, bool animated);
    void activate();
    struct Motion;
    std::unique_ptr<Motion> _motion;
    void wakeAnimation();
    void advance(double seconds);
};
}
