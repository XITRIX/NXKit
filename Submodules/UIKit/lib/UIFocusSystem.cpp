//
//  UIFocusSystem.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 02.04.2023.
//

#include <UIFocusSystem.h>
#include <UIPressesEvent.h>
#include <UIPress.h>
#include <UIWindow.h>
#include <UIViewController.h>

namespace NXKit {

UIFocusSystem::UIFocusSystem() = default;

void UIFocusSystem::sendEvent(const std::shared_ptr<UIEvent>& event) {
    auto pevent = std::dynamic_pointer_cast<UIPressesEvent>(event);
    if (pevent == nullptr) return;

    std::shared_ptr<UIPress> press;
    for (const auto& _press: pevent->allPresses()) {
        if (_press->type() == UIPressType::select) {
            press = _press;
        }
    }

    if (press != nullptr && !focusedItem().expired()) {
        auto focusedView = std::static_pointer_cast<UIView>(focusedItem().lock());
        if (focusedView) {
            if (press->phase() == UIPressPhase::began) {
                focusedView->pressesBegan(pevent->allPresses(), pevent);
                _selectedFocusedItem = focusedView;
            } else if (press->phase() == UIPressPhase::ended) {
                focusedView->pressesEnded(pevent->allPresses(), pevent);
                _selectedFocusedItem.reset();
            }
        }
        return;
    }

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _focusedItem;
    context._focusHeading = makeFocusHeadingFromEvent(pevent);

    if (context._focusHeading == UIFocusHeading::none) return;

    std::weak_ptr<UIFocusItem> nextItem;
    if (focusedItem().expired()) nextItem = _rootWindow.lock()->searchForFocus();
    else {
        auto current = std::dynamic_pointer_cast<UIView>(focusedItem().lock());
        if (current->superview().expired()) {
            nextItem = _rootWindow.lock()->searchForFocus();
        } else {
            std::shared_ptr<UIFocusItem> potencianNextItem;
            while (true) {
                potencianNextItem = current->superview().lock()->getNextFocusItem(current, context._focusHeading);
                if (!potencianNextItem) { // if no next item to focus
                    if (!current->superview().lock()->superview().expired()) { // but item has parent, check parent
                        current = current->superview().lock();
                        continue;
                    } else { // else stop searching
                        break;
                    }
                }

                context._nextFocusedItem = potencianNextItem;

                bool currentIsFine = context.previouslyFocusedItem().expired() || context.previouslyFocusedItem().lock()->shouldUpdateFocusIn(context);
                bool nextIsFine = context.nextFocusedItem().expired() || context.nextFocusedItem().lock()->shouldUpdateFocusIn(context);

                if (currentIsFine && nextIsFine) { break; }
            }
            nextItem = potencianNextItem;
        }
    }

    if (nextItem.expired()) return;

    context._nextFocusedItem = nextItem;
    applyFocusToItem(nextItem.lock(), context);
}

void UIFocusSystem::updateFocus() {
    auto item = _rootWindow.lock()->searchForFocus();

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _focusedItem;
    context._nextFocusedItem = item;
    context._focusHeading = UIFocusHeading::none;

    applyFocusToItem(item, context);
}

std::shared_ptr<UIFocusItem> UIFocusSystem::findItemToFocus() {
    auto vc = _rootWindow.lock()->rootViewController();
    while (!vc->children().empty()) {
        vc = vc->children().front();
    }
    return vc->view();
}

void UIFocusSystem::applyFocusToItem(const std::shared_ptr<UIFocusItem>& item, UIFocusUpdateContext context) {
    UIFocusAnimationCoordinator coordinator;
    _focusedItem = item;

    if (!context.previouslyFocusedItem().expired()) { context.previouslyFocusedItem().lock()->didUpdateFocusIn(context, &coordinator); }
    if (!context.nextFocusedItem().expired()) { context.nextFocusedItem().lock()->didUpdateFocusIn(context, &coordinator); }

    UIFocusAnimationContext animationContext;
    UIView::animate(animationContext.duration(), 0, curveEaseOut, [context, coordinator, animationContext]() {
        for (const auto& animation: coordinator._coordinatedAnimations) { animation(); }
        for (const auto& animation: coordinator._coordinatedFocusingAnimations) { animation(animationContext); }
        for (const auto& animation: coordinator._coordinatedUnfocusingAnimations) { animation(animationContext); }
    }, [coordinator](bool res) {
        for (const auto& completion: coordinator._coordinatedAnimationCompletions) { completion(); }
    });
}

UIFocusHeading UIFocusSystem::makeFocusHeadingFromEvent(const std::shared_ptr<UIPressesEvent>& event) {
    if (event == nullptr) return UIFocusHeading::none;

    for (const auto& press: event->allPresses()) {
        if (press->phase() != UIPressPhase::began) continue;

        auto type = press->type();

        if (type == UIPressType::rightArrow) {
            return UIFocusHeading::right;
        }

        if (type == UIPressType::upArrow) {
            return UIFocusHeading::up;
        }

        if (type == UIPressType::leftArrow) {
            return UIFocusHeading::left;
        }

        if (type == UIPressType::downArrow) {
            return UIFocusHeading::down;
        }
    }

    return UIFocusHeading::none;
}


}
