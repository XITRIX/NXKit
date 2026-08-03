#include <NXNavigationController.h>
#include <NXTabBarController.h>
#include <CABasicAnimation.h>
#include <UIApplication.h>
#include <UIApplicationDelegate.h>
#include <UIWindow.h>

#include <SDL3/SDL.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace NXKit;

namespace NXKit {

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(
    UIApplication*,
    std::map<std::string, std::any>
) {
    return true;
}

} // namespace NXKit

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

void completePendingAnimations() {
    const auto animatedLayers = UIView::layersWithAnimations;
    for (const auto& layer : animatedLayers) {
        layer->animateAt(Timer(1'000'000));
    }
}

class RecordingViewController final : public UIViewController {
public:
    int addedToParentCount = 0;
    int removedFromParentCount = 0;
    int willAppearCount = 0;
    int didAppearCount = 0;
    int willDisappearCount = 0;
    int didDisappearCount = 0;

    void didMoveToParent(std::shared_ptr<UIViewController> parent) override {
        UIViewController::didMoveToParent(parent);
        if (parent) {
            ++addedToParentCount;
        } else {
            ++removedFromParentCount;
        }
    }

    void viewWillAppear(bool animated) override {
        UIViewController::viewWillAppear(animated);
        ++willAppearCount;
    }

    void viewDidAppear(bool animated) override {
        UIViewController::viewDidAppear(animated);
        ++didAppearCount;
    }

    void viewWillDisappear(bool animated) override {
        UIViewController::viewWillDisappear(animated);
        ++willDisappearCount;
    }

    void viewDidDisappear(bool animated) override {
        UIViewController::viewDidDisappear(animated);
        ++didDisappearCount;
    }
};

class RecordingAnimationView final : public UIView {
public:
    UIViewAnimationOptions lastAnimationOptions = UIViewAnimationOptions::none;

    std::shared_ptr<CABasicAnimation> actionForKey(std::string event) override {
        const auto animation = UIView::actionForKey(std::move(event));
        if (animation && animation->animationGroup) {
            lastAnimationOptions = animation->animationGroup->options;
        }
        return animation;
    }
};

class RecordingDelegate final : public NXNavigationControllerDelegate {
public:
    int willShowCount = 0;
    int didShowCount = 0;
    std::shared_ptr<UIViewController> lastWillShow;
    std::shared_ptr<UIViewController> lastDidShow;

    void navigationControllerWillShow(
        const std::shared_ptr<NXNavigationController>&,
        const std::shared_ptr<UIViewController>& viewController,
        bool
    ) override {
        ++willShowCount;
        lastWillShow = viewController;
    }

    void navigationControllerDidShow(
        const std::shared_ptr<NXNavigationController>&,
        const std::shared_ptr<UIViewController>& viewController,
        bool
    ) override {
        ++didShowCount;
        lastDidShow = viewController;
    }
};

} // namespace

int main() {
    auto yogaRoot = new_shared<UIView>();
    yogaRoot->setAutolayoutEnabled(true);
    yogaRoot->setFrame(NXRect(0, 0, 320, 180));
    yogaRoot->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
    });
    auto firstYogaParent = new_shared<UIView>();
    firstYogaParent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexGrow(1);
    });
    auto secondYogaParent = new_shared<UIView>();
    secondYogaParent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexGrow(1);
    });
    auto reparentedYogaChild = new_shared<UIView>();
    reparentedYogaChild->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setSize({40, 40});
    });
    firstYogaParent->addSubview(reparentedYogaChild);
    yogaRoot->addSubview(firstYogaParent);
    yogaRoot->addSubview(secondYogaParent);
    yogaRoot->layoutIfNeeded();

    secondYogaParent->addSubview(reparentedYogaChild);
    yogaRoot->layoutIfNeeded();
    expect(
        reparentedYogaChild->superview().lock() == secondYogaParent,
        "a Yoga-enabled view can be reparented between layout passes"
    );

    auto root = new_shared<RecordingViewController>();
    auto second = new_shared<RecordingViewController>();
    auto third = new_shared<RecordingViewController>();
    root->setTitle("Root");
    second->setTitle("Second");
    third->setTitle("Third");

    auto navigationController = new_shared<NXNavigationController>(root);
    auto rootNavigationItem = navigationController->navigationItemFor(root);
    expect(
        rootNavigationItem == navigationController->navigationItemFor(root),
        "each controller has one stable navigation item"
    );
    expect(
        rootNavigationItem->statusWidgetPlacement()
            == NXNavigationWidgetPlacement::toolbarLeading,
        "the system status widget defaults to the toolbar leading corner"
    );
    expect(
        rootNavigationItem->actionsWidgetPlacement()
            == NXNavigationWidgetPlacement::toolbarTrailing,
        "the actions widget defaults to the toolbar trailing corner"
    );
    expect(navigationController->topViewController() == root, "the root starts at the top");
    expect(
        navigationController->visibleViewController() == root,
        "the root is initially the visible controller"
    );
    expect(root->parent().expired(), "containment remains lazy until the navigation view loads");

    navigationController->loadViewIfNeeded();
    expect(
        navigationController->additionalSafeAreaInsets() == UIEdgeInsets(88, 0, 73, 0),
        "the overlay bars reserve their content heights in child safe areas"
    );
    expect(root->parent().lock() == navigationController, "loading contains the root controller");
    expect(root->addedToParentCount == 1, "the root receives one containment callback");
    expect(!root->view()->superview().expired(), "the root view is mounted after loading");

    const auto statusWidget = navigationController->defaultStatusWidget();
    const auto actionsWidget = navigationController->defaultActionsWidget();
    expect(statusWidget && !statusWidget->superview().expired(), "status is installed by default");
    expect(statusWidget->compact(), "status uses the compact footer representation by default");
    expect(actionsWidget && !actionsWidget->superview().expired(), "actions are installed by default");
    expect(
        actionsWidget->actions().size() == 1
            && actionsWidget->actions()[0].button == NXActionButton::a
            && !actionsWidget->actions()[0].isEnabled,
        "a disabled A action is always shown when no primary action is available"
    );

    const auto originalStatusContainer = statusWidget->superview().lock();
    rootNavigationItem->setStatusWidgetPlacement(
        NXNavigationWidgetPlacement::navigationTrailing
    );
    expect(
        statusWidget->superview().lock() != originalStatusContainer && !statusWidget->compact(),
        "the default status widget can move to the navigation trailing corner"
    );
    rootNavigationItem->setStatusWidgetPlacement(NXNavigationWidgetPlacement::toolbarLeading);
    expect(statusWidget->compact(), "moving status back to the toolbar restores compact mode");

    auto customStatusWidget = new_shared<UIView>();
    rootNavigationItem->setStatusWidget(customStatusWidget);
    expect(
        !customStatusWidget->superview().expired() && statusWidget->superview().expired(),
        "a navigation item can replace the default status widget"
    );
    rootNavigationItem->setStatusWidget(nullptr);
    expect(
        !statusWidget->superview().expired() && customStatusWidget->superview().expired(),
        "clearing a custom status widget restores the navigation-owned default"
    );

    const auto originalActionsContainer = actionsWidget->superview().lock();
    rootNavigationItem->setActionsWidgetPlacement(
        NXNavigationWidgetPlacement::navigationTrailing
    );
    expect(
        actionsWidget->superview().lock() != originalActionsContainer,
        "the default actions widget can move to a navigation-bar corner"
    );
    auto customActionsWidget = new_shared<UIView>();
    rootNavigationItem->setActionsWidget(customActionsWidget);
    expect(
        !customActionsWidget->superview().expired() && actionsWidget->superview().expired(),
        "a navigation item can replace the default actions widget"
    );
    rootNavigationItem->setActionsWidget(nullptr);
    rootNavigationItem->setActionsWidgetPlacement(NXNavigationWidgetPlacement::hidden);
    expect(actionsWidget->superview().expired(), "the actions widget can be hidden per screen");
    rootNavigationItem->setActionsWidgetPlacement(NXNavigationWidgetPlacement::toolbarTrailing);
    NXResponderAction rootAction {
        .button = NXActionButton::a,
        .isEnabled = true,
        .action = UIAction("Continue"),
    };
    rootAction.registerOn(root);
    actionsWidget->refresh();
    expect(
        actionsWidget->actions().size() == 1
            && actionsWidget->actions()[0]
                == NXResponderAction {
                    .button = NXActionButton::a,
                    .isEnabled = true,
                    .action = UIAction("Continue"),
                },
        "a responder A action replaces the disabled placeholder"
    );
    rootAction.unregisterFrom(root);

    auto delegate = new_shared<RecordingDelegate>();
    navigationController->delegate = delegate;
    navigationController->viewWillAppear(false);
    navigationController->viewDidAppear(false);
    expect(root->willAppearCount == 1, "appearance begins only for the visible root");
    expect(root->didAppearCount == 1, "appearance completes only for the visible root");
    expect(delegate->willShowCount == 1, "the delegate is told the initial controller will show");
    expect(delegate->didShowCount == 1, "the delegate is told the initial controller did show");

    navigationController->pushViewController(second, false);
    expect(navigationController->viewControllers().size() == 2, "push appends to the stack");
    expect(navigationController->topViewController() == second, "push updates the top controller");
    expect(
        navigationController->visibleViewController() == second,
        "push updates the visible controller"
    );
    expect(root->parent().lock() == navigationController, "push keeps the root contained");
    expect(second->parent().lock() == navigationController, "push contains the new controller");
    expect(root->view()->superview().expired(), "push unmounts the previous view");
    expect(!second->view()->superview().expired(), "push mounts the new top view");
    expect(
        actionsWidget->actions().size() == 2
            && actionsWidget->actions()[0].button == NXActionButton::b
            && actionsWidget->actions()[0].isEnabled
            && actionsWidget->actions()[1].button == NXActionButton::a
            && !actionsWidget->actions()[1].isEnabled,
        "a pushed screen automatically shows Back before its disabled A placeholder"
    );

    int initialOptionsTouchCount = 0;
    int replacementOptionsTouchCount = 0;
    int closeSoftwareTouchCount = 0;
    NXResponderAction {
        .button = NXActionButton::plus,
        .isEnabled = true,
        .action = UIAction("Options", [&initialOptionsTouchCount]() {
            ++initialOptionsTouchCount;
        }),
    }.registerOn(second);
    NXResponderAction {
        .button = NXActionButton::x,
        .isEnabled = true,
        .action = UIAction("Close Software", [&closeSoftwareTouchCount]() {
            ++closeSoftwareTouchCount;
        }),
    }.registerOn(second);
    NXResponderAction {
        .button = NXActionButton::a,
        .isEnabled = true,
        .action = UIAction("Continue"),
    }.registerOn(second);
    actionsWidget->refresh();
    expect(
        actionsWidget->actions().size() == 4
            && actionsWidget->actions()[0].button == NXActionButton::plus
            && actionsWidget->actions()[1].button == NXActionButton::x
            && actionsWidget->actions()[2].button == NXActionButton::b
            && actionsWidget->actions()[3].button == NXActionButton::a,
        "the legend combines the nearest responder actions with navigation Back"
    );

    const auto plusLegendControl = std::dynamic_pointer_cast<UIControl>(
        actionsWidget->subviews()[0]
    );
    const auto xLegendControl = std::dynamic_pointer_cast<UIControl>(
        actionsWidget->subviews()[1]
    );
    const auto backLegendControl = std::dynamic_pointer_cast<UIControl>(
        actionsWidget->subviews()[2]
    );
    const auto aLegendControl = std::dynamic_pointer_cast<UIControl>(
        actionsWidget->subviews()[3]
    );
    expect(
        plusLegendControl && xLegendControl && backLegendControl && aLegendControl,
        "every legend entry uses a consistent control-backed presentation"
    );
    expect(
        plusLegendControl && plusLegendControl->isUserInteractionEnabled()
            && plusLegendControl->primaryAction.has_value()
            && xLegendControl && xLegendControl->isUserInteractionEnabled()
            && xLegendControl->primaryAction.has_value()
            && backLegendControl && backLegendControl->isUserInteractionEnabled()
            && backLegendControl->primaryAction.has_value(),
        "enabled non-A legend actions are touch clickable"
    );
    expect(
        aLegendControl && !aLegendControl->isUserInteractionEnabled()
            && !aLegendControl->primaryAction.has_value(),
        "the A legend action remains display-only"
    );
    expect(
        plusLegendControl && !plusLegendControl->canBecomeFocused()
            && xLegendControl && !xLegendControl->canBecomeFocused()
            && backLegendControl && !backLegendControl->canBecomeFocused()
            && aLegendControl && !aLegendControl->canBecomeFocused(),
        "legend actions never participate in controller focus traversal"
    );

    NXResponderAction {
        .button = NXActionButton::plus,
        .isEnabled = true,
        .action = UIAction("Options", [&replacementOptionsTouchCount]() {
            ++replacementOptionsTouchCount;
        }),
    }.registerOn(second);
    actionsWidget->refresh();
    if (plusLegendControl && xLegendControl) {
        plusLegendControl->performPrimaryAction();
        xLegendControl->performPrimaryAction();
    }
    expect(
        initialOptionsTouchCount == 0 && replacementOptionsTouchCount == 1,
        "a retained legend row invokes the latest responder callback"
    );
    expect(
        closeSoftwareTouchCount == 1,
        "a touch-triggered legend control performs its responder action"
    );

    NXResponderAction childAction {
        .button = NXActionButton::a,
        .isEnabled = false,
        .action = UIAction("Unavailable here"),
    };
    childAction.registerOn(second->view());
    actionsWidget->refresh();
    expect(
        actionsWidget->actions().back().action.title() == "Unavailable here"
            && !actionsWidget->actions().back().isEnabled,
        "the first responder-chain action wins and a disabled action blocks its parent"
    );
    childAction.unregisterFrom(second->view());

    bool duplicateWasRejected = false;
    try {
        navigationController->pushViewController(second, false);
    } catch (const std::invalid_argument&) {
        duplicateWasRejected = true;
    }
    expect(duplicateWasRejected, "pushing a duplicate controller is rejected");
    expect(navigationController->viewControllers().size() == 2, "a rejected push is atomic");

    const auto popped = navigationController->popViewController(false);
    expect(popped == second, "pop returns the removed top controller");
    expect(navigationController->topViewController() == root, "pop reveals the previous controller");
    expect(second->parent().expired(), "a popped controller is removed from containment");
    expect(second->removedFromParentCount == 1, "a popped controller receives didMove(nullptr)");
    expect(!root->view()->superview().expired(), "pop remounts the revealed controller view");
    expect(
        !navigationController->popViewController(false),
        "the root controller cannot be popped"
    );

    navigationController->pushViewController(second, false);
    navigationController->pushViewController(third, false);
    const auto poppedToSecond = navigationController->popToViewController(second, false);
    expect(poppedToSecond.size() == 1 && poppedToSecond[0] == third, "pop-to returns popped items");
    expect(navigationController->topViewController() == second, "pop-to reveals its target");
    expect(third->parent().expired(), "pop-to detaches the removed top controller");

    const auto poppedToRoot = navigationController->popToRootViewController(false);
    expect(
        poppedToRoot.size() == 1 && poppedToRoot[0] == second,
        "pop-to-root returns every controller above the root"
    );
    expect(navigationController->topViewController() == root, "pop-to-root preserves the root");
    expect(
        navigationController->popToViewController(third, false).empty(),
        "pop-to ignores a controller that is not in the stack"
    );

    root->show(second, root);
    expect(
        navigationController->topViewController() == second,
        "show from a contained controller propagates to navigation and pushes"
    );
    navigationController->popToRootViewController(false);

    auto nested = new_shared<RecordingViewController>();
    root->addChild(nested);
    nested->didMoveToParent(root);
    nested->show(third, nested);
    expect(
        navigationController->topViewController() == third,
        "show propagates through an intermediate custom container"
    );
    navigationController->popToRootViewController(false);

    bool emptyStackWasRejected = false;
    try {
        navigationController->setViewControllers({}, false);
    } catch (const std::invalid_argument&) {
        emptyStackWasRejected = true;
    }
    expect(emptyStackWasRejected, "an empty navigation stack is rejected");

    bool nullControllerWasRejected = false;
    try {
        navigationController->setViewControllers({ root, nullptr }, false);
    } catch (const std::invalid_argument&) {
        nullControllerWasRejected = true;
    }
    expect(nullControllerWasRejected, "a null stack entry is rejected");

    auto foreignParent = new_shared<UIViewController>();
    auto foreignChild = new_shared<UIViewController>();
    foreignParent->addChild(foreignChild);
    foreignChild->didMoveToParent(foreignParent);
    bool foreignChildWasRejected = false;
    try {
        navigationController->pushViewController(foreignChild, false);
    } catch (const std::invalid_argument&) {
        foreignChildWasRejected = true;
    }
    expect(foreignChildWasRejected, "a controller owned by another container is rejected");

    auto outerContainer = new_shared<UIViewController>();
    auto cycleRoot = new_shared<UIViewController>();
    auto nestedNavigationController = new_shared<NXNavigationController>(cycleRoot);
    outerContainer->addChild(nestedNavigationController);
    nestedNavigationController->didMoveToParent(outerContainer);
    bool ancestorCycleWasRejected = false;
    try {
        nestedNavigationController->pushViewController(outerContainer, false);
    } catch (const std::invalid_argument&) {
        ancestorCycleWasRejected = true;
    }
    expect(ancestorCycleWasRejected, "a stack entry cannot create a containment cycle");
    expect(
        nestedNavigationController->viewControllers().size() == 1,
        "cycle rejection leaves the stack unchanged"
    );

    auto visibleRoot = new_shared<RecordingViewController>();
    auto visibleNext = new_shared<RecordingViewController>();
    auto visibleNavigationController = new_shared<NXNavigationController>(visibleRoot);
    auto visibleDelegate = new_shared<RecordingDelegate>();
    visibleNavigationController->delegate = visibleDelegate;
    auto window = new_shared<UIWindow>();
    window->setFrame(NXRect(0, 0, 1280, 720));
    visibleNavigationController->view()->setFrame(window->bounds());
    window->addSubview(visibleNavigationController->view());
    visibleNavigationController->viewWillAppear(false);
    visibleNavigationController->viewDidAppear(false);

    visibleNavigationController->pushViewController(visibleNext, false);
    expect(visibleRoot->willDisappearCount == 1, "a visible push begins root disappearance");
    expect(visibleRoot->didDisappearCount == 1, "a visible push completes root disappearance");
    expect(visibleNext->willAppearCount == 1, "a visible push begins destination appearance");
    expect(visibleNext->didAppearCount == 1, "a visible push completes destination appearance");
    expect(
        visibleDelegate->lastWillShow == visibleNext
            && visibleDelegate->lastDidShow == visibleNext,
        "a visible push brackets the transition with delegate callbacks"
    );

    visibleNavigationController->popViewController(false);
    expect(visibleNext->willDisappearCount == 1, "a visible pop begins top disappearance");
    expect(visibleNext->didDisappearCount == 1, "a visible pop completes top disappearance");
    expect(visibleRoot->willAppearCount == 2, "a visible pop begins revealed-root appearance");
    expect(visibleRoot->didAppearCount == 2, "a visible pop completes revealed-root appearance");
    expect(
        visibleDelegate->lastWillShow == visibleRoot
            && visibleDelegate->lastDidShow == visibleRoot,
        "a visible pop brackets the transition with delegate callbacks"
    );

    auto gatedRoot = new_shared<RecordingViewController>();
    auto gatedDestination = new_shared<RecordingViewController>();
    auto gatedNextDestination = new_shared<RecordingViewController>();
    auto gatedDestinationView = new_shared<UIView>();
    auto gatedDestinationControl = new_shared<UIControl>();
    gatedDestinationView->addSubview(gatedDestinationControl);
    gatedDestination->setView(gatedDestinationView);

    auto gatedNavigationController = new_shared<NXNavigationController>(gatedRoot);
    auto gatedWindow = new_shared<UIWindow>();
    gatedWindow->setFrame(NXRect(0, 0, 1280, 720));
    gatedNavigationController->view()->setFrame(gatedWindow->bounds());
    gatedWindow->addSubview(gatedNavigationController->view());
    gatedNavigationController->viewWillAppear(false);
    gatedNavigationController->viewDidAppear(false);

    const auto weakGatedNavigationController =
        std::weak_ptr<NXNavigationController>(gatedNavigationController);
    const std::function<void()> pushFromDestination = [
        weakGatedNavigationController,
        gatedNextDestination
    ]() {
        if (const auto navigationController = weakGatedNavigationController.lock()) {
            navigationController->pushViewController(gatedNextDestination, true);
        }
    };
    gatedDestinationControl->primaryAction = UIAction(
        "Push Deeper",
        pushFromDestination
    );
    NXResponderAction {
        .button = NXActionButton::a,
        .isEnabled = true,
        .action = UIAction("Push Deeper", pushFromDestination),
    }.registerOn(gatedDestinationControl);

    gatedNavigationController->pushViewController(gatedDestination, true);
    expect(
        gatedWindow->focusSystem()->focusedItem().lock() == gatedDestinationControl,
        "an incoming controller receives focus when its transition begins"
    );
    const auto transitioningPrimaryAction = std::find_if(
        gatedNavigationController->defaultActionsWidget()->actions().begin(),
        gatedNavigationController->defaultActionsWidget()->actions().end(),
        [](const NXResponderAction& action) {
            return action.button == NXActionButton::a;
        }
    );
    expect(
        transitioningPrimaryAction
                != gatedNavigationController->defaultActionsWidget()->actions().end()
            && !transitioningPrimaryAction->isEnabled,
        "the incoming controller's legend actions are disabled during its transition"
    );

    gatedDestinationControl->performPrimaryAction();
    for (int step = 0; step < 4 && gatedNavigationController->isTransitioning(); ++step) {
        completePendingAnimations();
    }
    expect(
        gatedNavigationController->viewControllers()
            == std::vector<std::shared_ptr<UIViewController>> {
                gatedRoot,
                gatedDestination,
            },
        "a repeated confirm press cannot enqueue another push during presentation"
    );
    const auto presentedPrimaryAction = std::find_if(
        gatedNavigationController->defaultActionsWidget()->actions().begin(),
        gatedNavigationController->defaultActionsWidget()->actions().end(),
        [](const NXResponderAction& action) {
            return action.button == NXActionButton::a;
        }
    );
    expect(
        presentedPrimaryAction
                != gatedNavigationController->defaultActionsWidget()->actions().end()
            && presentedPrimaryAction->isEnabled,
        "the incoming controller's actions are enabled when presentation completes"
    );
    gatedDestinationControl->performPrimaryAction();
    expect(
        gatedNavigationController->topViewController() == gatedNextDestination,
        "the same action can push after the presentation transition completes"
    );
    for (int step = 0; step < 4 && gatedNavigationController->isTransitioning(); ++step) {
        completePendingAnimations();
    }

    auto transitionRoot = new_shared<RecordingViewController>();
    auto transitionSecond = new_shared<RecordingViewController>();
    auto transitionThird = new_shared<RecordingViewController>();
    auto transitionSecondView = new_shared<RecordingAnimationView>();
    transitionSecond->setView(transitionSecondView);
    auto transitionNavigationController =
        new_shared<NXNavigationController>(transitionRoot);
    auto transitionWindow = new_shared<UIWindow>();
    transitionWindow->setFrame(NXRect(0, 0, 1280, 720));
    transitionNavigationController->view()->setFrame(transitionWindow->bounds());
    transitionWindow->addSubview(transitionNavigationController->view());
    transitionNavigationController->viewWillAppear(false);
    transitionNavigationController->viewDidAppear(false);

    transitionNavigationController->pushViewController(transitionSecond, true);
    expect(
        transitionNavigationController->isTransitioning(),
        "an animated push reports an in-progress navigation transition"
    );
    expect(
        (transitionSecondView->lastAnimationOptions & preferredFramesPerSecond120)
            == preferredFramesPerSecond120,
        "an animated navigation transition requests a 120 fps frame limit"
    );
    transitionNavigationController->pushViewController(transitionThird, true);
    expect(
        transitionNavigationController->viewControllers()
            == std::vector<std::shared_ptr<UIViewController>> {
                transitionRoot,
                transitionSecond,
            },
        "a push requested during an animation waits without changing the public stack"
    );

    for (int step = 0; step < 4 && transitionNavigationController->isTransitioning(); ++step) {
        completePendingAnimations();
    }
    expect(
        !transitionNavigationController->isTransitioning()
            && transitionNavigationController->viewControllers()
                == std::vector<std::shared_ptr<UIViewController>> {
                    transitionRoot,
                    transitionSecond,
                    transitionThird,
                },
        "overlapping animated pushes run serially in request order"
    );

    const auto firstAnimatedPop = transitionNavigationController->popViewController(true);
    const auto secondAnimatedPop = transitionNavigationController->popViewController(true);
    expect(
        firstAnimatedPop == transitionThird && secondAnimatedPop == transitionSecond,
        "overlapping pops return the controllers reserved by their ordered operations"
    );
    expect(
        transitionNavigationController->viewControllers()
            == std::vector<std::shared_ptr<UIViewController>> {
                transitionRoot,
                transitionSecond,
            },
        "a queued pop leaves the public stack at the active transition destination"
    );

    for (int step = 0; step < 4 && transitionNavigationController->isTransitioning(); ++step) {
        completePendingAnimations();
    }
    expect(
        !transitionNavigationController->isTransitioning()
            && transitionNavigationController->viewControllers()
                == std::vector<std::shared_ptr<UIViewController>> { transitionRoot },
        "overlapping animated pops run serially without corrupting containment"
    );
    expect(
        transitionSecond->parent().expired() && transitionThird->parent().expired(),
        "each controller removed by queued pops is detached after its transition"
    );

    auto firstTab = new_shared<RecordingViewController>();
    auto selectedTab = new_shared<RecordingViewController>();
    auto tabBarController = new_shared<NXTabBarController>(
        NXTabBarController::ViewControllerSection { firstTab, selectedTab }
    );
    expect(tabBarController->setSelectedIndex(1), "the routing test selects a non-first tab");

    auto responderNavigationController =
        new_shared<NXNavigationController>(tabBarController);
    auto responderWindow = new_shared<UIWindow>();
    responderWindow->setFrame(NXRect(0, 0, 1280, 720));
    responderWindow->setRootViewController(responderNavigationController);

    auto application = std::make_shared<UIApplication>();
    auto applicationDelegate = std::make_shared<UIApplicationDelegate>();
    applicationDelegate->window = responderWindow;
    application->delegate = applicationDelegate;
    application->keyWindow = responderWindow;
    UIApplication::shared = application;
    responderNavigationController->loadViewIfNeeded();
    responderNavigationController->view()->setFrame(responderWindow->bounds());
    responderWindow->addSubview(responderNavigationController->view());
    responderWindow->updateFocus();

    auto responderDestination = new_shared<RecordingViewController>();
    auto destinationRootView = new_shared<UIView>();
    auto destinationFocusControl = new_shared<UIControl>();
    destinationRootView->addSubview(destinationFocusControl);
    responderDestination->setView(destinationRootView);
    responderNavigationController->pushViewController(responderDestination, false);
    expect(
        responderWindow->focusSystem()->requestFocusUpdate(destinationFocusControl),
        "the destination control accepts focus for responder-chain testing"
    );

    int customPrimaryCount = 0;
    NXResponderAction {
        .button = NXActionButton::a,
        .isEnabled = true,
        .action = UIAction("Custom A", [&customPrimaryCount]() {
            ++customPrimaryCount;
        }),
    }.registerOn(destinationFocusControl);
    responderNavigationController->defaultActionsWidget()->refresh();
    expect(
        responderNavigationController->defaultActionsWidget()->actions().back().action.title()
            == "Custom A",
        "the legend resolves A from the focused responder"
    );

    int overriddenBackCount = 0;
    NXResponderAction destinationBackAction {
        .button = NXActionButton::b,
        .isEnabled = false,
        .action = UIAction("Unavailable", [&overriddenBackCount]() {
            ++overriddenBackCount;
        }),
    };
    destinationBackAction.registerOn(destinationFocusControl);

    const bool eventSubsystemReady = SDL_InitSubSystem(SDL_INIT_EVENTS);
    expect(eventSubsystemReady, "SDL's event subsystem is available for press routing");
    if (eventSubsystemReady) {
        SDL_Event returnDown {};
        returnDown.type = SDL_EVENT_KEY_DOWN;
        returnDown.key.key = SDLK_RETURN;
        returnDown.key.scancode = SDL_SCANCODE_RETURN;
        returnDown.key.down = true;

        SDL_Event returnUp = returnDown;
        returnUp.type = SDL_EVENT_KEY_UP;
        returnUp.key.down = false;

        expect(SDL_PushEvent(&returnDown), "the responder A key-down event is queued");
        expect(SDL_PushEvent(&returnUp), "the responder A key-up event is queued");
        application->handleEventsIfNeeded();
        expect(
            customPrimaryCount == 1,
            "a focused responder action executes once despite focused press dispatch"
        );

        SDL_Event keyDown {};
        keyDown.type = SDL_EVENT_KEY_DOWN;
        keyDown.key.key = SDLK_ESCAPE;
        keyDown.key.scancode = SDL_SCANCODE_ESCAPE;
        keyDown.key.down = true;

        SDL_Event keyUp = keyDown;
        keyUp.type = SDL_EVENT_KEY_UP;
        keyUp.key.down = false;

        expect(SDL_PushEvent(&keyDown), "the Escape key-down event is queued");
        expect(SDL_PushEvent(&keyUp), "the Escape key-up event is queued");
        application->handleEventsIfNeeded();

        expect(
            responderNavigationController->topViewController() == responderDestination
                && overriddenBackCount == 0,
            "a disabled child B action consumes input without falling through to navigation"
        );

        destinationBackAction = NXResponderAction {
            .button = NXActionButton::b,
            .isEnabled = true,
            .action = UIAction("Custom Back", [&overriddenBackCount]() {
                ++overriddenBackCount;
            }),
        };
        destinationBackAction.registerOn(destinationFocusControl);
        expect(SDL_PushEvent(&keyDown), "the custom Back key-down event is queued");
        expect(SDL_PushEvent(&keyUp), "the custom Back key-up event is queued");
        application->handleEventsIfNeeded();
        expect(
            responderNavigationController->topViewController() == responderDestination
                && overriddenBackCount == 1,
            "an enabled child B action runs instead of navigation Back"
        );

        destinationBackAction.unregisterFrom(destinationFocusControl);
        expect(SDL_PushEvent(&keyDown), "the navigation Back key-down event is queued");
        expect(SDL_PushEvent(&keyUp), "the navigation Back key-up event is queued");
        application->handleEventsIfNeeded();
        expect(
            responderNavigationController->topViewController() == tabBarController,
            "an unclaimed menu press reaches the navigation controller's Back action"
        );
        SDL_QuitSubSystem(SDL_INIT_EVENTS);
    }

    responderWindow.reset();
    applicationDelegate.reset();
    application.reset();
    UIApplication::shared.reset();

    if (failures == 0) {
        std::cout << "NXNavigationController tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
