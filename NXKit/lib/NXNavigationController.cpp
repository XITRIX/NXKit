#include <NXNavigationController.h>

#include <UIApplication.h>
#include <UIBlurView.h>
#include <UIImageView.h>
#include <UILabel.h>
#include <UIWindow.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

using namespace NXKit;
using namespace NXKit::yoga::literals;

namespace {

constexpr NXFloat navigationBarContentHeight = 88;
constexpr NXFloat toolbarContentHeight = 73;
constexpr double navigationTransitionDuration = 0.3;

bool containsController(
    const std::vector<std::shared_ptr<UIViewController>>& controllers,
    const std::shared_ptr<UIViewController>& controller
) {
    return std::find(controllers.begin(), controllers.end(), controller) != controllers.end();
}

}

NXNavigationController::NXNavigationController(
    const std::shared_ptr<UIViewController>& rootViewController
) {
    if (!rootViewController) {
        throw std::invalid_argument(
            "NXNavigationController requires a non-null root view controller"
        );
    }
    if (!rootViewController->parent().expired()) {
        throw std::invalid_argument(
            "The root view controller already belongs to another container"
        );
    }
    _viewControllers.push_back(rootViewController);
}

void NXNavigationController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::systemBackground);
    rootView->setAutolayoutEnabled(true);

    auto navigationBar = new_shared<UIBlurView>();
    navigationBar->setAutolayoutEnabled(true);
    navigationBar->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setLeft(0_pt);
        layout->setTop(0_pt);
        layout->setWidth(100_percent);
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setAlignItems(YGAlignStretch);
        layout->setHeight(YGValue { navigationBarContentHeight, YGUnitPoint });
        layout->setPaddingHorizontal(30_pt);
    });
    _navigationBar = navigationBar;

    auto navigationBarContent = new_shared<UIView>();
    navigationBarContent->setAutolayoutEnabled(true);
    navigationBarContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setJustifyContent(YGJustifySpaceBetween);
        layout->setFlexGrow(1);
        layout->setPaddingHorizontal(30_pt);
    });

    _navigationLeadingContainer = new_shared<UIView>();
    _navigationLeadingContainer->setAutolayoutEnabled(true);
    _navigationLeadingContainer->setUserInteractionEnabled(true);
    _navigationLeadingContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(24);
        layout->setFlexGrow(1);
        layout->setFlexShrink(1);
    });

    _titleContainer = new_shared<UIView>();
    _titleContainer->setAutolayoutEnabled(true);
    _titleContainer->setUserInteractionEnabled(false);
    _titleContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(12);
    });

    _iconView = new_shared<UIImageView>();
    _iconView->setAutolayoutEnabled(true);
    _iconView->setUserInteractionEnabled(false);
    _iconView->setContentMode(UIViewContentMode::scaleAspectFill);
    _iconView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setSize({58, 58});
    });

    _titleLabel = new_shared<UILabel>();
    _titleLabel->setAutolayoutEnabled(true);
    _titleLabel->setUserInteractionEnabled(false);
    _titleLabel->setFontSize(28);
    _titleLabel->setTextAlignment(NSTextAlignment::left);
    _titleLabel->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexShrink(1);
    });

    _titleContainer->addSubview(_iconView);
    _titleContainer->addSubview(_titleLabel);
    _navigationLeadingContainer->addSubview(_titleContainer);

    _navigationTrailingContainer = new_shared<UIView>();
    _navigationTrailingContainer->setAutolayoutEnabled(true);
    _navigationTrailingContainer->setUserInteractionEnabled(true);
    _navigationTrailingContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setJustifyContent(YGJustifyFlexEnd);
        layout->setAllGap(24);
    });

    navigationBarContent->addSubview(_navigationLeadingContainer);
    navigationBarContent->addSubview(_navigationTrailingContainer);
    navigationBar->addSubview(navigationBarContent);

    auto separator = new_shared<UIView>();
    separator->setAutolayoutEnabled(true);
    separator->setBackgroundColor(UIColor::label);
    separator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setHeight(1_pt);
    });
    navigationBar->addSubview(separator);

    _contentView = new_shared<UIView>();
    _contentView->setAutolayoutEnabled(true);
    _contentView->setClipsToBounds(true);
    _contentView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setLeft(0_pt);
        layout->setTop(0_pt);
        layout->setWidth(100_percent);
        layout->setHeight(100_percent);
    });

    rootView->addSubview(_contentView);

    auto toolbar = new_shared<UIBlurView>();
    toolbar->setAutolayoutEnabled(true);
    toolbar->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setLeft(0_pt);
        layout->setBottom(0_pt);
        layout->setWidth(100_percent);
        layout->setHeight(YGValue { toolbarContentHeight, YGUnitPoint });
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setAlignItems(YGAlignStretch);
        layout->setPaddingHorizontal(30_pt);
    });
    _toolbar = toolbar;

    auto toolbarSeparator = new_shared<UIView>();
    toolbarSeparator->setAutolayoutEnabled(true);
    toolbarSeparator->setBackgroundColor(UIColor::label);
    toolbarSeparator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setHeight(1_pt);
    });
    toolbar->addSubview(toolbarSeparator);

    auto toolbarContent = new_shared<UIView>();
    toolbarContent->setAutolayoutEnabled(true);
    toolbarContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setJustifyContent(YGJustifySpaceBetween);
        layout->setFlexGrow(1);
        layout->setPaddingHorizontal(30_pt);
    });

    _toolbarLeadingContainer = new_shared<UIView>();
    _toolbarLeadingContainer->setAutolayoutEnabled(true);
    _toolbarLeadingContainer->setUserInteractionEnabled(true);
    _toolbarLeadingContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(24);
    });

    _toolbarTrailingContainer = new_shared<UIView>();
    _toolbarTrailingContainer->setAutolayoutEnabled(true);
    _toolbarTrailingContainer->setUserInteractionEnabled(true);
    _toolbarTrailingContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setJustifyContent(YGJustifyFlexEnd);
        layout->setAllGap(24);
    });

    toolbarContent->addSubview(_toolbarLeadingContainer);
    toolbarContent->addSubview(_toolbarTrailingContainer);
    toolbar->addSubview(toolbarContent);

    _defaultStatusWidget = new_shared<NXNavigationStatusView>();
    _defaultActionsWidget = new_shared<NXNavigationActionsView>();
    const auto weakSelf = weak_from_base<NXNavigationController>();
    _defaultActionsWidget->setActionProvider([weakSelf]() {
        if (const auto self = weakSelf.lock()) {
            return self->resolvedActions();
        }
        return std::vector<NXResponderAction> {};
    });

    rootView->addSubview(_navigationBar);
    rootView->addSubview(_toolbar);
    setView(rootView);
}

void NXNavigationController::viewDidLoad() {
    UIViewController::viewDidLoad();
    updateBarSafeAreas();
    _isTransitioning = true;
    try {
        attachConfiguredChildren();
        updateVisibleViewController(Operation::none, false);
    } catch (...) {
        _isTransitioning = false;
        throw;
    }
}

void NXNavigationController::viewWillAppear(bool animated) {
    updateChrome();
    const auto visible = _visibleViewController
        ? _visibleViewController
        : topViewController();
    if (!visible) {
        return;
    }

    if (const auto strongDelegate = delegate.lock()) {
        strongDelegate->navigationControllerWillShow(
            shared_from_base<NXNavigationController>(),
            visible,
            animated
        );
    }
    visible->viewWillAppear(animated);
}

void NXNavigationController::viewDidAppear(bool animated) {
    const auto visible = _visibleViewController
        ? _visibleViewController
        : topViewController();
    if (!visible) {
        return;
    }

    visible->viewDidAppear(animated);
    if (const auto strongDelegate = delegate.lock()) {
        strongDelegate->navigationControllerDidShow(
            shared_from_base<NXNavigationController>(),
            visible,
            animated
        );
    }
}

void NXNavigationController::viewWillDisappear(bool animated) {
    const auto visible = _visibleViewController
        ? _visibleViewController
        : topViewController();
    if (visible) {
        visible->viewWillDisappear(animated);
    }
}

void NXNavigationController::viewDidDisappear(bool animated) {
    const auto visible = _visibleViewController
        ? _visibleViewController
        : topViewController();
    if (visible) {
        visible->viewDidDisappear(animated);
    }
}

void NXNavigationController::viewSafeAreaInsetsDidChange() {
    updateBarSafeAreas();
}

void NXNavigationController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    updateBarSafeAreas();
}

void NXNavigationController::validateViewControllers(
    const std::vector<std::shared_ptr<UIViewController>>& viewControllers
) {
    if (viewControllers.empty()) {
        throw std::invalid_argument(
            "NXNavigationController requires at least one view controller"
        );
    }

    std::unordered_set<const UIViewController*> uniqueControllers;
    for (const auto& controller : viewControllers) {
        if (!controller) {
            throw std::invalid_argument(
                "NXNavigationController does not accept null view controllers"
            );
        }
        if (controller.get() == this) {
            throw std::invalid_argument("NXNavigationController cannot contain itself");
        }
        for (auto ancestor = parent().lock(); ancestor; ancestor = ancestor->parent().lock()) {
            if (controller == ancestor) {
                throw std::invalid_argument(
                    "Adding this controller would create a controller hierarchy cycle"
                );
            }
        }
        if (!uniqueControllers.insert(controller.get()).second) {
            throw std::invalid_argument(
                "NXNavigationController does not accept duplicate view controllers"
            );
        }
        if (const auto parent = controller->parent().lock(); parent && parent.get() != this) {
            throw std::invalid_argument(
                "An NXNavigationController child already belongs to another container"
            );
        }
    }
}

void NXNavigationController::setViewControllers(
    std::vector<std::shared_ptr<UIViewController>> viewControllers,
    bool animated
) {
    validateViewControllers(viewControllers);
    if (requestedViewControllers() == viewControllers) {
        return;
    }
    if (_isTransitioning) {
        _pendingStackUpdates.push_back({ std::move(viewControllers), animated });
        return;
    }

    beginStackUpdate(std::move(viewControllers), animated);
}

const std::vector<std::shared_ptr<UIViewController>>&
NXNavigationController::requestedViewControllers() const {
    return _pendingStackUpdates.empty()
        ? _viewControllers
        : _pendingStackUpdates.back().viewControllers;
}

void NXNavigationController::beginStackUpdate(
    std::vector<std::shared_ptr<UIViewController>> viewControllers,
    bool animated
) {
    if (_viewControllers == viewControllers) {
        completeStackUpdate();
        return;
    }

    const auto previousViewControllers = _viewControllers;
    const auto previousTop = topViewController();
    const auto nextTop = viewControllers.back();

    std::vector<std::shared_ptr<UIViewController>> removedControllers;
    for (const auto& controller : previousViewControllers) {
        if (!containsController(viewControllers, controller)) {
            removedControllers.push_back(controller);
        }
    }

    Operation operation = Operation::none;
    if (previousTop != nextTop) {
        operation = containsController(previousViewControllers, nextTop)
            ? Operation::pop
            : Operation::push;
    }

    _viewControllers = std::move(viewControllers);
    if (!viewIsLoaded()) {
        return;
    }

    _isTransitioning = true;
    try {
        attachConfiguredChildren();
        updateVisibleViewController(operation, animated, std::move(removedControllers));
    } catch (...) {
        _isTransitioning = false;
        throw;
    }
}

void NXNavigationController::completeStackUpdate() {
    if (_pendingStackUpdates.empty()) {
        _isTransitioning = false;
        if (_defaultActionsWidget) {
            _defaultActionsWidget->refresh();
        }
        return;
    }

    auto nextUpdate = std::move(_pendingStackUpdates.front());
    _pendingStackUpdates.pop_front();
    beginStackUpdate(std::move(nextUpdate.viewControllers), nextUpdate.animated);
}

void NXNavigationController::pushViewController(
    const std::shared_ptr<UIViewController>& viewController,
    bool animated
) {
    if (!viewController) {
        throw std::invalid_argument(
            "NXNavigationController::pushViewController requires a non-null view controller"
        );
    }

    auto updatedViewControllers = requestedViewControllers();
    updatedViewControllers.push_back(viewController);
    setViewControllers(std::move(updatedViewControllers), animated);
}

std::shared_ptr<UIViewController> NXNavigationController::popViewController(bool animated) {
    const auto& requestedControllers = requestedViewControllers();
    if (requestedControllers.size() <= 1) {
        return nullptr;
    }

    auto poppedViewController = requestedControllers.back();
    auto updatedViewControllers = requestedControllers;
    updatedViewControllers.pop_back();
    setViewControllers(std::move(updatedViewControllers), animated);
    return poppedViewController;
}

std::vector<std::shared_ptr<UIViewController>> NXNavigationController::popToViewController(
    const std::shared_ptr<UIViewController>& viewController,
    bool animated
) {
    if (!viewController) {
        return {};
    }

    const auto& requestedControllers = requestedViewControllers();
    const auto target = std::find(
        requestedControllers.begin(),
        requestedControllers.end(),
        viewController
    );
    if (target == requestedControllers.end()
        || std::next(target) == requestedControllers.end()) {
        return {};
    }

    std::vector<std::shared_ptr<UIViewController>> poppedViewControllers(
        std::next(target),
        requestedControllers.end()
    );
    std::vector<std::shared_ptr<UIViewController>> updatedViewControllers(
        requestedControllers.begin(),
        std::next(target)
    );
    setViewControllers(std::move(updatedViewControllers), animated);
    return poppedViewControllers;
}

std::vector<std::shared_ptr<UIViewController>>
NXNavigationController::popToRootViewController(bool animated) {
    const auto& requestedControllers = requestedViewControllers();
    if (requestedControllers.empty()) {
        return {};
    }
    return popToViewController(requestedControllers.front(), animated);
}

std::shared_ptr<UIViewController> NXNavigationController::topViewController() const {
    return _viewControllers.empty() ? nullptr : _viewControllers.back();
}

std::shared_ptr<UIViewController> NXNavigationController::visibleViewController() const {
    if (const auto presented =
            const_cast<NXNavigationController*>(this)->presentedViewController()) {
        return presented;
    }
    return _visibleViewController ? _visibleViewController : topViewController();
}

std::shared_ptr<NXNavigationItem> NXNavigationController::navigationItemFor(
    const std::shared_ptr<UIViewController>& viewController
) {
    if (!viewController) {
        throw std::invalid_argument(
            "NXNavigationController::navigationItemFor requires a non-null view controller"
        );
    }

    std::erase_if(_navigationItems, [](const NavigationItemAssociation& association) {
        return association.viewController.expired();
    });
    for (const auto& association : _navigationItems) {
        if (association.viewController.lock() == viewController) {
            return association.item;
        }
    }

    auto item = new_shared<NXNavigationItem>();
    const auto weakSelf = weak_from_base<NXNavigationController>();
    const std::weak_ptr<UIViewController> weakViewController = viewController;
    item->_changeHandler = [weakSelf, weakViewController]() {
        const auto self = weakSelf.lock();
        const auto controller = weakViewController.lock();
        if (self && controller && self->viewIsLoaded()
            && self->topViewController() == controller) {
            self->updateChrome();
        }
    };
    _navigationItems.push_back({ viewController, item });
    return item;
}

std::shared_ptr<NXNavigationItem> NXNavigationController::topNavigationItem() {
    const auto top = topViewController();
    return top ? navigationItemFor(top) : nullptr;
}

void NXNavigationController::show(
    const std::shared_ptr<UIViewController>& viewController,
    const std::shared_ptr<UIResponder>& sender
) {
    (void)sender;
    pushViewController(viewController, true);
}

void NXNavigationController::attachConfiguredChildren() {
    const auto self = shared_from_this();
    for (const auto& controller : _viewControllers) {
        if (const auto parent = controller->parent().lock()) {
            if (parent.get() != this) {
                throw std::logic_error(
                    "An NXNavigationController child belongs to another container"
                );
            }
            continue;
        }

        addChild(controller);
        controller->didMoveToParent(self);
    }
}

void NXNavigationController::detachRemovedControllers(
    const std::vector<std::shared_ptr<UIViewController>>& controllers
) {
    for (const auto& controller : controllers) {
        if (containsController(_viewControllers, controller)) {
            continue;
        }
        if (const auto parent = controller->parent().lock(); parent.get() == this) {
            controller->willMoveToParent(nullptr);
            if (controller->viewIsLoaded()) {
                controller->view()->removeFromSuperview();
            }
            controller->removeFromParent();
        }
    }
}

std::vector<NXResponderAction> NXNavigationController::resolvedActions() {
    std::shared_ptr<UIResponder> firstResponder;
    if (viewIsLoaded()) {
        if (const auto window = view()->window()) {
            if (const auto focusedView = std::dynamic_pointer_cast<UIView>(
                    window->focusSystem()->focusedItem().lock()
                ); focusedView && focusedView->window() == window) {
                firstResponder = focusedView;
            }
        }
    }
    if (!firstResponder) {
        if (const auto top = topViewController()) {
            firstResponder = top->view();
        }
    }
    return NXCollectResponderActions(firstResponder);
}

void NXNavigationController::mountWidget(
    const std::shared_ptr<UIView>& widget,
    NXNavigationWidgetPlacement placement
) {
    if (!widget || placement == NXNavigationWidgetPlacement::hidden) {
        return;
    }
    if (std::find(_mountedItemWidgets.begin(), _mountedItemWidgets.end(), widget)
        != _mountedItemWidgets.end()) {
        return;
    }

    std::shared_ptr<UIView> container;
    switch (placement) {
        case NXNavigationWidgetPlacement::navigationLeading:
            container = _navigationLeadingContainer;
            break;
        case NXNavigationWidgetPlacement::navigationTrailing:
            container = _navigationTrailingContainer;
            break;
        case NXNavigationWidgetPlacement::toolbarLeading:
            container = _toolbarLeadingContainer;
            break;
        case NXNavigationWidgetPlacement::toolbarTrailing:
            container = _toolbarTrailingContainer;
            break;
        case NXNavigationWidgetPlacement::hidden:
            return;
    }

    if (widget == _defaultStatusWidget) {
        _defaultStatusWidget->setCompact(
            placement == NXNavigationWidgetPlacement::toolbarLeading
                || placement == NXNavigationWidgetPlacement::toolbarTrailing
        );
        _defaultStatusWidget->refresh();
    }
    widget->setAutolayoutEnabled(true);
    container->addSubview(widget);
    _mountedItemWidgets.push_back(widget);
}

void NXNavigationController::updateChrome() {
    if (!_titleLabel || !_iconView || !_defaultStatusWidget || !_defaultActionsWidget) {
        return;
    }

    const auto top = topViewController();
    const auto item = topNavigationItem();

    const auto self = shared_from_base<NXNavigationController>();
    const bool canNavigateBack = _viewControllers.size() > 1;
    NXResponderAction backAction {
        .button = NXActionButton::b,
        .isEnabled = true,
        .action = UIAction(canNavigateBack ? "Back" : "Exit", [
            weakSelf = weak_from_base<NXNavigationController>()
        ]() {
            if (const auto navigationController = weakSelf.lock()) {
                if (navigationController->popViewController(true)) {
                    return;
                }
                if (UIApplication::shared) {
                    UIApplication::shared->handleSDLQuit();
                }
            }
        }),
    };
    backAction.registerOn(self);
    const auto title = item && item->titleOverride()
        ? *item->titleOverride()
        : (top ? top->title() : std::string {});
    _titleLabel->setText(title);

    const auto icon = item ? item->icon() : nullptr;
    _iconView->setImage(icon);
    _iconView->setHidden(!icon);

    for (const auto& widget : _mountedItemWidgets) {
        if (widget) {
            widget->removeFromSuperview();
        }
    }
    _mountedItemWidgets.clear();

    _defaultActionsWidget->refresh();
    const auto statusWidget = item && item->statusWidget()
        ? item->statusWidget()
        : std::static_pointer_cast<UIView>(_defaultStatusWidget);
    const auto actionsWidget = item && item->actionsWidget()
        ? item->actionsWidget()
        : std::static_pointer_cast<UIView>(_defaultActionsWidget);
    mountWidget(
        statusWidget,
        item ? item->statusWidgetPlacement() : NXNavigationWidgetPlacement::toolbarLeading
    );
    mountWidget(
        actionsWidget,
        item ? item->actionsWidgetPlacement() : NXNavigationWidgetPlacement::toolbarTrailing
    );
}

void NXNavigationController::updateBarSafeAreas() {
    if (!_navigationBar || !_toolbar || !viewIsLoaded()) {
        return;
    }

    const auto superview = view()->superview().lock();
    const auto systemInsets = superview ? superview->safeAreaInsets() : UIEdgeInsets::zero;
    const auto topInset = std::max<NXFloat>(0, systemInsets.top);
    const auto bottomInset = std::max<NXFloat>(0, systemInsets.bottom);
    _navigationBar->configureLayout([topInset](const std::shared_ptr<YGLayout>& layout) {
        layout->setHeight(YGValue {
            static_cast<float>(navigationBarContentHeight + topInset),
            YGUnitPoint
        });
        layout->setPaddingTop(YGValue { static_cast<float>(topInset), YGUnitPoint });
    });
    _toolbar->configureLayout([bottomInset](const std::shared_ptr<YGLayout>& layout) {
        layout->setHeight(YGValue {
            static_cast<float>(toolbarContentHeight + bottomInset),
            YGUnitPoint
        });
        layout->setPaddingBottom(YGValue { static_cast<float>(bottomInset), YGUnitPoint });
    });

    const UIEdgeInsets chromeInsets {
        navigationBarContentHeight,
        0,
        toolbarContentHeight,
        0,
    };
    if (additionalSafeAreaInsets() != chromeInsets) {
        setAdditionalSafeAreaInsets(chromeInsets);
    }
}

void NXNavigationController::updateVisibleViewController(
    Operation operation,
    bool animated,
    std::vector<std::shared_ptr<UIViewController>> removedControllers
) {
    const auto nextViewController = topViewController();
    const auto previousViewController = _visibleViewController;
    updateChrome();

    if (previousViewController == nextViewController) {
        detachRemovedControllers(removedControllers);
        completeStackUpdate();
        return;
    }

    const bool isVisible = view()->window() != nullptr;
    const auto nextView = nextViewController->view();
    nextView->setAutolayoutEnabled(true);
    nextView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setLeft(0_pt);
        layout->setTop(0_pt);
        layout->setWidth(100_percent);
        layout->setHeight(100_percent);
    });

    const auto previousView = previousViewController && previousViewController->viewIsLoaded()
        ? previousViewController->view()
        : nullptr;
    const bool previousViewIsMounted = previousView
        && previousView->superview().lock() == _contentView;

    if (operation == Operation::pop && previousViewIsMounted) {
        _contentView->insertSubviewBelow(nextView, previousView);
    } else {
        _contentView->addSubview(nextView);
    }
    _visibleViewController = nextViewController;

    if (isVisible) {
        if (const auto strongDelegate = delegate.lock()) {
            strongDelegate->navigationControllerWillShow(
                shared_from_base<NXNavigationController>(),
                nextViewController,
                animated
            );
        }
        if (previousViewController) {
            previousViewController->viewWillDisappear(animated);
        }
        nextViewController->viewWillAppear(animated);
    }

    if (isVisible) {
        if (const auto window = view()->window()) {
            window->focusSystem()->requestFocusUpdate(nextViewController);
        }
    }
    _defaultActionsWidget->refresh();

    NXFloat width = _contentView->bounds().width();
    if (width <= 0) {
        width = view()->bounds().width();
    }
    const bool shouldAnimate = animated && isVisible && previousViewIsMounted && width > 0;

    const auto weakSelf = weak_from_base<NXNavigationController>();
    auto finish = [
        weakSelf,
        previousViewController,
        nextViewController,
        previousView,
        nextView,
        removedControllers = std::move(removedControllers),
        isVisible,
        animated
    ]() {
        const auto self = weakSelf.lock();
        if (!self) {
            return;
        }

        if (previousView && previousView != nextView) {
            previousView->removeFromSuperview();
            previousView->setTransform(NXAffineTransform::identity);
        }
        nextView->setTransform(NXAffineTransform::identity);
        self->detachRemovedControllers(removedControllers);

        if (isVisible) {
            if (previousViewController) {
                previousViewController->viewDidDisappear(animated);
            }
            nextViewController->viewDidAppear(animated);
        }

        if (isVisible) {
            if (const auto strongDelegate = self->delegate.lock()) {
                strongDelegate->navigationControllerDidShow(
                    self,
                    nextViewController,
                    animated
                );
            }
            if (const auto window = self->view()->window()) {
                if (!window->focusSystem()->requestFocusUpdate(nextViewController)) {
                    window->updateFocus();
                }
            }
        }
        self->completeStackUpdate();
    };

    if (!shouldAnimate) {
        finish();
        return;
    }

    if (operation == Operation::pop) {
        nextView->setTransform(NXAffineTransform::translationBy(-width * 0.25, 0));
    } else {
        nextView->setTransform(NXAffineTransform::translationBy(width, 0));
    }

    UIView::animate(
        navigationTransitionDuration,
        0,
        UIViewAnimationOptions(
            curveEaseOut | preferredFramesPerSecond120
        ),
        [operation, width, previousView, nextView]() {
            nextView->setTransform(NXAffineTransform::identity);
            if (!previousView) {
                return;
            }
            previousView->setTransform(
                operation == Operation::pop
                    ? NXAffineTransform::translationBy(width, 0)
                    : NXAffineTransform::translationBy(-width * 0.25, 0)
            );
        },
        [finish = std::move(finish)](bool) {
            finish();
        }
    );
}
