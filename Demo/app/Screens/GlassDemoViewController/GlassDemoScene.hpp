#pragma once
#include <UIKit.h>
#include "GlassDemoSpec.h"

/** The actual interactive demo stage, also used unchanged by raster comparisons.
 Its logical size is 720 x 440. The gallery scales the stage to available space. */
class GlassDemoScene final : public NXKit::UIView {
public:
    GlassDemoScene(GlassDemoID demo, const std::shared_ptr<NXKit::UIImage>& artwork);
    void setProgress(float progress);
    float progress() const { return _progress; }
    void setBackdrop(int backdrop); // 0 artwork, 1 gray, 2 black, 3 white, 4 split
    int backdrop() const { return _backdrop; }
    void reset();
    void setReversedOrder(bool reversed);
    bool reversedOrder() const { return _reversedOrder; }
    void layoutSubviews() override;
    GlassDemoID demo() const { return _demo; }
    const std::vector<std::shared_ptr<NXKit::BackdropEffectView>>& glassItems() const { return _items; }
private:
    void makeDraggable(const std::shared_ptr<NXKit::BackdropEffectView>& item);
    GlassDemoID _demo;
    float _progress=0;
    int _backdrop=0;
    bool _reversedOrder=false;
    std::shared_ptr<NXKit::UIImageView> _artwork;
    std::vector<std::shared_ptr<NXKit::UIView>> _panels;
    std::shared_ptr<NXKit::GlassContainerView> _container;
    std::vector<std::shared_ptr<NXKit::BackdropEffectView>> _items;
    std::vector<std::shared_ptr<NXKit::UIView>> _symbols;
};
