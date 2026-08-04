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

    return requestExactFocusUpdate(resolveItem(environment));
}

bool UIFocusSystem::requestExactFocusUpdate(
    const std::shared_ptr<UIFocusItem>& item
) {
    if (!item || !item->canBecomeFocused()) {
        return false;
    }

    const auto rootWindow = _rootWindow.lock();
    const auto itemView = std::dynamic_pointer_cast<UIView>(item);
    const auto focusRoot = rootWindow ? rootWindow->focusRootView() : nullptr;
    if (!rootWindow || !focusRoot || !itemView
        || itemView->window() != rootWindow
        || !itemView->isDescendantOf(focusRoot)) {
        return false;
    }

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _isActive
        ? _focusedItem
        : std::weak_ptr<UIFocusItem> {};
    context._nextFocusedItem = item;
    context._focusHeading = UIFocusHeading::none;

    if (!shouldAllowFocusUpdate(context)) {
        return false;
    }

    if (!_isActive) {
        // Touch input hides the focus appearance, but actions may still update
        // the item that should regain focus when controller input resumes.
        // Retain that destination without turning a touch-driven action into a
        // visible focus update.
        _focusedItem = item;
        return true;
    }

    applyFocusToItem(item, context);
    return true;
}

void UIFocusSystem::setActive(bool active) {
    if (_isActive == active) {
        return;
    }

    auto retainedItem = _focusedItem.lock();
    if (active) {
        const auto rootWindow = _rootWindow.lock();
        const auto focusRoot = rootWindow ? rootWindow->focusRootView() : nullptr;
        const auto retainedView =
            std::dynamic_pointer_cast<UIView>(retainedItem);
        if (!focusRoot || !retainedView
            || !retainedView->isDescendantOf(focusRoot)) {
            retainedItem = focusRoot ? focusRoot->searchForFocus() : nullptr;
        }
    }

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

    updatePressedFocusHeadings(pevent);

    if (!_isActive) {
        setActive(true);
        return;
    }

    const auto rootWindow = _rootWindow.lock();
    const auto focusRoot = rootWindow ? rootWindow->focusRootView() : nullptr;
    if (!rootWindow || !focusRoot) {
        return;
    }

    if (const auto current = std::dynamic_pointer_cast<UIView>(
            focusedItem().lock()
        ); current && !current->isDescendantOf(focusRoot)) {
        updateFocus();
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
            } else if (press->phase() == UIPressPhase::cancelled) {
                focusedView->pressesCancelled(pevent->allPresses(), pevent);
                _selectedFocusedItem.reset();
            }
        }
        return;
    }

    UIFocusUpdateContext context;
    context._previouslyFocusedItem = _focusedItem;
    context._focusHeading = makeFocusHeadingFromEvent(pevent);

    if (context._focusHeading == UIFocusHeading::none) return;

    auto current = std::dynamic_pointer_cast<UIView>(focusedItem().lock());
    const bool currentIsInFocusRoot = current
        && current->isDescendantOf(focusRoot);
    std::shared_ptr<UIFocusItem> nextItem;
    if (!currentIsInFocusRoot) {
        nextItem = focusRoot->searchForFocus();
    } else {
        // A modal presentation is its own focus branch. Walk no farther than
        // the top presented controller's root view, even though that view is a
        // sibling of the presenting hierarchy in UIWindow.
        while (current && current != focusRoot) {
            const auto parent = current->superview().lock();
            if (!parent) {
                break;
            }

            const auto candidate = parent->getNextFocusItem(
                current,
                context._focusHeading
            );
            if (candidate) {
                const auto candidateView =
                    std::dynamic_pointer_cast<UIView>(candidate);
                if (candidateView && candidateView->isDescendantOf(focusRoot)) {
                    nextItem = candidate;
                    break;
                }
            }
            current = parent;
        }
    }

    if (!nextItem && currentIsInFocusRoot) {
        nextItem = _focusedItem.lock();
    }

    context._nextFocusedItem = nextItem;
    if (!shouldAllowFocusUpdate(context)) {
        nextItem = currentIsInFocusRoot ? _focusedItem.lock() : nullptr;
        context._nextFocusedItem = nextItem;
    }
    applyFocusToItem(nextItem, context);
}

void UIFocusSystem::updatePressedFocusHeadings(
    const std::shared_ptr<UIPressesEvent>& event
) {
    if (!event) {
        return;
    }

    std::erase_if(_pressedFocusHeadings, [](const auto& entry) {
        return entry.first.expired();
    });
    for (const auto& press : event->allPresses()) {
        if (!press || press->isRepeat()) {
            continue;
        }

        UIFocusHeading heading = UIFocusHeading::none;
        switch (press->type()) {
            case UIPressType::upArrow:
                heading = UIFocusHeading::up;
                break;
            case UIPressType::downArrow:
                heading = UIFocusHeading::down;
                break;
            case UIPressType::leftArrow:
                heading = UIFocusHeading::left;
                break;
            case UIPressType::rightArrow:
                heading = UIFocusHeading::right;
                break;
            default:
                break;
        }
        if (heading == UIFocusHeading::none) {
            continue;
        }

        std::erase_if(_pressedFocusHeadings, [&press](const auto& entry) {
            return entry.first.lock() == press;
        });
        if (press->phase() == UIPressPhase::began) {
            _pressedFocusHeadings.emplace_back(press, heading);
        }
    }
}

bool UIFocusSystem::isFocusHeadingPressed(UIFocusHeading heading) {
    std::erase_if(_pressedFocusHeadings, [](const auto& entry) {
        return entry.first.expired();
    });
    return _isActive && std::any_of(
        _pressedFocusHeadings.begin(),
        _pressedFocusHeadings.end(),
        [heading](const auto& entry) {
            return entry.second == heading;
        }
    );
}

bool UIFocusSystem::shouldAllowFocusUpdate(UIFocusUpdateContext context) {
    std::unordered_set<const UIFocusEnvironment*> visited;
    const auto validate = [&visited, &context](
        const std::shared_ptr<UIFocusEnvironment>& environment
    ) {
        return !environment || !visited.insert(environment.get()).second
            || environment->shouldUpdateFocusIn(context);
    };
    const auto validateParentPath = [&validate](
        const std::shared_ptr<UIFocusItem>& item
    ) {
        std::unordered_set<const UIFocusEnvironment*> pathVisited;
        std::shared_ptr<UIFocusEnvironment> environment = item
            ? item->parentFocusEnvironment()
            : nullptr;
        while (environment) {
            if (!pathVisited.insert(environment.get()).second) {
                break;
            }
            if (!validate(environment)) {
                return false;
            }
            environment = environment->parentFocusEnvironment();
        }
        return true;
    };

    const auto previous = context.previouslyFocusedItem().lock();
    const auto next = context.nextFocusedItem().lock();
    return validate(previous)
        && validate(next)
        && validateParentPath(previous)
        && validateParentPath(next);
}

void UIFocusSystem::updateFocus() {
    const auto rootWindow = _rootWindow.lock();
    const auto focusRoot = rootWindow ? rootWindow->focusRootView() : nullptr;
    const auto item = focusRoot ? focusRoot->searchForFocus() : nullptr;

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
    UIView::animate(animationContext.duration(), 0,
                    UIViewAnimationOptions(
                        curveEaseOut
                            | allowUserInteraction
                            | beginFromCurrentState
                    ), [context, coordinator, animationContext]() {
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
