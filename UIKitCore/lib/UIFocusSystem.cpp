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

#include <functional>
#include <unordered_set>

namespace NXKit {

UIFocusSystem::UIFocusSystem() = default;

bool UIFocusSystem::requestFocusUpdate(
    const std::shared_ptr<UIFocusEnvironment>& environment
) {
    if (!environment) {
        return false;
    }

    std::unordered_set<const UIFocusEnvironment*> visited;
    std::function<std::shared_ptr<UIFocusItem>(
        const std::shared_ptr<UIFocusEnvironment>&
    )> resolveItem;
    resolveItem = [&visited, &resolveItem](
        const std::shared_ptr<UIFocusEnvironment>& candidate
    ) -> std::shared_ptr<UIFocusItem> {
        if (!candidate || !visited.insert(candidate.get()).second) {
            return nullptr;
        }
        if (const auto view = std::dynamic_pointer_cast<UIView>(candidate)) {
            return view->searchForFocus();
        }
        if (const auto item = std::dynamic_pointer_cast<UIFocusItem>(candidate);
            item && item->canBecomeFocused()) {
            return item;
        }
        for (const auto& preferred : candidate->preferredFocusEnvironments()) {
            if (const auto item = resolveItem(preferred)) {
                return item;
            }
        }
        if (const auto viewController =
                std::dynamic_pointer_cast<UIViewController>(candidate)) {
            return viewController->view()->searchForFocus();
        }
        return nullptr;
    };

    const auto item = resolveItem(environment);

    if (!item || !item->canBecomeFocused()) {
        return false;
    }

    const auto rootWindow = _rootWindow.lock();
    const auto itemView = std::dynamic_pointer_cast<UIView>(item);
    if (!rootWindow || !itemView || itemView->window() != rootWindow) {
        return false;
    }

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _isActive
        ? _focusedItem
        : std::weak_ptr<UIFocusItem> {};
    context._nextFocusedItem = item;
    context._focusHeading = UIFocusHeading::none;

    if (const auto previous = context.previouslyFocusedItem().lock();
        previous && !previous->shouldUpdateFocusIn(context)) {
        return false;
    }
    if (!item->shouldUpdateFocusIn(context)) {
        return false;
    }

    _isActive = true;
    applyFocusToItem(item, context);
    return true;
}

void UIFocusSystem::setActive(bool active) {
    if (_isActive == active) {
        return;
    }

    const auto retainedItem = _focusedItem.lock();
    UIFocusUpdateContext context;
    context._previouslyFocusedItem = active
        ? std::weak_ptr<UIFocusItem> {}
        : retainedItem;
    context._nextFocusedItem = active
        ? retainedItem
        : std::weak_ptr<UIFocusItem> {};
    context._focusHeading = UIFocusHeading::none;

    _isActive = active;
    applyFocusToItem(retainedItem, context);
}

void UIFocusSystem::sendEvent(const std::shared_ptr<UIEvent>& event) {
    auto pevent = std::dynamic_pointer_cast<UIPressesEvent>(event);
    if (pevent == nullptr) return setActive(false);

    if (!_isActive) {
        setActive(true);
        return;
    }

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

    if (nextItem.expired()) {
        nextItem = _focusedItem;
    }

    context._nextFocusedItem = nextItem;
    applyFocusToItem(nextItem.lock(), context);
}

void UIFocusSystem::updateFocus() {
    auto item = _rootWindow.lock()->searchForFocus();

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _focusedItem;
    context._nextFocusedItem = _isActive ? item : std::weak_ptr<UIFocusItem>();
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

    if (context.previouslyFocusedItem().lock() == context.nextFocusedItem().lock()) {
        if (!context.previouslyFocusedItem().expired()) { context.previouslyFocusedItem().lock()->didUpdateFocusIn(context, &coordinator); }
    } else {
        if (!context.previouslyFocusedItem().expired()) { context.previouslyFocusedItem().lock()->didUpdateFocusIn(context, &coordinator); }
        if (!context.nextFocusedItem().expired()) { context.nextFocusedItem().lock()->didUpdateFocusIn(context, &coordinator); }
    }

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
