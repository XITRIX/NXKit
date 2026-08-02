#include <Screens/NavigationTestViewController/NavigationTestViewController.hpp>

#include <NXNavigationController.h>
#include <NXResponderAction.h>
#include <UIButton.h>
#include <UILabel.h>

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

using namespace NXKit;
using namespace NXKit::yoga::literals;

namespace {

enum class DestinationChrome {
    standard,
    statusInNavigationBar,
    actionsInNavigationBar,
};

class NavigationDestinationViewController;

std::shared_ptr<NXNavigationController> enclosingNavigationController(
    const std::shared_ptr<UIViewController>& viewController
) {
    for (auto current = viewController; current; current = current->parent().lock()) {
        if (const auto navigationController =
                std::dynamic_pointer_cast<NXNavigationController>(current)) {
            return navigationController;
        }
    }
    return nullptr;
}

std::shared_ptr<UIButton> makeButton(
    std::string title,
    std::function<void()> handler
) {
    auto button = new_shared<UIButton>(UIButtonStyle::tinted);
    button->setText(title);
    button->layer()->setCornerRadius(8);
    button->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(100_percent);
        layout->setHeight(64_pt);
        layout->setPaddingHorizontal(20_pt);
    });
    button->primaryAction = UIAction(std::move(title), std::move(handler));
    return button;
}

void configureContentContainer(
    const std::shared_ptr<UIView>& contentContainer,
    UIEdgeInsets safeAreaInsets,
    NXFloat availableWidth
) {
    const auto contentWidth = availableWidth > 0
        ? std::min<NXFloat>(720, availableWidth)
        : 720;
    contentContainer->configureLayout([
        safeAreaInsets,
        contentWidth
    ](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue { static_cast<float>(contentWidth), YGUnitPoint });
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setAllGap(16);
        layout->setPaddingLeft(48_pt);
        layout->setPaddingRight(48_pt);
        layout->setPaddingTop(YGValue {
            static_cast<float>(safeAreaInsets.top + 36),
            YGUnitPoint
        });
        layout->setPaddingBottom(YGValue {
            static_cast<float>(safeAreaInsets.bottom + 36),
            YGUnitPoint
        });
    });
}

void showDestination(
    const std::shared_ptr<UIViewController>& source,
    std::string title,
    std::string detail,
    DestinationChrome chrome,
    int depth
);

class NavigationDestinationViewController final : public UIViewController {
public:
    NavigationDestinationViewController(
        std::string title,
        std::string detail,
        DestinationChrome chrome,
        int depth
    ) : _detail(std::move(detail)), _chrome(chrome), _depth(depth) {
        setTitle(std::move(title));
    }

    void loadView() override {
        auto rootView = new_shared<UIView>();
        rootView->setBackgroundColor(UIColor::systemBackground);
        rootView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setAlignItems(YGAlignCenter);
        });

        _contentContainer = new_shared<UIView>();
        configureContentContainer(_contentContainer, UIEdgeInsets::zero, 720);

        auto headingLabel = new_shared<UILabel>();
        headingLabel->setText("Navigation destination");
        headingLabel->setFontSize(30);
        headingLabel->setFontWeight(600);
        headingLabel->setAutolayoutEnabled(true);

        auto depthLabel = new_shared<UILabel>();
        depthLabel->setText("Stack depth exercised: " + std::to_string(_depth));
        depthLabel->setFontSize(20);
        depthLabel->setTextColor(UIColor::secondaryLabel);
        depthLabel->setAutolayoutEnabled(true);

        auto detailLabel = new_shared<UILabel>();
        detailLabel->setText(_detail);
        detailLabel->setFontSize(20);
        detailLabel->setTextColor(UIColor::secondaryLabel);
        detailLabel->setAutolayoutEnabled(true);
        detailLabel->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setMarginBottom(12_pt);
        });

        const auto weakSelf = weak_from_base<NavigationDestinationViewController>();
        const std::function<void()> pushDeeper = [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                showDestination(
                    self,
                    "Nested destination " + std::to_string(self->_depth + 1),
                    "This controller was pushed from another navigation destination.",
                    self->_chrome,
                    self->_depth + 1
                );
            }
        };
        auto deeperButton = makeButton("Push one level deeper", pushDeeper);
        NXResponderAction {
            .button = NXActionButton::a,
            .isEnabled = true,
            .action = UIAction("Push Deeper", pushDeeper),
        }.registerOn(deeperButton);

        _contentContainer->addSubview(headingLabel);
        _contentContainer->addSubview(depthLabel);
        _contentContainer->addSubview(detailLabel);
        _contentContainer->addSubview(deeperButton);
        rootView->addSubview(_contentContainer);
        setView(rootView);
    }

    void viewSafeAreaInsetsDidChange() override {
        UIViewController::viewSafeAreaInsetsDidChange();
        if (_contentContainer) {
            configureContentContainer(
                _contentContainer,
                view()->safeAreaInsets(),
                view()->bounds().width()
            );
        }
    }

    void viewDidLayoutSubviews() override {
        UIViewController::viewDidLayoutSubviews();
        if (_contentContainer) {
            configureContentContainer(
                _contentContainer,
                view()->safeAreaInsets(),
                view()->bounds().width()
            );
        }
    }

private:
    std::string _detail;
    DestinationChrome _chrome;
    int _depth;
    std::shared_ptr<UIView> _contentContainer;
};

void showDestination(
    const std::shared_ptr<UIViewController>& source,
    std::string title,
    std::string detail,
    DestinationChrome chrome,
    int depth
) {
    auto destination = new_shared<NavigationDestinationViewController>(
        std::move(title),
        std::move(detail),
        chrome,
        depth
    );

    if (const auto navigationController = enclosingNavigationController(source)) {
        const auto item = navigationController->navigationItemFor(destination);
        switch (chrome) {
            case DestinationChrome::standard:
                break;
            case DestinationChrome::statusInNavigationBar:
                item->setStatusWidgetPlacement(
                    NXNavigationWidgetPlacement::navigationTrailing
                );
                break;
            case DestinationChrome::actionsInNavigationBar:
                item->setActionsWidgetPlacement(
                    NXNavigationWidgetPlacement::navigationTrailing
                );
                break;
        }
    }

    source->show(destination, source);
}

} // namespace

NavigationTestViewController::NavigationTestViewController() {
    setTitle("Navigation test");
}

void NavigationTestViewController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::systemBackground);
    rootView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setAlignItems(YGAlignCenter);
    });

    _contentContainer = new_shared<UIView>();
    configureContentContainer(_contentContainer, UIEdgeInsets::zero, 720);

    auto titleLabel = new_shared<UILabel>();
    titleLabel->setText("Navigation controller test");
    titleLabel->setFontSize(30);
    titleLabel->setFontWeight(600);
    titleLabel->setAutolayoutEnabled(true);

    auto detailLabel = new_shared<UILabel>();
    detailLabel->setText(
        "Choose a destination. Each button uses UIViewController::show(), then B returns "
        "through the navigation stack."
    );
    detailLabel->setFontSize(20);
    detailLabel->setTextColor(UIColor::secondaryLabel);
    detailLabel->setAutolayoutEnabled(true);
    detailLabel->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setMarginBottom(12_pt);
    });

    const auto weakSelf = weak_from_base<NavigationTestViewController>();
    struct Destination {
        std::string buttonTitle;
        std::string controllerTitle;
        std::string detail;
        DestinationChrome chrome;
    };
    const std::vector<Destination> destinations {
        {
            "Push standard controller",
            "Standard destination",
            "Uses the default footer status and action-hint placements.",
            DestinationChrome::standard,
        },
        {
            "Push controller with header status",
            "Header status",
            "Moves the live battery and time widget into the navigation trailing corner.",
            DestinationChrome::statusInNavigationBar,
        },
        {
            "Push controller with header actions",
            "Header actions",
            "Moves the available B and A action hints into the navigation trailing corner.",
            DestinationChrome::actionsInNavigationBar,
        },
    };

    _contentContainer->addSubview(titleLabel);
    _contentContainer->addSubview(detailLabel);
    for (const auto& destination : destinations) {
        _contentContainer->addSubview(makeButton(
            destination.buttonTitle,
            [weakSelf, destination]() {
                if (const auto self = weakSelf.lock()) {
                    showDestination(
                        self,
                        destination.controllerTitle,
                        destination.detail,
                        destination.chrome,
                        1
                    );
                }
            }
        ));
    }

    rootView->addSubview(_contentContainer);
    setView(rootView);
}

void NavigationTestViewController::viewSafeAreaInsetsDidChange() {
    UIViewController::viewSafeAreaInsetsDidChange();
    if (_contentContainer) {
        configureContentContainer(
            _contentContainer,
            view()->safeAreaInsets(),
            view()->bounds().width()
        );
    }
}

void NavigationTestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    if (_contentContainer) {
        configureContentContainer(
            _contentContainer,
            view()->safeAreaInsets(),
            view()->bounds().width()
        );
    }
}
