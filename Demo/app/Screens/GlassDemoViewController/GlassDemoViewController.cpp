#include <Screens/GlassDemoViewController/GlassDemoViewController.hpp>

#include <algorithm>

using namespace NXKit;

namespace {

constexpr NXFloat glassWidth = 120;
constexpr NXFloat glassHeight = 60;

}

GlassDemoViewController::GlassDemoViewController() {
    setTitle("Glass");
}

void GlassDemoViewController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::black);
    rootView->setClipsToBounds(true);

    _backgroundImageView = new_shared<UIImageView>(
        UIImage::fromRes("img/glass_demo_background.png")
    );
    _backgroundImageView->setContentMode(UIViewContentMode::scaleAspectFill);
    rootView->addSubview(_backgroundImageView);

    _glassView = new_shared<BackdropEffectView>(BackdropEffect::glass());
    _glassView->setFrame(NXRect(0, 0, glassWidth, glassHeight));
    _glassView->layer()->setCornerRadius(glassHeight * 0.5f);
    rootView->addSubview(_glassView);

    auto panGesture = new_shared<UIPanGestureRecognizer>();
    panGesture->onStateChanged = [this](const auto& gesture) {
        const auto pan = std::static_pointer_cast<UIPanGestureRecognizer>(gesture);
        const auto glassView = pan->view().lock();
        if (!glassView) return;

        switch (pan->state()) {
            case UIGestureRecognizerState::began:
                _dragOrigin = glassView->frame().origin;
                break;
            case UIGestureRecognizerState::changed: {
                auto frame = glassView->frame();
                frame.origin = _dragOrigin + pan->translationInView(view());
                glassView->setFrame(frame);
                clampGlassToBounds();
                break;
            }
            default:
                break;
        }
    };
    _glassView->addGestureRecognizer(panGesture);

    setView(rootView);
}

void GlassDemoViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    const auto rootBounds = view()->bounds();
    _backgroundImageView->setFrame(rootBounds);

    if (!_hasPositionedGlass) {
        _glassView->setCenter(NXPoint(rootBounds.midX(), rootBounds.midY()));
        _hasPositionedGlass = true;
    }
    clampGlassToBounds();
}

void GlassDemoViewController::clampGlassToBounds() {
    if (!_glassView || !viewIsLoaded()) return;

    const auto bounds = view()->bounds();
    auto frame = _glassView->frame();
    const auto maximumX = std::max(bounds.minX(), bounds.maxX() - frame.width());
    const auto maximumY = std::max(bounds.minY(), bounds.maxY() - frame.height());
    frame.origin.x = std::clamp(frame.origin.x, bounds.minX(), maximumX);
    frame.origin.y = std::clamp(frame.origin.y, bounds.minY(), maximumY);
    _glassView->setFrame(frame);
}
