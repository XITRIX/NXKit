//
// Created by Даниил Виноградов on 24.02.2025.
//

#include <NXControl.h>

using namespace NXKit;

void NXControl::didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) {
    UIControl::didUpdateFocusIn(context, coordinator);

    if (context.nextFocusedItem().lock() == shared_from_this()) {
        willGainFocus();
        primaryAction->perform();
    } else {
        willLoseFocus();
    }
}

void NXControl::willGainFocus() {
    layer()->setShadowColor(UIColor::black);
    layer()->setShadowOpacity(0.4);
    layer()->setShadowOffset({0, 6});
    layer()->setShadowRadius(8);
    layer()->setZPosition(1);
    layer()->setBorderColor(UIColor::tint);
    setBackgroundColor(UIColor::secondarySystemBackground);
}

void NXControl::willLoseFocus() {
    layer()->setShadowOpacity(0);
    layer()->setShadowRadius(0);
    layer()->setZPosition(0);
    layer()->setBorderColor(std::nullopt);
    setBackgroundColor(std::nullopt);
}

void NXControl::focusFailedToMove(UIFocusHeading heading) {
    NXFloat x = 0;
    NXFloat y = 0;
    NXFloat power = 10;

    switch (heading) {
        case UIFocusHeading::up:
            y = -1;
            break;
        case UIFocusHeading::down:
            y = 1;
            break;
        case UIFocusHeading::left:
            x = -1;
            break;
        case UIFocusHeading::right:
            x = 1;
            break;
        default:
            break;
    }

    setTransform(NXAffineTransform::translationBy(x * power, y * power));
    UIView::animate(0.3, 0, 0.4, 3, UIViewAnimationOptions::allowUserInteraction, [this]() {
        setTransform(NXAffineTransform::identity);
    });
}

void NXControl::willChangeFocusHighlight(bool highlighted) { }

void NXControl::willChangeHighlight(bool highlighted) {
    setAlpha(highlighted ? 0.8 : 1);

    if (highlighted) {
        setBackgroundColor(UIColor::tint.withAlphaComponent(0.2f));
    } else {
        setBackgroundColor(UIColor::clear);
    }
}