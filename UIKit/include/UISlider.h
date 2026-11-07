#pragma once
#include <UIControl.h>
#include <UISliderTrackConfiguration.h>
#include <map>
namespace NXKit {
class LensView;
class UIImageView;
class UILabel;
enum class UISliderStyle { defaultStyle, thumbless };
/** Apple-styled horizontal value control. Programmatic setters never send
    valueChanged. Touches send continuously, or once on successful release
    when continuous is false. Select/arrow input is also available on consoles. */
class UISlider : public UIControl {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UISlider>(); }
    explicit UISlider(NXRect frame = {}, std::optional<UIAction> primaryAction = std::nullopt);
    ~UISlider() override;
    NXFloat value() const { return _value; }
    void setValue(NXFloat value, bool animated = false);
    NXFloat minimumValue() const { return _minimumValue; }
    void setMinimumValue(NXFloat value);
    NXFloat maximumValue() const { return _maximumValue; }
    void setMaximumValue(NXFloat value);
    bool isContinuous() const { return _continuous; }
    void setContinuous(bool value) { _continuous=value; }
    UISliderStyle sliderStyle() const { return _style; }
    void setSliderStyle(UISliderStyle style);
    // Configuration has value semantics; edit a returned copy and reassign it.
    std::shared_ptr<UISliderTrackConfiguration> trackConfiguration() const { return _configuration?_configuration->copy():nullptr; }
    void setTrackConfiguration(const std::shared_ptr<UISliderTrackConfiguration>& configuration);

    std::shared_ptr<UIImage> minimumValueImage() const;
    void setMinimumValueImage(const std::shared_ptr<UIImage>& image);
    std::shared_ptr<UIImage> maximumValueImage() const;
    void setMaximumValueImage(const std::shared_ptr<UIImage>& image);
    const std::optional<UIColor>& minimumTrackTintColor() const { return _minimumTrackTint; }
    void setMinimumTrackTintColor(std::optional<UIColor> color);
    const std::optional<UIColor>& maximumTrackTintColor() const { return _maximumTrackTint; }
    void setMaximumTrackTintColor(std::optional<UIColor> color);
    const std::optional<UIColor>& thumbTintColor() const { return _thumbTint; }
    void setThumbTintColor(std::optional<UIColor> color);
    void setThumbImage(const std::shared_ptr<UIImage>& image, UIControlState state);
    void setMinimumTrackImage(const std::shared_ptr<UIImage>& image, UIControlState state);
    void setMaximumTrackImage(const std::shared_ptr<UIImage>& image, UIControlState state);
    std::shared_ptr<UIImage> thumbImageForState(UIControlState state) const;
    std::shared_ptr<UIImage> minimumTrackImageForState(UIControlState state) const;
    std::shared_ptr<UIImage> maximumTrackImageForState(UIControlState state) const;
    std::shared_ptr<UIImage> currentThumbImage() const;
    std::shared_ptr<UIImage> currentMinimumTrackImage() const;
    std::shared_ptr<UIImage> currentMaximumTrackImage() const;

    virtual NXRect minimumValueImageRectForBounds(NXRect bounds) const;
    virtual NXRect maximumValueImageRectForBounds(NXRect bounds) const;
    virtual NXRect trackRectForBounds(NXRect bounds) const;
    virtual NXRect thumbRectForBounds(NXRect bounds, NXRect trackRect, NXFloat value) const;
    NXSize sizeThatFits(NXSize size) override;
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
    virtual void accessibilityIncrement();
    virtual void accessibilityDecrement();
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;
protected:
    void controlStateDidChange() override;
    bool requiresExclusiveTouchTracking() const override { return true; }
    bool capturesUndelayedDrag(const std::shared_ptr<UITouch>& touch) const override;
    void contactFeedbackBegan(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) override;
    void contactFeedbackCancelled() override;
    bool deferEndTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) override;
    void trackingDidEnd() override;
private:
    friend class UISliderTestHarness;
    NXFloat _value=0, _minimumValue=0, _maximumValue=1;
    bool _continuous=true, _pressed=false, _contactFeedback=false;
    UISliderStyle _style=UISliderStyle::defaultStyle;
    std::optional<UIColor> _minimumTrackTint, _maximumTrackTint, _thumbTint;
    std::shared_ptr<UISliderTrackConfiguration> _configuration;
    std::map<UIControlState,std::shared_ptr<UIImage>> _thumbImages, _minimumTrackImages, _maximumTrackImages;
    std::shared_ptr<UIView> _visuals, _track, _minimumTrack, _maximumTrack;
    std::shared_ptr<UIImageView> _minimumImage, _maximumImage, _thumbImage, _minimumTrackImage, _maximumTrackImage;
    std::shared_ptr<LensView> _lens;
    std::vector<std::shared_ptr<UIView>> _ticks;
    NXFloat _trackingBaseValue=0;
    NXPoint _dragOrigin;
    bool _dragging=false;
    NXPoint _lastTouch;
    NXSize _lastLayoutSize;
    void updateAppearance();
    void updateThumb();
    void stopValueAnimation();
    void setPressed(bool pressed, bool animated);
    void updateFromTouch(const std::shared_ptr<UITouch>& touch);
    NXFloat fraction(NXFloat value) const;
    NXFloat constrainedValue(NXFloat value, bool interactive) const;
    void rebuildTicks();
    void adjustValue(NXFloat direction);
    struct Tracking;
    std::unique_ptr<Tracking> _interaction;
    void advanceRelease(double seconds);
    void advanceThumb(double seconds);
};
}
