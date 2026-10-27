#include <NXTabBarController.h>
#include <NXResponderAction.h>
#include <NXNavigationController.h>
#include <DispatchQueue.h>

#include <NXSeparatorView.h>
#include <UIWindow.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

using namespace NXKit;
using namespace NXKit::yoga::literals;

NXTabBarButton::NXTabBarButton() {
    _rectView = new_shared<UIView>();
    _titleLabel = new_shared<UILabel>();

    _rectView->setAlpha(0);
    _rectView->setBackgroundColor(UIColor::tint);

    _titleLabel->setHidden(true);
    _titleLabel->setFontSize(22);

    addSubview(_rectView);
    addSubview(_titleLabel);

    _rectView->setAutolayoutEnabled(true);
    _titleLabel->setAutolayoutEnabled(true);

    _rectView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(4_pt);
        layout->setMarginVertical(9_pt);
    });

    configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setJustifyContent(YGJustifyFlexStart);
        layout->setAlignItems(YGAlignStretch);
        layout->setAllGap(8);
        layout->setHeight(70_pt);
        layout->setPaddingHorizontal(8_pt);
    });

    layer()->setCornerRadius(4);
    layer()->setBorderWidth(4);

    std::for_each(subviews().begin(), subviews().end(), [](const auto& item) {
        item->setUserInteractionEnabled(false);
    });
}

void NXTabBarButton::didUpdateFocusIn(
    UIFocusUpdateContext context,
    UIFocusAnimationCoordinator* coordinator
) {
    NXControl::didUpdateFocusIn(context, coordinator);

    const auto next = context.nextFocusedItem().lock();
    const auto previous = context.previouslyFocusedItem().lock();
    if (_selectionFollowsFocus && !isSelected() && next.get() == this && previous != next) {
        if (_selectionAction) {
            _selectionAction();
        }
    }
}

void NXTabBarButton::setSelected(bool selected) {
    UIControl::setSelected(selected);
    if (selected) {
        _titleLabel->setTextColor(UIColor::tint);
        _rectView->setAlpha(1);
    } else {
        _titleLabel->setTextColor(UIColor::label);
        _rectView->setAlpha(0);
    }
}

void NXTabBarButton::setText(const std::string& text) {
    _titleLabel->setText(text);
    _titleLabel->setHidden(text.empty());
}

NXTabBar::NXTabBar() {
    setFocusTrackingMode(UIScrollViewFocusTrackingMode::centered);

    _container = new_shared<UIView>();
    addSubview(_container);

    _container->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setPaddingLeft(80_pt);
        layout->setPaddingRight(30_pt);
        layout->setPaddingTop(32_pt);
        layout->setPaddingBottom(47_pt);
    });

    setBackgroundColor(UIColor::quaternarySystemFill);
    setBounceVertically(true);
}

bool NXTabBar::contains(const IndexPath& indexPath) const {
    if (indexPath.section() < 0 || indexPath.item() < 0) {
        return false;
    }

    const auto section = static_cast<size_t>(indexPath.section());
    const auto item = static_cast<size_t>(indexPath.item());
    return section < _items.size() && item < _items[section].size();
}

std::optional<IndexPath> NXTabBar::firstIndexPath() const {
    for (size_t section = 0; section < _items.size(); ++section) {
        if (!_items[section].empty()) {
            return IndexPath(0, static_cast<int>(section));
        }
    }
    return std::nullopt;
}

void NXTabBar::setItems(Items items) {
    const auto previousSelection = _selectedIndexPath;
    _items = std::move(items);

    const auto oldSubviews = _container->subviews();
    for (const auto& subview : oldSubviews) {
        subview->removeFromSuperview();
    }

    _buttons.clear();
    _buttons.reserve(_items.size());

    const auto weakSelf = weak_from_base<NXTabBar>();
    bool hasPreviousItems = false;
    for (size_t section = 0; section < _items.size(); ++section) {
        std::vector<std::shared_ptr<NXTabBarButton>> sectionButtons;
        sectionButtons.reserve(_items[section].size());

        if (hasPreviousItems && !_items[section].empty()) {
            auto separator = new_shared<NXSeparatorView>();
            _container->addSubview(separator);
        }

        for (size_t item = 0; item < _items[section].size(); ++item) {
            auto button = new_shared<NXTabBarButton>();
            button->setAutolayoutEnabled(true);
            button->setSelectionFollowsFocus(_selectionFollowsFocus && _allowsFocusSelection);
            button->setText(_items[section][item]);
            button->_selectionAction = [weakSelf, section, item]() {
                if (const auto self = weakSelf.lock()) {
                    self->activateItemAt(
                        IndexPath(static_cast<int>(item), static_cast<int>(section))
                    );
                }
            };
            button->primaryAction = UIAction(
                "",
                [weakSelf, section, item]() {
                    if (const auto self = weakSelf.lock()) {
                        const IndexPath indexPath(
                            static_cast<int>(item),
                            static_cast<int>(section)
                        );
                        if (self->activateItemAt(indexPath)
                            && self->_primaryActionHandler) {
                            self->_primaryActionHandler(indexPath);
                        }
                    }
                }
            );

            sectionButtons.push_back(button);
            _container->addSubview(button);
        }
        hasPreviousItems = hasPreviousItems || !_items[section].empty();
        _buttons.push_back(std::move(sectionButtons));
    }

    _selectedIndexPath.reset();
    if (previousSelection && contains(*previousSelection)) {
        setSelectedIndexPath(*previousSelection);
    } else if (const auto first = firstIndexPath()) {
        setSelectedIndexPath(*first);
    }
}

std::vector<std::shared_ptr<UIFocusEnvironment>>
NXTabBar::preferredFocusEnvironments() {
    if (_selectedIndexPath && contains(*_selectedIndexPath)) {
        const auto section = static_cast<size_t>(_selectedIndexPath->section());
        const auto item = static_cast<size_t>(_selectedIndexPath->item());
        if (const auto& button = _buttons[section][item];
            button && button->canBecomeFocused()) {
            return { button };
        }
    }
    return UIScrollView::preferredFocusEnvironments();
}

bool NXTabBar::setSelectedIndexPath(const IndexPath& indexPath) {
    if (!contains(indexPath)) {
        return false;
    }

    if (_selectedIndexPath && contains(*_selectedIndexPath)) {
        _buttons[static_cast<size_t>(_selectedIndexPath->section())]
                [static_cast<size_t>(_selectedIndexPath->item())]
                    ->setSelected(false);
    }

    _selectedIndexPath = indexPath;
    _buttons[static_cast<size_t>(indexPath.section())]
            [static_cast<size_t>(indexPath.item())]
                ->setSelected(true);
    return true;
}

bool NXTabBar::activateItemAt(const IndexPath& indexPath) {
    if (!contains(indexPath)) {
        return false;
    }
    if (_selectionHandler && !_selectionHandler(indexPath)) {
        return false;
    }
    if (!setSelectedIndexPath(indexPath)) {
        return false;
    }
    if (selectionDidChange) {
        selectionDidChange(indexPath);
    }
    return true;
}

void NXTabBar::clearSelection() {
    if (_selectedIndexPath && contains(*_selectedIndexPath)) {
        _buttons[static_cast<size_t>(_selectedIndexPath->section())]
                [static_cast<size_t>(_selectedIndexPath->item())]
                    ->setSelected(false);
    }
    _selectedIndexPath.reset();
}

void NXTabBar::setSelectionFollowsFocus(bool selectionFollowsFocus) {
    if (_selectionFollowsFocus == selectionFollowsFocus) {
        return;
    }
    _selectionFollowsFocus = selectionFollowsFocus;
    for (const auto& section : _buttons) {
        for (const auto& button : section) {
            button->setSelectionFollowsFocus(selectionFollowsFocus && _allowsFocusSelection);
        }
    }
}

void NXTabBar::setAllowsFocusSelection(bool allowed) {
    if (_allowsFocusSelection == allowed) return;
    _allowsFocusSelection = allowed;
    for (const auto& section : _buttons) {
        for (const auto& button : section) {
            button->setSelectionFollowsFocus(_selectionFollowsFocus && allowed);
        }
    }
}

NXTabBarController::NXTabBarController(ViewControllerSection controllers) {
    setViewControllers(std::move(controllers));
}

NXTabBarController::NXTabBarController(ViewControllerSections controllers) {
    setViewControllerSections(std::move(controllers));
}

std::vector<std::shared_ptr<UIViewController>> NXTabBarController::viewControllers() const {
    std::vector<std::shared_ptr<UIViewController>> result;
    for (const auto& section : _viewControllers) {
        result.insert(result.end(), section.begin(), section.end());
    }
    return result;
}

void NXTabBarController::validateViewControllers(const ViewControllerSections& controllers) {
    std::unordered_set<const UIViewController*> uniqueControllers;
    for (const auto& section : controllers) {
        for (const auto& controller : section) {
            if (!controller) {
                throw std::invalid_argument(
                    "NXTabBarController does not accept null view controllers"
                );
            }
            if (!uniqueControllers.insert(controller.get()).second) {
                throw std::invalid_argument(
                    "NXTabBarController does not accept duplicate view controllers"
                );
            }
        }
    }
}

void NXTabBarController::setViewControllers(ViewControllerSection controllers) {
    ViewControllerSections sections;
    if (!controllers.empty()) {
        sections.push_back(std::move(controllers));
    }
    setViewControllerSections(std::move(sections));
}

void NXTabBarController::setViewControllerSections(ViewControllerSections controllers) {
    validateViewControllers(controllers);

    for (const auto& section : controllers) {
        for (const auto& controller : section) {
            if (controller.get() == this) {
                throw std::invalid_argument(
                    "NXTabBarController cannot contain itself"
                );
            }
            if (const auto parent = controller->parent().lock(); parent && parent.get() != this
                && !(controller == _compactController && parent == _compactNavigation.lock())) {
                throw std::invalid_argument(
                    "An NXTabBarController child already belongs to another container"
                );
            }
        }
    }

    const auto previousViewControllers = viewControllers();
    const auto previouslySelected = selectedViewController();
    _viewControllers = std::move(controllers);
    _selectedIndexPath = indexPathOf(previouslySelected);
    if (!_selectedIndexPath) {
        _selectedIndexPath = firstIndexPath();
    }

    if (viewIsLoaded()) {
        reloadTabBarItems();
        attachConfiguredChildren();
        updateTabSelection();
        removeUnconfiguredChildren(previousViewControllers);
    }
}

bool NXTabBarController::contains(const IndexPath& indexPath) const {
    if (indexPath.section() < 0 || indexPath.item() < 0) {
        return false;
    }

    const auto section = static_cast<size_t>(indexPath.section());
    const auto item = static_cast<size_t>(indexPath.item());
    return section < _viewControllers.size() && item < _viewControllers[section].size();
}

std::optional<IndexPath> NXTabBarController::firstIndexPath() const {
    for (size_t section = 0; section < _viewControllers.size(); ++section) {
        if (!_viewControllers[section].empty()) {
            return IndexPath(0, static_cast<int>(section));
        }
    }
    return std::nullopt;
}

std::optional<IndexPath> NXTabBarController::indexPathForFlatIndex(size_t index) const {
    size_t offset = 0;
    for (size_t section = 0; section < _viewControllers.size(); ++section) {
        if (index < offset + _viewControllers[section].size()) {
            return IndexPath(
                static_cast<int>(index - offset),
                static_cast<int>(section)
            );
        }
        offset += _viewControllers[section].size();
    }
    return std::nullopt;
}

std::optional<IndexPath> NXTabBarController::indexPathOf(
    const std::shared_ptr<UIViewController>& viewController
) const {
    if (!viewController) {
        return std::nullopt;
    }

    for (size_t section = 0; section < _viewControllers.size(); ++section) {
        const auto& controllers = _viewControllers[section];
        const auto iterator = std::find(controllers.begin(), controllers.end(), viewController);
        if (iterator != controllers.end()) {
            return IndexPath(
                static_cast<int>(std::distance(controllers.begin(), iterator)),
                static_cast<int>(section)
            );
        }
    }
    return std::nullopt;
}

std::shared_ptr<UIViewController> NXTabBarController::viewControllerAt(
    const IndexPath& indexPath
) const {
    if (!contains(indexPath)) {
        return nullptr;
    }
    return _viewControllers[static_cast<size_t>(indexPath.section())]
                           [static_cast<size_t>(indexPath.item())];
}

std::optional<size_t> NXTabBarController::selectedIndex() const {
    if (!_selectedIndexPath || !contains(*_selectedIndexPath)) {
        return std::nullopt;
    }

    size_t result = static_cast<size_t>(_selectedIndexPath->item());
    for (int section = 0; section < _selectedIndexPath->section(); ++section) {
        result += _viewControllers[static_cast<size_t>(section)].size();
    }
    return result;
}

bool NXTabBarController::setSelectedIndex(size_t index) {
    const auto indexPath = indexPathForFlatIndex(index);
    return indexPath && setSelectedIndexPath(*indexPath);
}

bool NXTabBarController::setSelectedIndexPath(const IndexPath& indexPath) {
    if (!contains(indexPath)) {
        return false;
    }

    _selectedIndexPath = indexPath;
    if (_tabBar) {
        _tabBar->setSelectedIndexPath(indexPath);
    }
    if (viewIsLoaded()) {
        updateTabSelection();
    }
    return true;
}

std::shared_ptr<UIViewController> NXTabBarController::selectedViewController() const {
    if (!_selectedIndexPath) {
        return nullptr;
    }
    return viewControllerAt(*_selectedIndexPath);
}

bool NXTabBarController::setSelectedViewController(
    const std::shared_ptr<UIViewController>& viewController
) {
    const auto indexPath = indexPathOf(viewController);
    return indexPath && setSelectedIndexPath(*indexPath);
}

std::shared_ptr<NXTabBar> NXTabBarController::tabBar() {
    loadViewIfNeeded();
    return _tabBar;
}

void NXTabBarController::setTabBarWidth(NXFloat tabBarWidth) {
    if (!std::isfinite(tabBarWidth) || tabBarWidth <= 0) {
        throw std::invalid_argument("NXTabBarController tab bar width must be finite and positive");
    }
    if (_tabBarWidth == tabBarWidth) {
        return;
    }

    _tabBarWidth = tabBarWidth;
    if (_tabBar) updateAdaptiveLayout();
}

void NXTabBarController::loadView() {
    auto contentView = new_shared<UIView>();

    _tabBar = new_shared<NXTabBar>();
    _tabBar->setAutolayoutEnabled(true);
    _tabBar->configureLayout([this](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue { _tabBarWidth, YGUnitPoint });
    });

    const auto weakSelf = weak_from_base<NXTabBarController>();
    _tabBar->_selectionHandler = [weakSelf](const IndexPath& indexPath) {
        if (const auto self = weakSelf.lock()) {
            return self->handleUserSelection(indexPath);
        }
        return false;
    };
    _tabBar->_primaryActionHandler = [weakSelf](const IndexPath&) {
        if (const auto self = weakSelf.lock()) {
            self->focusPresentedViewController();
        }
    };

    _contentView = new_shared<UIView>();
    _contentView->setAutolayoutEnabled(true);
    _contentView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexGrow(1);
        layout->setFlexShrink(1);
        layout->setFlexBasis(0_pt);
        layout->setAlignItems(YGAlignStretch);
    });
    NXResponderAction {
        .button = NXActionButton::b,
        .isEnabled = true,
        .action = UIAction("Back", [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                self->focusSelectedTab();
            }
        }),
        .canPerform = [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                return self->canReturnFocusToSelectedTab();
            }
            return false;
        },
    }.registerOn(shared_from_base<NXTabBarController>());

    contentView->addSubview(_tabBar);
    contentView->addSubview(_contentView);
    contentView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignStretch);
    });

    setView(contentView);
}

void NXTabBarController::viewDidLoad() {
    UIViewController::viewDidLoad();
    reloadTabBarItems();
    attachConfiguredChildren();
    updateAdaptiveLayout();
    updateTabSelection();
}

void NXTabBarController::viewWillAppear(bool animated) {
    if (const auto selected = _embeddedViewController) {
        selected->viewWillAppear(animated);
    }
}

void NXTabBarController::viewDidAppear(bool animated) {
    if (const auto selected = _embeddedViewController) selected->viewDidAppear(animated);
    // Modal dismissal clears its relationships after viewDidAppear returns.
    if (_compactController) scheduleAdaptivePresentation();
    else updateAdaptivePresentation();
}

void NXTabBarController::viewWillDisappear(bool animated) {
    if (const auto selected = _embeddedViewController) {
        selected->viewWillDisappear(animated);
    }
}

void NXTabBarController::viewDidDisappear(bool animated) {
    if (const auto selected = _embeddedViewController) {
        selected->viewDidDisappear(animated);
    }
}

void NXTabBarController::reloadTabBarItems() {
    if (!_tabBar) {
        return;
    }

    NXTabBar::Items titles;
    titles.reserve(_viewControllers.size());
    for (const auto& section : _viewControllers) {
        NXTabBar::ItemSection sectionTitles;
        sectionTitles.reserve(section.size());
        for (const auto& viewController : section) {
            sectionTitles.push_back(viewController->title());
        }
        titles.push_back(std::move(sectionTitles));
    }

    _tabBar->setItems(std::move(titles));
    restoreTabBarSelection();
}

void NXTabBarController::attachConfiguredChildren() {
    const auto self = shared_from_this();
    for (const auto& controller : viewControllers()) {
        if (controller == _compactController) continue;
        if (const auto parent = controller->parent().lock()) {
            if (parent.get() != this) {
                throw std::invalid_argument(
                    "An NXTabBarController child already belongs to another container"
                );
            }
            continue;
        }

        addChild(controller);
        controller->didMoveToParent(self);
    }
}

void NXTabBarController::removeUnconfiguredChildren(
    const std::vector<std::shared_ptr<UIViewController>>& previousViewControllers
) {
    for (const auto& controller : previousViewControllers) {
        if (indexPathOf(controller)) {
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

bool NXTabBarController::handleUserSelection(const IndexPath& indexPath) {
    updateAdaptivePresentation();
    const auto candidate = viewControllerAt(indexPath);
    if (!candidate) {
        restoreTabBarSelection();
        return false;
    }

    const auto strongDelegate = delegate.lock();
    const auto self = shared_from_base<NXTabBarController>();
    if (strongDelegate && !strongDelegate->tabBarControllerShouldSelect(self, candidate)) {
        restoreTabBarSelection();
        return false;
    }

    // A delegate is allowed to mutate the controller list. Revalidate the request
    // after the callback before committing the selection.
    if (viewControllerAt(indexPath) != candidate || !setSelectedIndexPath(indexPath)) {
        restoreTabBarSelection();
        return false;
    }

    if (strongDelegate) strongDelegate->tabBarControllerDidSelect(self, candidate);
    // A selection callback can replace the list or the selection again.
    if (_compactMode && selectedViewController() == candidate) {
        _compactPresentationRequested = true;
        updateAdaptivePresentation();
    }
    return true;
}

bool NXTabBarController::focusPresentedViewController() {
    if (!_embeddedViewController) {
        return false;
    }
    const auto window = view()->window();
    return window && window->focusSystem()->requestFocusUpdate(
        _embeddedViewController
    );
}

bool NXTabBarController::canReturnFocusToSelectedTab() {
    if (_compactMode) return false;
    if (!_tabBar || !_selectedIndexPath || !_tabBar->contains(*_selectedIndexPath)) {
        return false;
    }
    const auto window = viewIsLoaded() ? view()->window() : nullptr;
    if (!window) {
        return false;
    }
    const auto focusedView = std::dynamic_pointer_cast<UIView>(
        window->focusSystem()->focusedItem().lock()
    );
    return !focusedView || !focusedView->isDescendantOf(_tabBar);
}

bool NXTabBarController::focusSelectedTab() {
    if (!_tabBar || !_selectedIndexPath || !_tabBar->contains(*_selectedIndexPath)) {
        return false;
    }
    const auto section = static_cast<size_t>(_selectedIndexPath->section());
    const auto item = static_cast<size_t>(_selectedIndexPath->item());
    const auto button = _tabBar->_buttons[section][item];
    const auto window = view()->window();
    return button && window
        && window->focusSystem()->requestFocusUpdate(button);
}

void NXTabBarController::restoreTabBarSelection() {
    if (!_tabBar) {
        return;
    }
    if (_selectedIndexPath && _tabBar->setSelectedIndexPath(*_selectedIndexPath)) {
        return;
    }
    _tabBar->clearSelection();
}

void NXTabBarController::updateTabSelection() {
    if (_compactMode || _compactController) {
        updateAdaptivePresentation();
        return;
    }
    const auto nextViewController = selectedViewController();
    if (_embeddedViewController == nextViewController) {
        return;
    }

    const bool isVisible = viewIsLoaded() && view()->window() != nullptr;
    const auto previousViewController = _embeddedViewController;

    if (isVisible && previousViewController) {
        previousViewController->viewWillDisappear(false);
    }
    if (isVisible && nextViewController) {
        nextViewController->viewWillAppear(false);
    }

    if (previousViewController && previousViewController->viewIsLoaded()) {
        previousViewController->view()->removeFromSuperview();
    }

    _embeddedViewController = nextViewController;
    if (nextViewController) {
        if (const auto parent = nextViewController->parent().lock(); parent && parent.get() != this) {
            throw std::logic_error(
                "The selected NXTabBarController child belongs to another container"
            );
        } else if (!parent) {
            addChild(nextViewController);
            nextViewController->didMoveToParent(shared_from_this());
        }
        nextViewController->view()->setAutolayoutEnabled(true);
        nextViewController->view()->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(100_percent);
            layout->setHeight(100_percent);
        });
        _contentView->addSubview(nextViewController->view());
    }

    if (isVisible && previousViewController) {
        previousViewController->viewDidDisappear(false);
    }
    if (isVisible && nextViewController) {
        nextViewController->viewDidAppear(false);
    }

    if (isVisible) {
        if (const auto window = view()->window()) {
            window->updateFocus();
        }
    }
}

bool NXTabBarController::isCompactLayout() const {
    const auto traits = traitCollection();
    return traits && traits->horizontalSizeClass() == UIUserInterfaceSizeClass::compact;
}

std::shared_ptr<NXNavigationController> NXTabBarController::enclosingNavigationController() {
    for (auto ancestor = parent().lock(); ancestor; ancestor = ancestor->parent().lock()) {
        if (const auto navigation = std::dynamic_pointer_cast<NXNavigationController>(ancestor)) return navigation;
    }
    return nullptr;
}

void NXTabBarController::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) {
    UIViewController::traitCollectionDidChange(std::move(previous));
    if (viewIsLoaded()) updateAdaptiveLayout();
}

void NXTabBarController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    updateAdaptiveLayout();
}

void NXTabBarController::updateAdaptiveLayout() {
    if (!_tabBar || !_contentView) return;
    const bool compact = isCompactLayout();
    if (_compactMode != compact) {
        _compactMode = compact;
        // A fresh collapse shows the list. An interrupted expansion keeps an
        // already open compact destination until the latest mode can settle.
        _compactPresentationRequested = compact && _compactController != nullptr;
    }
    _tabBar->setAllowsFocusSelection(!compact);
    _tabBar->configureLayout([compact, width = _tabBarWidth](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(compact ? 100_percent : YGValue{width,YGUnitPoint});
    });
    _contentView->setHidden(compact);
    _contentView->configureLayout([compact](const std::shared_ptr<YGLayout>& layout) {
        layout->setDisplay(compact ? YGDisplayNone : YGDisplayFlex);
    });
    if (compact && _embeddedViewController) {
        const auto previous = std::exchange(_embeddedViewController,nullptr);
        const bool visible = view()->window() != nullptr;
        if (visible) previous->viewWillDisappear(false);
        if (previous->viewIsLoaded()) previous->view()->removeFromSuperview();
        if (visible) previous->viewDidDisappear(false);
    }
    updateAdaptivePresentation();
}

void NXTabBarController::scheduleAdaptivePresentation() {
    if (_adaptivePresentationScheduled) return;
    _adaptivePresentationScheduled = true;
    DispatchQueue::main()->async([weak = weak_from_base<NXTabBarController>()] {
        if (const auto self = weak.lock()) {
            self->_adaptivePresentationScheduled = false;
            self->updateAdaptivePresentation();
        }
    });
}

void NXTabBarController::returnToTabContainer(const std::shared_ptr<UIViewController>& controller) {
    if (!indexPathOf(controller) || !controller->parent().expired()) return;
    addChild(controller);
    controller->didMoveToParent(shared_from_this());
}

void NXTabBarController::releaseCompactController() {
    const auto controller = std::exchange(_compactController,nullptr);
    if (!_closingCompactPresentation) _compactPresentationRequested = false;
    _closingCompactPresentation = false;
    _compactNavigation.reset();
    if (controller) returnToTabContainer(controller);
}

void NXTabBarController::updateAdaptivePresentation() {
    if (!_tabBar || !_contentView) return;
    if (_updatingAdaptivePresentation) {
        _needsAdaptivePresentation = true;
        return;
    }
    _updatingAdaptivePresentation = true;
    struct Restore { bool& flag; ~Restore() { flag = false; } } restore{_updatingAdaptivePresentation};
    do {
        _needsAdaptivePresentation = false;
        const auto selected = selectedViewController();
        auto navigation = _compactController ? _compactNavigation.lock() : enclosingNavigationController();
        if (navigation && _observedNavigation.lock() != navigation) {
            _observedNavigation = navigation;
            navigation->observeStackUpdates(shared_from_this(), [
                weak = weak_from_base<NXTabBarController>(),
                source = std::weak_ptr<NXNavigationController>(navigation)
            ] {
                if (const auto self = weak.lock(); self && self->_observedNavigation.lock() == source.lock()) {
                    self->updateAdaptivePresentation();
                }
            });
        }
        // Never reparent a controller whose push/pop is still in progress.
        // The observer retries once the queued stack operations have settled.
        if (navigation && navigation->isTransitioning()) break;

        if (_compactController) {
            const bool close = !_compactMode || !_compactPresentationRequested || selected != _compactController;
            if (navigation) {
                auto stack = navigation->viewControllers();
                const auto entry = std::find(stack.begin(),stack.end(),_compactController);
                if (entry == stack.end()) {
                    // Back has already completed. Preserve the selection, but
                    // keep the compact list visible instead of reopening it.
                    releaseCompactController();
                } else if (close) {
                    _closingCompactPresentation = true;
                    stack.erase(entry); // Leave unrelated pushes above this tab intact.
                    navigation->setViewControllers(std::move(stack),false);
                    if (navigation->isTransitioning()) break;
                    releaseCompactController();
                }
            } else if (!_compactController->presentingViewController()
                       && !_compactController->isBeingPresented()) {
                releaseCompactController();
            } else if (close && !_closingCompactPresentation) {
                _closingCompactPresentation = true;
                const std::weak_ptr<UIViewController> destination = _compactController;
                _compactController->dismiss(false,[weak = weak_from_base<NXTabBarController>(),destination] {
                    if (const auto self = weak.lock(); self && self->_compactController == destination.lock()) {
                        self->releaseCompactController();
                        self->updateAdaptivePresentation();
                    }
                });
            }
        }

        if (!_compactMode && !_compactController) {
            updateTabSelection();
        } else if (_compactMode && !_compactController && _compactPresentationRequested && selected) {
            navigation = enclosingNavigationController();
            if (!navigation && view()->window() == nullptr) break;
            if (const auto owner = selected->parent().lock(); owner.get() == this) {
                selected->willMoveToParent(nullptr);
                if (selected->viewIsLoaded()) selected->view()->removeFromSuperview();
                selected->removeFromParent();
            }
            _compactController = selected;
            _compactNavigation = navigation;
            try {
                // Use the actual tab controller, preserving its navigation item,
                // responder chain, view state and any internal navigation stack.
                show(selected,shared_from_this());
            } catch (...) {
                _compactPresentationRequested = false;
                releaseCompactController();
                throw;
            }
        }
    } while (_needsAdaptivePresentation);
    // Overlay presentations do not make the list disappear/reappear. Observe
    // their dismissal on the main queue too, without replacing the tab's own
    // presentation delegate. Each drain schedules at most one later check.
    if (_compactController && _compactNavigation.expired()) scheduleAdaptivePresentation();
}
