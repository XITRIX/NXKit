#pragma once
#include <BackdropEffect.h>
#include <CALayer.h>
#include <UIView.h>

namespace NXKit::Internal {
// Independent approximation calibrated against the native lens fixture.
BackdropEffect lensMaterial(bool selection, NXFloat liftProgress,
                            bool dark = false);
class LensLayer final : public CALayer {
public:
  LensLayer() = default;
  explicit LensLayer(LensLayer *source);
  std::shared_ptr<CALayer> copy() override;
  void draw(SkCanvas *canvas) override;
  void setLiftState(bool selection, NXFloat progress);
  void setSource(const std::shared_ptr<UIView>& source, const std::shared_ptr<UIView>& owner);
  void drawSource(SkCanvas* canvas);

private:
  std::weak_ptr<UIView> _source, _owner;
  bool _selection = false;
  NXFloat _progress = 0;
};
} // namespace NXKit::Internal
