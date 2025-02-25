//
// Created by Даниил Виноградов on 22.02.2025.
//

#include "NXTabBarController.h"

using namespace NXKit;

NXTabBarButton::NXTabBarButton() {
    _rectView = new_shared<UIView>();
    _titleLabel = new_shared<UILabel>();

    _rectView->setAlpha(0);
    _rectView->setBackgroundColor(UIColor::tint);

    _titleLabel->setHidden(true);
    _titleLabel->setFontSize(22);
//        _titleLabel->setFontWeight(600);

    addSubview(_rectView);
    addSubview(_titleLabel);

    _rectView->setAutolayoutEnabled(true);
    _titleLabel->setAutolayoutEnabled(true);

    _rectView->configureLayout([](const std::shared_ptr<YGLayout> &layout) {
        layout->setWidth(4_pt);
        layout->setMarginVertical(9_pt);
    });

    configureLayout([](const std::shared_ptr<YGLayout> &layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setJustifyContent(YGJustifyFlexStart);
        layout->setAlignItems(YGAlignStretch);
        layout->setAllGap(8);
        layout->setHeight(70_pt);
        layout->setPaddingHorizontal(8_pt);
    });

    layer()->setCornerRadius(4);
    layer()->setBorderWidth(4);

    std::for_each(subviews().begin(), subviews().end(), [](auto item) {
        item->setUserInteractionEnabled(false);
    });
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

void NXTabBar::setItems(const std::vector<std::vector<std::string>>& items) {
    // Clear old items
    auto _subviews = _container->subviews();
    for (const auto & subview : _subviews) {
        subview->removeFromSuperview();
    }

    // Fill new items
    _buttons.clear();
    for (int section = 0; section < items.size(); section++) {
        std::vector<std::shared_ptr<NXTabBarButton>> sectionButtons;

        if (section > 0) {
            auto separator = new_shared<NXSeparatorView>();
            _container->addSubview(separator);
        }

        for (int item = 0; item < items[section].size(); item++) {
            auto button = new_shared<NXTabBarButton>();
            sectionButtons.push_back(button);
            button->setAutolayoutEnabled(true);
            _container->addSubview(button);
            button->setText(items[section][item]);
            button->primaryAction = UIAction("", [this, section, item, button]() {
                if (!selectionDidChange.has_value()) return;

                this->_buttons[_selected.section()][_selected.item()]->setSelected(false);
                button->setSelected(true);

                _selected = IndexPath(item, section);
                this->selectionDidChange.value().perform();
            });
        }
        _buttons.push_back(sectionButtons);
    }
}

NXTabBarController::NXTabBarController(const std::vector<std::shared_ptr<UIViewController>>& controllers) {
    _viewControllers.push_back(controllers);
}

NXTabBarController::NXTabBarController(const std::vector<std::vector<std::shared_ptr<UIViewController>>>& controllers) {
    _viewControllers = controllers;
}

void NXTabBarController::loadView() {
    auto contentView = new_shared<UIView>();

    _tabBar = new_shared<NXTabBar>();
    _tabBar->setAutolayoutEnabled(true);
    contentView->addSubview(_tabBar);

    _tabBar->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(410_pt);
    });

    contentView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignStretch);
    });

    setView(contentView);

    _tabBar->selectionDidChange = UIAction("", [this]() {
        this->_selected = _tabBar->selected();
        this->updateTabSelection();
    });
}

void NXTabBarController::viewDidLoad() {
    std::vector<std::vector<std::string>> titles;
    titles.reserve(_viewControllers.size());

    for (const auto & section : _viewControllers) {
        std::vector<std::string> sections;
        sections.reserve(section.size());

        for (const auto & item : section) {
            sections.push_back(item->title());
        }
        titles.push_back(sections);
    }

    _tabBar->setItems(titles);
    updateTabSelection();
}

void NXTabBarController::updateTabSelection() {
    auto newPresentedViewController = _viewControllers[_selected.section()][_selected.item()];
    if (newPresentedViewController == nullptr || _presentedViewController == newPresentedViewController) return;

    if (_presentedViewController != nullptr) {
        _presentedViewController->willMoveToParent(nullptr);
        _presentedViewController->view()->removeFromSuperview();
        _presentedViewController->removeFromParent();
    }

    _presentedViewController = newPresentedViewController;
    newPresentedViewController->view()->setAutolayoutEnabled(true);

    addChild(_presentedViewController);
    view()->insertSubviewAt(_presentedViewController->view(), 1);
    _presentedViewController->didMoveToParent(shared_from_this());

    _presentedViewController->view()->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
//        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setFlexGrow(1);
    });
}
