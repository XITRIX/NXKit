#include <NXTabBarController.h>
#include <NXResponderAction.h>

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
    if (_selectionFollowsFocus && next.get() == this && previous != next) {
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
            button->setSelectionFollowsFocus(_selectionFollowsFocus);
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
            button->setSelectionFollowsFocus(selectionFollowsFocus);
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
            if (const auto parent = controller->parent().lock(); parent && parent.get() != this) {
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
    if (_tabBar) {
        _tabBar->configureLayout([tabBarWidth](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(YGValue { tabBarWidth, YGUnitPoint });
        });
    }
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
    updateTabSelection();
}

void NXTabBarController::viewWillAppear(bool animated) {
    if (const auto selected = selectedViewController()) {
        selected->viewWillAppear(animated);
    }
}

void NXTabBarController::viewDidAppear(bool animated) {
    if (const auto selected = selectedViewController()) {
        selected->viewDidAppear(animated);
    }
}

void NXTabBarController::viewWillDisappear(bool animated) {
    if (const auto selected = selectedViewController()) {
        selected->viewWillDisappear(animated);
    }
}

void NXTabBarController::viewDidDisappear(bool animated) {
    if (const auto selected = selectedViewController()) {
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

    if (strongDelegate) {
        strongDelegate->tabBarControllerDidSelect(self, candidate);
    }
    return true;
}

bool NXTabBarController::focusPresentedViewController() {
    if (!_presentedViewController) {
        return false;
    }
    const auto window = view()->window();
    return window && window->focusSystem()->requestFocusUpdate(
        _presentedViewController
    );
}

bool NXTabBarController::canReturnFocusToSelectedTab() {
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
    const auto nextViewController = selectedViewController();
    if (_presentedViewController == nextViewController) {
        return;
    }

    const bool isVisible = viewIsLoaded() && view()->window() != nullptr;
    const auto previousViewController = _presentedViewController;

    if (isVisible && previousViewController) {
        previousViewController->viewWillDisappear(false);
    }
    if (isVisible && nextViewController) {
        nextViewController->viewWillAppear(false);
    }

    if (previousViewController && previousViewController->viewIsLoaded()) {
        previousViewController->view()->removeFromSuperview();
    }

    _presentedViewController = nextViewController;
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
