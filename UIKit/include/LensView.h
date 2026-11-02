#pragma once

#include <UIView.h>
#include <optional>
#include <utility>

namespace NXKit {
class BackdropEffectView;
class UITouch;
enum class UITouchPhase;

/** NXKit extension for a resting selection/knob that becomes a clear lens.
    This is not a public Apple UIKit API. The caller owns position and size.
    Add selected content to contentView(), or supply a full underlying row with
    setLiftedContentView(). The lifted lens refracts that source near its edges
    before clipping the finished output to its capsule.
    Normal content and selection routing
    belong to the containing control. Elastic motion is opt-in and uses a
    measured portable approximation; it does not claim exact UIKit physics. */
class LensView : public UIView {
public:
  enum class Style { regular = 0, selection = 1 };
  enum class InteractionSource { contact, viewMotion };
  static std::shared_ptr<UIView> init() { return new_shared<LensView>(); }
  explicit LensView(Style style = Style::regular);
  ~LensView() override;

  [[nodiscard]] Style style() const { return _style; }
  void setStyle(Style style);
  [[nodiscard]] bool isLifted() const { return _lifted; }
  void setLifted(bool lifted, bool animated = false);

  /** Color-dependent foreground blending for a semantic selected source.
      Defaults false, preserving caller RGB ink. The containing control may
      enable it for system-colored icons/labels, independently of lift. */
  [[nodiscard]] bool isContentVibrancyEnabled() const { return _contentVibrancy; }
  void setContentVibrancyEnabled(bool enabled);

  /** Optional source magnification about the lens center, interpolated with
      lift progress. Defaults to 1, matching a standalone native lens whose
      caller has not enlarged its source. Must be finite and at least 1.
      Controls may instead scale individual source items about their own
      centers, as UIKit's tab bar does; do not apply both to the same content. */
  [[nodiscard]] NXFloat liftedContentMagnification() const { return _liftedContentMagnification; }
  void setLiftedContentMagnification(NXFloat magnification);

  /** Opt-in elastic contact response. The containing control continues to own
      selection, model frame, and lifted state. Explicit hooks also work when
      this view's ordinary user interaction is disabled. */
  [[nodiscard]] bool isInteractive() const;
  void setInteractive(bool interactive);
  [[nodiscard]] InteractionSource interactionSource() const;
  /** Exclusive force source: contact supports stationary knobs; viewMotion
      samples caller-controlled global presentation geometry. Contact-driven
      deformation is an NXKit extension; native calibration uses viewMotion. */
  void setInteractionSource(InteractionSource source);
  /** Contact location and velocity are in the current superview's coordinates
      (points and points/second). Keep that coordinate space stable for the
      contact lifetime; removal/reparenting cancels the elastic response. */
  void beginInteraction(NXPoint location);
  /** The one-argument update estimates velocity from a monotonic clock;
      provide velocity explicitly for gesture recognizers and deterministic replay. */
  void updateInteraction(NXPoint location);
  void updateInteraction(NXPoint location, NXPoint velocity);
  void endInteraction(NXPoint velocity = {}, bool cancelled = false);
  /** Additional visual-only affine in lens-local coordinates. This is the
      same value used by the root presentation layer, before bounds origin,
      so a containing control can align its content punchout exactly. */
  [[nodiscard]] NXAffineTransform interactionTransform() const;

  [[nodiscard]] std::shared_ptr<UIView> contentView() const { return _content; }
  /** Optional full selected-content source. Stored weakly; nullptr uses
      contentView(). The source must share a view ancestor with the lens and
      must not contain it. Its complete presented bounds are sampled before
      the finished lens is clipped. Source ancestor visibility and opacity
      remain effective; their output masks do not preclip the optical input.
      A live lens suppresses the source's ordinary rendering so translucent
      and antialiased ink is composited once. */
  void setLiftedContentView(const std::shared_ptr<UIView> &source);
  [[nodiscard]] std::shared_ptr<UIView> liftedContentView() const {
    return _liftedContent.lock();
  }
  /** Clear by default. The caller supplies the resting fill/content. Its
      opacity is preserved while an internal host fades it during lifting. */
  [[nodiscard]] std::shared_ptr<UIView> restingBackgroundView() const {
    return _resting;
  }
  [[nodiscard]] std::optional<UIColor> restingBackgroundColor() const;
  void setRestingBackgroundColor(std::optional<UIColor> color);

  void setFrame(NXRect frame) override;
  void setBounds(NXRect bounds) override;
  void layoutSubviews() override;
  void traitCollectionDidChange(
      std::shared_ptr<UITraitCollection> previous) override;
  bool applyXMLAttribute(const std::string &name,
                         const std::string &value) override;

private:
  friend class LensViewTestHarness;
  struct Animation;
  void updateLayout();
  void updateMaterial();
  void updateDisplayScale();
  void advance(double seconds);
  void wakeAnimation();
  void prepareViewMotion();
  std::optional<std::pair<NXPoint,NXAffineTransform>> motionPose();
  void resetInteraction();
  void applyInteractionTransform(NXAffineTransform transform);
  void observeTouch(UITouchPhase phase, const std::shared_ptr<UITouch>& touch);
  void setSourceOwnership(const std::shared_ptr<CALayer> &source, bool owned);
  Style _style;
  bool _lifted = false;
  bool _contentVibrancy = false;
  NXFloat _progress = 0;
  NXFloat _liftedContentMagnification = 1;
  NXFloat _resolvedDisplayScale = 0;
  std::shared_ptr<UIView> _restingHost;
  std::shared_ptr<UIView> _resting;
  std::shared_ptr<UIView> _content;
  std::shared_ptr<BackdropEffectView> _optics;
  std::shared_ptr<UIView> _contact;
  std::weak_ptr<UIView> _liftedContent;
  std::weak_ptr<CALayer> _registeredSource;
  std::unique_ptr<Animation> _animation;
};
} // namespace NXKit
