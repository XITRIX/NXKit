#include <NXTabBarController.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

class RecordingViewController final : public UIViewController {
public:
    int addedToParentCount = 0;
    int removedFromParentCount = 0;

    void didMoveToParent(std::shared_ptr<UIViewController> parent) override {
        UIViewController::didMoveToParent(parent);
        if (parent) {
            ++addedToParentCount;
        } else {
            ++removedFromParentCount;
        }
    }
};

class RecordingDelegate final : public NXTabBarControllerDelegate {
public:
    bool allowsSelection = true;
    int shouldSelectCount = 0;
    int didSelectCount = 0;

    bool tabBarControllerShouldSelect(
        const std::shared_ptr<NXTabBarController>&,
        const std::shared_ptr<UIViewController>&
    ) override {
        ++shouldSelectCount;
        return allowsSelection;
    }

    void tabBarControllerDidSelect(
        const std::shared_ptr<NXTabBarController>&,
        const std::shared_ptr<UIViewController>&
    ) override {
        ++didSelectCount;
    }
};

} // namespace

int main() {
    auto first = new_shared<RecordingViewController>();
    auto second = new_shared<RecordingViewController>();
    auto third = new_shared<RecordingViewController>();
    first->setTitle("First");
    second->setTitle("Second");
    third->setTitle("Third");

    NXTabBarController::ViewControllerSections sections = {
        {},
        { first, second }
    };
    auto controller = new_shared<NXTabBarController>(sections);

    expect(controller->selectedIndex() == 0, "the first non-empty tab is selected initially");
    expect(
        controller->selectedIndexPath() == std::optional<IndexPath>(IndexPath(0, 1)),
        "the initial section and item are preserved"
    );
    expect(controller->selectedViewController() == first, "the selected controller is queryable");
    expect(controller->viewControllers().size() == 2, "the flat controller list skips empty sections");

    controller->loadViewIfNeeded();
    expect(first->parent().lock() == controller, "the selected controller is contained after loading");
    expect(second->parent().lock() == controller, "unselected controllers remain children of the tab container");
    expect(first->addedToParentCount == 1, "the selected controller receives didMoveToParent");
    expect(second->addedToParentCount == 1, "an unselected controller receives didMoveToParent");
    expect(
        controller->tabBar()->selectedIndexPath() == controller->selectedIndexPath(),
        "the visual tab selection matches the controller selection"
    );

    auto delegate = new_shared<RecordingDelegate>();
    controller->delegate = delegate;
    int publicSelectionCount = 0;
    controller->tabBar()->selectionDidChange = [&publicSelectionCount](const IndexPath&) {
        ++publicSelectionCount;
    };

    delegate->allowsSelection = false;
    expect(
        !controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "a delegate can reject user-driven selection"
    );
    expect(controller->selectedViewController() == first, "rejected selection preserves content");
    expect(publicSelectionCount == 0, "rejected selection emits no public change callback");

    delegate->allowsSelection = true;
    expect(
        controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "an allowed user-driven selection succeeds"
    );
    expect(controller->selectedViewController() == second, "allowed selection changes content");
    expect(first->parent().lock() == controller, "the previous tab remains a child of the container");
    expect(second->parent().lock() == controller, "the new selected controller is contained");
    expect(first->removedFromParentCount == 0, "selection does not change controller containment");
    expect(delegate->shouldSelectCount == 2, "the delegate is consulted for both user selections");
    expect(delegate->didSelectCount == 1, "the delegate is notified only for accepted selection");
    expect(publicSelectionCount == 1, "accepted selection emits the public change callback");

    expect(
        controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "reselecting the active tab is still a user selection"
    );
    expect(delegate->shouldSelectCount == 3, "reselection consults the delegate");
    expect(delegate->didSelectCount == 2, "reselection notifies the delegate");
    expect(publicSelectionCount == 2, "reselection emits the public callback");

    expect(!controller->setSelectedIndex(9), "an invalid flat index is rejected");
    expect(
        !controller->setSelectedIndexPath(IndexPath(-1, 0)),
        "a negative index path is rejected"
    );
    expect(controller->selectedViewController() == second, "invalid selection does not mutate state");

    controller->setViewControllerSections({ { second, third } });
    expect(controller->selectedViewController() == second, "reconfiguration preserves a surviving selection");
    expect(controller->selectedIndex() == 0, "the preserved selection receives its new flat index");
    expect(first->parent().expired(), "removed tabs are detached from the container");
    expect(first->removedFromParentCount == 1, "removed tabs receive didMoveToParent(nullptr)");
    expect(third->parent().lock() == controller, "new tabs are attached to the container");

    bool duplicateWasRejected = false;
    try {
        controller->setViewControllerSections({ { first, first } });
    } catch (const std::invalid_argument&) {
        duplicateWasRejected = true;
    }
    expect(duplicateWasRejected, "duplicate controllers are rejected visibly");

    bool nullWasRejected = false;
    try {
        controller->setViewControllerSections({ { nullptr } });
    } catch (const std::invalid_argument&) {
        nullWasRejected = true;
    }
    expect(nullWasRejected, "null controllers are rejected visibly");

    bool invalidWidthWasRejected = false;
    try {
        controller->setTabBarWidth(0);
    } catch (const std::invalid_argument&) {
        invalidWidthWasRejected = true;
    }
    expect(invalidWidthWasRejected, "non-positive tab bar widths are rejected visibly");

    controller->setViewControllerSections({});
    expect(!controller->selectedIndex().has_value(), "empty content has no selected index");
    expect(!controller->selectedIndexPath().has_value(), "empty content has no selected index path");
    expect(!controller->selectedViewController(), "empty content has no selected controller");
    expect(second->parent().expired(), "clearing tabs detaches the selected controller");
    expect(third->parent().expired(), "clearing tabs detaches unselected controllers");
    expect(!controller->tabBar()->selectedIndexPath(), "clearing tabs clears the visual selection");

    if (failures == 0) {
        std::cout << "NXTabBarController tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
