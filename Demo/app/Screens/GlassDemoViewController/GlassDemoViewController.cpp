#include <Screens/GlassDemoViewController/GlassDemoViewController.hpp>

#include <algorithm>

using namespace NXKit;

namespace {

constexpr NXFloat glassWidth = 120;
constexpr NXFloat glassHeight = 60;
constexpr NXFloat regularGlassWidth = 300;
constexpr NXFloat regularGlassHeight = 160;

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

    _clearGlassView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::clear)
    );
    _clearGlassView->setFrame(NXRect(0, 0, glassWidth, glassHeight));
    _clearGlassView->layer()->setCornerRadius(glassHeight * 0.5f);
    rootView->addSubview(_clearGlassView);
    makeDraggable(_clearGlassView);

    _regularGlassView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::regular)
    );
    _regularGlassView->setFrame(NXRect(
        0,
        0,
        regularGlassWidth,
        regularGlassHeight
    ));
    _regularGlassView->layer()->setCornerRadius(52);
    rootView->addSubview(_regularGlassView);
    makeDraggable(_regularGlassView);

    setView(rootView);
}

void GlassDemoViewController::makeDraggable(
    const std::shared_ptr<BackdropEffectView>& glassView
) {
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
                clampGlassToBounds(
                    std::static_pointer_cast<BackdropEffectView>(glassView)
                );
                break;
            }
            default:
                break;
        }
    };
    glassView->addGestureRecognizer(panGesture);
}

void GlassDemoViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    const auto rootBounds = view()->bounds();
    _backgroundImageView->setFrame(rootBounds);

    if (!_hasPositionedGlass) {
        _clearGlassView->setCenter(NXPoint(
            rootBounds.minX() + rootBounds.width() / 3.0f,
            rootBounds.midY()
        ));
        _regularGlassView->setCenter(NXPoint(
            rootBounds.minX() + rootBounds.width() * 2.0f / 3.0f,
            rootBounds.midY()
        ));
        _hasPositionedGlass = true;
    }
    clampGlassToBounds(_clearGlassView);
    clampGlassToBounds(_regularGlassView);
}

void GlassDemoViewController::clampGlassToBounds(
    const std::shared_ptr<BackdropEffectView>& glassView
) {
    if (!glassView || !viewIsLoaded()) return;

    const auto bounds = view()->bounds();
    auto frame = glassView->frame();
    const auto maximumX = std::max(bounds.minX(), bounds.maxX() - frame.width());
    const auto maximumY = std::max(bounds.minY(), bounds.maxY() - frame.height());
    frame.origin.x = std::clamp(frame.origin.x, bounds.minX(), maximumX);
    frame.origin.y = std::clamp(frame.origin.y, bounds.minY(), maximumY);
    glassView->setFrame(frame);
}
