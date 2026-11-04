#pragma once

#include <CABlurLayer.h>
#include <NSDirectionalRectEdge.h>
#include <UIView.h>
#include <UIViewController.h>

#include <functional>
#include <utility>

namespace NXKit::Internal {

// NXKit deliberately renders all requested styles as a hard material for now.
// A single material belongs to each bar, independent of the screen's lifetime.
NXRect navigationScrollPocketMaterialRect(UIViewController& controller,
                                        const std::shared_ptr<UIView>& bar,
                                        NSDirectionalRectEdge edge);

class NavigationScrollPocketLayer final : public CABlurLayer {
public:
    using RegionProvider = std::function<NXRect()>;
    explicit NavigationScrollPocketLayer(RegionProvider regionProvider)
        : _regionProvider(std::move(regionProvider)) {}
    explicit NavigationScrollPocketLayer(NavigationScrollPocketLayer* source)
        : CABlurLayer(source), _regionProvider(source->_regionProvider) {}

    std::shared_ptr<CALayer> copy() override {
        return new_shared<NavigationScrollPocketLayer>(this);
    }
    void draw(SkCanvas* context) override;

private:
    RegionProvider _regionProvider;
};

}
