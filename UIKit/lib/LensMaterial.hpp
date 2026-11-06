#pragma once
#include <BackdropEffect.h>
#include <CALayer.h>
#include <UIView.h>

namespace NXKit::Internal {
// Recovered native lens settings and independently translated pass equations.
BackdropEffect lensMaterial(bool selection, NXFloat liftProgress,
                            bool dark = false, NXSize size = {});
class LensLayer final : public CALayer {
public:
  LensLayer() = default;
  explicit LensLayer(LensLayer *source);
  std::shared_ptr<CALayer> copy() override;
  void draw(SkCanvas *canvas) override;
  void setLiftState(bool selection, NXFloat progress, NXFloat magnification, bool vibrant, bool dark);
  void setSource(const std::shared_ptr<UIView>& source, const std::shared_ptr<UIView>& owner);
  void drawSource(SkCanvas* canvas,bool isolateForeground=true,bool cropSource=true);

private:
  std::weak_ptr<UIView> _source, _owner;
  bool _selection = false;
  NXFloat _progress = 0;
  NXFloat _magnification = 1;
  bool _vibrant = false, _dark = false;
};
} // namespace NXKit::Internal
