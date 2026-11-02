#pragma once

#include <BackdropEffect.h>
#include <CALayer.h>

namespace NXKit {
/** Low-level rendering extension. Applies a custom backdrop shader without
    installing any component styling, glass material, or input feedback. */
class CABackdropLayer : public CALayer {
public:
    explicit CABackdropLayer(const BackdropEffect& effect);
    explicit CABackdropLayer(CABackdropLayer* source);
    ~CABackdropLayer() override;
    [[nodiscard]] BackdropEffect effect() const;
    void setEffect(const BackdropEffect& effect);
    std::shared_ptr<CALayer> copy() override;
    void draw(SkCanvas* canvas) override;
private:
    struct State;
    std::shared_ptr<State> _state;
};
}
