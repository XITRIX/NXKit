#include <Screens/NavigationTestViewController/NavigationTestViewController.hpp>

#include <NXNavigationController.h>
#include <NXResponderAction.h>
#include <UIButton.h>
#include <UILabel.h>
#include <UIPresentationController.h>
#include <UIScrollView.h>
#include <UIViewControllerTransitioning.h>

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
class ModalPresentationTestViewController;

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

void updateScrollableContent(
    const std::shared_ptr<UIScrollView>& scrollView,
    const std::shared_ptr<UIView>& contentContainer
) {
    if (!scrollView || !contentContainer) {
        return;
    }

    configureContentContainer(
        contentContainer,
        scrollView->safeAreaInsets(),
        scrollView->bounds().width()
    );
    scrollView->setContentSize(NXSize(
        std::max<NXFloat>(0, scrollView->bounds().width()),
        std::max<NXFloat>(0, contentContainer->frame().height())
    ));
}

void showDestination(
    const std::shared_ptr<UIViewController>& source,
    std::string title,
    std::string detail,
    DestinationChrome chrome,
    int depth
);

void showModalPresentationTests(const std::shared_ptr<UIViewController>& source);

class CardPresentationController final : public UIPresentationController {
public:
    using UIPresentationController::UIPresentationController;

    NXRect frameOfPresentedViewInContainerView() const override {
        const auto container = containerView();
        if (!container) {
            return NXRect();
        }

        const auto bounds = container->bounds();
        const auto width = std::min<NXFloat>(640, std::max<NXFloat>(0, bounds.width() - 80));
        const auto height = std::min<NXFloat>(520, std::max<NXFloat>(0, bounds.height() - 80));
        return NXRect(
            bounds.midX() - width * 0.5f,
            bounds.midY() - height * 0.5f,
            width,
            height
        );
    }

    bool shouldRemovePresentersView() const override { return false; }

    void presentationTransitionWillBegin() override {
        const auto container = containerView();
        if (!container) {
            return;
        }

        _dimmingView = new_shared<UIView>(container->bounds());
        _dimmingView->setBackgroundColor(UIColor::black);
        _dimmingView->setAlpha(0);
        container->addSubview(_dimmingView);
        UIView::animate(
            0.45,
            0,
            UIViewAnimationOptions(
                preferredFramesPerSecond120 | allowUserInteraction
            ),
            [dimmingView = _dimmingView]() {
                dimmingView->setAlpha(0.58f);
            }
        );
    }

    void presentationTransitionDidEnd(bool completed) override {
        if (!completed && _dimmingView) {
            _dimmingView->removeFromSuperview();
            _dimmingView.reset();
        }
    }

    void dismissalTransitionWillBegin() override {
        if (!_dimmingView) {
            return;
        }
        UIView::animate(
            0.35,
            0,
            UIViewAnimationOptions(
                preferredFramesPerSecond120 | allowUserInteraction
            ),
            [dimmingView = _dimmingView]() {
                dimmingView->setAlpha(0);
            }
        );
    }

    void dismissalTransitionDidEnd(bool completed) override {
        if (!_dimmingView) {
            return;
        }
        if (completed) {
            _dimmingView->removeFromSuperview();
            _dimmingView.reset();
        } else {
            _dimmingView->setAlpha(0.58f);
        }
    }

    void containerViewWillLayoutSubviews() override {
        if (const auto container = containerView(); container && _dimmingView) {
            _dimmingView->setFrame(container->bounds());
        }
    }

private:
    std::shared_ptr<UIView> _dimmingView;
};

class CardTransitionAnimator final : public UIViewControllerAnimatedTransitioning {
public:
    explicit CardTransitionAnimator(bool presenting) : _presenting(presenting) {}

    double transitionDuration(
        const std::shared_ptr<UIViewControllerContextTransitioning>&
    ) const override {
        return _presenting ? 0.45 : 0.35;
    }

    void animateTransition(
        const std::shared_ptr<UIViewControllerContextTransitioning>& context
    ) override {
        const auto key = _presenting
            ? UITransitionContextViewKey::to
            : UITransitionContextViewKey::from;
        const auto card = context->viewForKey(key);
        if (!card) {
            context->completeTransition(false);
            return;
        }

        const auto hiddenTransform = NXAffineTransform::translationBy(0, 96)
            * NXAffineTransform::scaleBy(0.9f, 0.9f);
        if (_presenting) {
            card->setAlpha(0);
            card->setTransform(hiddenTransform);
        }

        const auto animations = [card, hiddenTransform, presenting = _presenting]() {
            card->setAlpha(presenting ? 1 : 0);
            card->setTransform(
                presenting ? NXAffineTransform::identity : hiddenTransform
            );
        };
        if (!context->isAnimated()) {
            animations();
            context->completeTransition(true);
            if (!_presenting) {
                card->setAlpha(1);
                card->setTransform(NXAffineTransform::identity);
            }
            return;
        }

        UIView::animate(
            transitionDuration(context),
            0,
            0.82,
            0.15,
            UIViewAnimationOptions(
                curveEaseOut
                    | preferredFramesPerSecond120
                    | allowUserInteraction
            ),
            animations,
            [context, card, presenting = _presenting](bool finished) {
                context->completeTransition(finished);
                if (!presenting) {
                    card->setAlpha(1);
                    card->setTransform(NXAffineTransform::identity);
                }
            }
        );
    }

private:
    bool _presenting;
};

class CardTransitioningDelegate final : public UIViewControllerTransitioningDelegate {
public:
    std::shared_ptr<UIViewControllerAnimatedTransitioning>
    animationControllerForPresented(
        const std::shared_ptr<UIViewController>&,
        const std::shared_ptr<UIViewController>&,
        const std::shared_ptr<UIViewController>&
    ) override {
        return new_shared<CardTransitionAnimator>(true);
    }

    std::shared_ptr<UIViewControllerAnimatedTransitioning>
    animationControllerForDismissed(
        const std::shared_ptr<UIViewController>&
    ) override {
        return new_shared<CardTransitionAnimator>(false);
    }

    std::shared_ptr<UIPresentationController>
    presentationControllerForPresented(
        const std::shared_ptr<UIViewController>& presented,
        const std::shared_ptr<UIViewController>& presenting,
        const std::shared_ptr<UIViewController>&
    ) override {
        return new_shared<CardPresentationController>(presented, presenting);
    }
};

class ModalContentViewController final : public UIViewController {
public:
    ModalContentViewController(std::string title, std::string detail, bool card)
        : _detail(std::move(detail)), _card(card) {
        setTitle(std::move(title));
    }

    void retainTransitioningDelegate(
        const std::shared_ptr<UIViewControllerTransitioningDelegate>& delegate
    ) {
        _retainedTransitioningDelegate = delegate;
        setTransitioningDelegate(delegate);
    }

    void loadView() override {
        auto rootView = new_shared<UIView>();
        rootView->setBackgroundColor(
            _card ? UIColor::secondarySystemBackground : UIColor::systemBackground
        );
        rootView->setClipsToBounds(_card);
        rootView->layer()->setCornerRadius(_card ? 28 : 0);
        rootView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyCenter);
        });

        auto content = new_shared<UIView>();
        content->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(100_percent);
            layout->setFlexDirection(YGFlexDirectionColumn);
            layout->setAllGap(18);
            layout->setPaddingHorizontal(40_pt);
        });

        auto heading = new_shared<UILabel>();
        heading->setText(title());
        heading->setFontSize(32);
        heading->setFontWeight(600);
        heading->setAutolayoutEnabled(true);

        auto detail = new_shared<UILabel>();
        detail->setText(_detail);
        detail->setFontSize(20);
        detail->setTextColor(UIColor::secondaryLabel);
        detail->setAutolayoutEnabled(true);

        const auto weakSelf = weak_from_base<ModalContentViewController>();
        const std::function<void()> dismiss = [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                self->dismiss(true);
            }
        };
        auto dismissButton = makeButton("Dismiss presentation", dismiss);
        NXResponderAction {
            .button = NXActionButton::b,
            .isEnabled = true,
            .action = UIAction("Dismiss", dismiss),
        }.registerOn(dismissButton);

        content->addSubview(heading);
        content->addSubview(detail);
        content->addSubview(dismissButton);
        rootView->addSubview(content);
        setView(rootView);
    }

private:
    std::string _detail;
    bool _card;
    std::shared_ptr<UIViewControllerTransitioningDelegate>
        _retainedTransitioningDelegate;
};

class ModalPresentationTestViewController final : public UIViewController {
public:
    ModalPresentationTestViewController() {
        setTitle("Modal presentations");
    }

    void loadView() override {
        _scrollView = new_shared<UIScrollView>();
        _scrollView->setBackgroundColor(UIColor::systemBackground);
        _scrollView->setContentInsetAdjustmentBehavior(
            UIScrollViewContentInsetAdjustmentBehavior::never
        );
        _scrollView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setAlignItems(YGAlignCenter);
        });

        _contentContainer = new_shared<UIView>();
        configureContentContainer(_contentContainer, UIEdgeInsets::zero, 720);

        auto heading = new_shared<UILabel>();
        heading->setText("Modal presentation test");
        heading->setFontSize(30);
        heading->setFontWeight(600);
        heading->setAutolayoutEnabled(true);

        auto detail = new_shared<UILabel>();
        detail->setText(
            "Exercise UIKit's four modal transition styles, over-full-screen behavior, "
            "then a custom transitioning delegate and presentation controller."
        );
        detail->setFontSize(20);
        detail->setTextColor(UIColor::secondaryLabel);
        detail->setAutolayoutEnabled(true);

        _contentContainer->addSubview(heading);
        _contentContainer->addSubview(detail);

        const auto weakSelf = weak_from_base<ModalPresentationTestViewController>();
        struct StyleTest {
            const char* buttonTitle;
            const char* controllerTitle;
            const char* detail;
            UIModalTransitionStyle style;
        };
        const std::vector<StyleTest> styleTests {
            {
                "Present cover vertical",
                "Cover vertical",
                "The default full-screen presentation slides up from the bottom.",
                UIModalTransitionStyle::coverVertical,
            },
            {
                "Present flip horizontal",
                "Flip horizontal",
                "A portable horizontal flip transition presents this controller.",
                UIModalTransitionStyle::flipHorizontal,
            },
            {
                "Present cross dissolve",
                "Cross dissolve",
                "The presented content fades over the navigation interface.",
                UIModalTransitionStyle::crossDissolve,
            },
            {
                "Present partial curl",
                "Partial curl",
                "NXKit uses a portable 2D fold approximation for UIKit's page curl.",
                UIModalTransitionStyle::partialCurl,
            },
        };

        for (const auto& test : styleTests) {
            _contentContainer->addSubview(makeButton(
                test.buttonTitle,
                [weakSelf, test]() {
                    if (const auto self = weakSelf.lock()) {
                        auto modal = new_shared<ModalContentViewController>(
                            test.controllerTitle,
                            test.detail,
                            false
                        );
                        modal->setModalTransitionStyle(test.style);
                        self->present(modal, true);
                    }
                }
            ));
        }

        _contentContainer->addSubview(makeButton(
            "Present over full screen",
            [weakSelf]() {
                if (const auto self = weakSelf.lock()) {
                    auto modal = new_shared<ModalContentViewController>(
                        "Over full screen",
                        "The navigation hierarchy remains mounted while this modal "
                        "cross-dissolves above it.",
                        false
                    );
                    modal->setModalPresentationStyle(
                        UIModalPresentationStyle::overFullScreen
                    );
                    modal->setModalTransitionStyle(
                        UIModalTransitionStyle::crossDissolve
                    );
                    self->present(modal, true);
                }
            }
        ));

        _contentContainer->addSubview(makeButton(
            "Present custom card",
            [weakSelf]() {
                if (const auto self = weakSelf.lock()) {
                    auto modal = new_shared<ModalContentViewController>(
                        "Custom presentation",
                        "A UIPresentationController supplies the centered frame and "
                        "dimming chrome while custom animator objects drive both directions.",
                        true
                    );
                    modal->setModalPresentationStyle(UIModalPresentationStyle::custom);
                    modal->retainTransitioningDelegate(
                        new_shared<CardTransitioningDelegate>()
                    );
                    self->present(modal, true);
                }
            }
        ));

        _scrollView->addSubview(_contentContainer);
        setView(_scrollView);
    }

    void viewSafeAreaInsetsDidChange() override {
        UIViewController::viewSafeAreaInsetsDidChange();
        updateContentLayout();
    }

    void viewDidLayoutSubviews() override {
        UIViewController::viewDidLayoutSubviews();
        updateContentLayout();
    }

private:
    void updateContentLayout() {
        updateScrollableContent(_scrollView, _contentContainer);
    }

    std::shared_ptr<UIScrollView> _scrollView;
    std::shared_ptr<UIView> _contentContainer;
};

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

void showModalPresentationTests(const std::shared_ptr<UIViewController>& source) {
    source->show(new_shared<ModalPresentationTestViewController>(), source);
}

} // namespace

NavigationTestViewController::NavigationTestViewController() {
    setTitle("Navigation test");
}

void NavigationTestViewController::loadView() {
    _scrollView = new_shared<UIScrollView>();
    _scrollView->setBackgroundColor(UIColor::systemBackground);
    _scrollView->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    _scrollView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
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
    _contentContainer->addSubview(makeButton(
        "Push modal presentation tests",
        [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                showModalPresentationTests(self);
            }
        }
    ));

    _scrollView->addSubview(_contentContainer);
    setView(_scrollView);
}

void NavigationTestViewController::viewSafeAreaInsetsDidChange() {
    UIViewController::viewSafeAreaInsetsDidChange();
    updateScrollableContent(_scrollView, _contentContainer);
}

void NavigationTestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    updateScrollableContent(_scrollView, _contentContainer);
}
