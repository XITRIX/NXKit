#include <UIScrollView.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace NXKit {

class UIScrollViewIndicatorTestHarness {
public:
    static NXRect horizontalFrame(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_horizontalScrollIndicatorLayer->getFrame();
    }

    static NXRect verticalFrame(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_verticalScrollIndicatorLayer->getFrame();
    }

    static bool horizontalIsHidden(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_horizontalScrollIndicatorLayer->isHidden();
    }

    static bool verticalIsHidden(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_verticalScrollIndicatorLayer->isHidden();
    }

    static NXFloat horizontalOpacity(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_horizontalScrollIndicatorLayer->opacity();
    }

    static NXFloat verticalOpacity(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_verticalScrollIndicatorLayer->opacity();
    }

    static NXFloat visibleVerticalOpacity(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_verticalScrollIndicatorLayer
            ->presentationOrSelf()
            ->opacity();
    }

    static std::optional<UIColor> verticalColor(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_verticalScrollIndicatorLayer->backgroundColor();
    }

    static void show(const std::shared_ptr<UIScrollView>& scrollView) {
        scrollView->showScrollIndicators();
    }

    static void hide(const std::shared_ptr<UIScrollView>& scrollView) {
        scrollView->hideScrollIndicators();
    }

    static void setSafeAreaInsets(
        const std::shared_ptr<UIScrollView>& scrollView,
        UIEdgeInsets insets
    ) {
        scrollView->setSafeAreaInsets(insets);
    }
};

} // namespace NXKit

namespace {

int failures = 0;

bool nearlyEqual(NXFloat lhs, NXFloat rhs, NXFloat tolerance = 0.001f) {
    return std::abs(lhs - rhs) <= tolerance;
}

void expect(bool condition, const std::string& message) {
    if (condition) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

template<typename Function>
void expectInvalidArgument(Function function, const std::string& message) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return;
    }
    expect(false, message);
}

} // namespace

int main() {
    auto scrollView = new_shared<UIScrollView>(NXRect(0, 0, 100, 100));
    scrollView->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    scrollView->setContentSize(NXSize(400, 300));
    scrollView->layoutIfNeeded();

    expect(
        scrollView->showsHorizontalScrollIndicator()
            && scrollView->showsVerticalScrollIndicator(),
        "both UIKit scroll-indicator flags default to true"
    );
    expect(
        scrollView->indicatorStyle()
            == UIScrollViewIndicatorStyle::defaultStyle,
        "the indicator style defaults to UIKit's default style"
    );
    expect(
        scrollView->horizontalScrollIndicatorInsets() == UIEdgeInsets::zero
            && scrollView->verticalScrollIndicatorInsets()
                == UIEdgeInsets::zero,
        "the axis-specific indicator insets default to zero"
    );
    expect(
        scrollView->subviews().empty(),
        "private overlay indicators do not appear as scroll content"
    );

    auto firstContentView = new_shared<UIView>();
    auto secondContentView = new_shared<UIView>();
    auto insertedContentView = new_shared<UIView>();
    scrollView->addSubview(firstContentView);
    scrollView->addSubview(secondContentView);
    scrollView->insertSubviewAt(insertedContentView, 1);
    const auto& orderedSubviews = scrollView->subviews();
    const auto orderedSublayers = scrollView->layer()->sublayers();
    expect(
        orderedSubviews.size() == 3
            && orderedSubviews[0] == firstContentView
            && orderedSubviews[1] == insertedContentView
            && orderedSubviews[2] == secondContentView,
        "private overlays preserve indexed UIScrollView subview ordering"
    );
    expect(
        orderedSublayers.size() == 5
            && orderedSublayers[0] == firstContentView->layer()
            && orderedSublayers[1] == insertedContentView->layer()
            && orderedSublayers[2] == secondContentView->layer(),
        "view-backed layers remain before the two private overlays"
    );
    expect(
        !UIScrollViewIndicatorTestHarness::horizontalIsHidden(scrollView)
            && !UIScrollViewIndicatorTestHarness::verticalIsHidden(scrollView),
        "both scrollable axes have indicator geometry"
    );
    expect(
        UIScrollViewIndicatorTestHarness::horizontalOpacity(scrollView) == 0
            && UIScrollViewIndicatorTestHarness::verticalOpacity(scrollView)
                == 0,
        "indicators begin faded out"
    );

    UIScrollViewIndicatorTestHarness::show(scrollView);
    expect(
        UIScrollViewIndicatorTestHarness::horizontalOpacity(scrollView) == 1
            && UIScrollViewIndicatorTestHarness::verticalOpacity(scrollView)
                == 1,
        "showing indicators makes both scrollable axes immediately visible"
    );

    const auto initialHorizontal =
        UIScrollViewIndicatorTestHarness::horizontalFrame(scrollView);
    const auto initialVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(scrollView);
    expect(
        nearlyEqual(initialHorizontal.minX(), 2)
            && nearlyEqual(initialHorizontal.minY(), 95)
            && nearlyEqual(initialHorizontal.width(), 36)
            && nearlyEqual(initialHorizontal.height(), 3),
        "the horizontal thumb uses the viewport track and minimum length"
    );
    expect(
        nearlyEqual(initialVertical.minX(), 95)
            && nearlyEqual(initialVertical.minY(), 2)
            && nearlyEqual(initialVertical.width(), 3)
            && nearlyEqual(initialVertical.height(), 36),
        "the vertical thumb uses the viewport track and minimum length"
    );

    scrollView->setContentOffset(NXPoint(150, 100), false);
    const auto centeredHorizontal =
        UIScrollViewIndicatorTestHarness::horizontalFrame(scrollView);
    const auto centeredVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(scrollView);
    expect(
        nearlyEqual(centeredHorizontal.minX() - scrollView->contentOffset().x,
                    29.5f)
            && nearlyEqual(
                centeredVertical.minY() - scrollView->contentOffset().y,
                29.5f
            ),
        "thumb positions track normalized content progress"
    );
    expect(
        nearlyEqual(centeredHorizontal.minY() - scrollView->contentOffset().y,
                    95)
            && nearlyEqual(
                centeredVertical.minX() - scrollView->contentOffset().x,
                95
            ),
        "indicator layers remain fixed to the viewport while bounds move"
    );

    scrollView->setVerticalScrollIndicatorInsets(
        UIEdgeInsets(10, 0, 20, 7)
    );
    scrollView->setHorizontalScrollIndicatorInsets(
        UIEdgeInsets(0, 5, 9, 11)
    );
    const auto insetHorizontal =
        UIScrollViewIndicatorTestHarness::horizontalFrame(scrollView);
    const auto insetVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(scrollView);
    expect(
        nearlyEqual(insetHorizontal.minY() - scrollView->contentOffset().y, 86)
            && nearlyEqual(
                insetVertical.minX() - scrollView->contentOffset().x,
                88
            ),
        "axis-specific bottom and right insets move their indicators"
    );

    const UIEdgeInsets combinedInsets(4, 6, 8, 10);
    scrollView->setScrollIndicatorInsets(combinedInsets);
    expect(
        scrollView->horizontalScrollIndicatorInsets() == combinedInsets
            && scrollView->verticalScrollIndicatorInsets() == combinedInsets,
        "the combined setter updates both axis-specific inset properties"
    );

    scrollView->setShowsVerticalScrollIndicator(false);
    expect(
        UIScrollViewIndicatorTestHarness::verticalIsHidden(scrollView)
            && !UIScrollViewIndicatorTestHarness::horizontalIsHidden(scrollView),
        "each shows-indicator property gates only its own axis"
    );
    scrollView->setShowsVerticalScrollIndicator(true);

    scrollView->setIndicatorStyle(UIScrollViewIndicatorStyle::black);
    auto indicatorColor =
        UIScrollViewIndicatorTestHarness::verticalColor(scrollView);
    expect(
        indicatorColor.has_value()
            && indicatorColor->r() == 0
            && indicatorColor->g() == 0
            && indicatorColor->b() == 0,
        "the black style applies a dark indicator color"
    );
    scrollView->setIndicatorStyle(UIScrollViewIndicatorStyle::white);
    indicatorColor = UIScrollViewIndicatorTestHarness::verticalColor(scrollView);
    expect(
        indicatorColor.has_value()
            && indicatorColor->r() == 255
            && indicatorColor->g() == 255
            && indicatorColor->b() == 255,
        "the white style applies a light indicator color"
    );

    UIScrollViewIndicatorTestHarness::hide(scrollView);
    expect(
        UIScrollViewIndicatorTestHarness::horizontalOpacity(scrollView) == 0
            && UIScrollViewIndicatorTestHarness::verticalOpacity(scrollView)
                == 0,
        "hiding indicators sets the faded-out model state"
    );
    expect(
        UIScrollViewIndicatorTestHarness::visibleVerticalOpacity(scrollView)
            == 1,
        "hiding indicators fades from the currently visible presentation"
    );
    scrollView->flashScrollIndicators();
    expect(
        UIScrollViewIndicatorTestHarness::horizontalOpacity(scrollView) == 1
            && UIScrollViewIndicatorTestHarness::verticalOpacity(scrollView)
                == 1,
        "flashScrollIndicators makes scrollable indicators visible immediately"
    );

    auto axisSpecific = new_shared<UIScrollView>(NXRect(0, 0, 100, 100));
    axisSpecific->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    axisSpecific->setContentSize(NXSize(300, 300));
    axisSpecific->layoutIfNeeded();
    axisSpecific->withScrollIndicatorsShownForContentOffsetChanges(
        [axisSpecific]() {
            axisSpecific->setContentOffset(NXPoint(50, 0), false);
        }
    );
    expect(
        UIScrollViewIndicatorTestHarness::horizontalOpacity(axisSpecific) == 1
            && UIScrollViewIndicatorTestHarness::verticalOpacity(axisSpecific)
                == 0,
        "the content-offset block shows only axes whose offset changed"
    );

    auto safeAreaScrollView = new_shared<UIScrollView>(
        NXRect(0, 0, 100, 100)
    );
    safeAreaScrollView->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    safeAreaScrollView->setContentSize(NXSize(400, 400));
    safeAreaScrollView->layoutIfNeeded();
    UIScrollViewIndicatorTestHarness::setSafeAreaInsets(
        safeAreaScrollView,
        UIEdgeInsets(12, 18, 24, 30)
    );
    UIScrollViewIndicatorTestHarness::show(safeAreaScrollView);
    const auto safeHorizontal =
        UIScrollViewIndicatorTestHarness::horizontalFrame(safeAreaScrollView);
    const auto safeVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(safeAreaScrollView);
    expect(
        nearlyEqual(safeHorizontal.minX(), 20)
            && nearlyEqual(safeHorizontal.minY(), 71)
            && nearlyEqual(safeVertical.minX(), 65)
            && nearlyEqual(safeVertical.minY(), 14),
        "indicator tracks stay inside every safe-area edge"
    );
    expect(
        safeHorizontal.minX() >= 18
            && safeHorizontal.maxX() <= 70
            && safeHorizontal.minY() >= 12
            && safeHorizontal.maxY() <= 76
            && safeVertical.minX() >= 18
            && safeVertical.maxX() <= 70
            && safeVertical.minY() >= 12
            && safeVertical.maxY() <= 76,
        "both indicator thumbs remain entirely within visible bounds"
    );

    safeAreaScrollView->setVerticalScrollIndicatorInsets(
        UIEdgeInsets(20, 0, 0, 35)
    );
    safeAreaScrollView->setHorizontalScrollIndicatorInsets(
        UIEdgeInsets(0, 25, 30, 0)
    );
    const auto explicitlyInsetHorizontal =
        UIScrollViewIndicatorTestHarness::horizontalFrame(safeAreaScrollView);
    const auto explicitlyInsetVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(safeAreaScrollView);
    expect(
        nearlyEqual(explicitlyInsetHorizontal.minX(), 27)
            && nearlyEqual(explicitlyInsetHorizontal.minY(), 65)
            && nearlyEqual(explicitlyInsetVertical.minX(), 60)
            && nearlyEqual(explicitlyInsetVertical.minY(), 22),
        "larger explicit indicator insets remain effective inside the safe area"
    );

    UIScrollViewIndicatorTestHarness::setSafeAreaInsets(
        safeAreaScrollView,
        UIEdgeInsets(98, 0, 0, 0)
    );
    expect(
        UIScrollViewIndicatorTestHarness::horizontalIsHidden(
            safeAreaScrollView
        )
            && UIScrollViewIndicatorTestHarness::verticalIsHidden(
                safeAreaScrollView
            ),
        "indicators hide instead of crossing a fully occluded visible axis"
    );

    auto verticalOnly = new_shared<UIScrollView>(NXRect(0, 0, 100, 100));
    verticalOnly->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    verticalOnly->setContentSize(NXSize(100, 400));
    verticalOnly->layoutIfNeeded();
    UIScrollViewIndicatorTestHarness::show(verticalOnly);
    expect(
        UIScrollViewIndicatorTestHarness::horizontalIsHidden(verticalOnly)
            && !UIScrollViewIndicatorTestHarness::verticalIsHidden(verticalOnly),
        "an indicator is omitted when its content axis does not scroll"
    );

    auto insetScrollable = new_shared<UIScrollView>(NXRect(0, 0, 100, 100));
    insetScrollable->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    insetScrollable->setContentInset(UIEdgeInsets(20, 20, 20, 20));
    insetScrollable->setContentSize(NXSize(80, 80));
    insetScrollable->layoutIfNeeded();
    UIScrollViewIndicatorTestHarness::show(insetScrollable);
    const auto insetContentVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(insetScrollable);
    expect(
        !UIScrollViewIndicatorTestHarness::horizontalIsHidden(insetScrollable)
            && !UIScrollViewIndicatorTestHarness::verticalIsHidden(
                insetScrollable
            )
            && nearlyEqual(insetContentVertical.height(), 75.8333f),
        "content insets contribute to scrollability and thumb proportion"
    );

    verticalOnly->setContentOffset(NXPoint(0, -20), false);
    const auto compressedVertical =
        UIScrollViewIndicatorTestHarness::verticalFrame(verticalOnly);
    expect(
        nearlyEqual(
            compressedVertical.minY() - verticalOnly->contentOffset().y,
            2
        )
            && nearlyEqual(compressedVertical.height(), 16),
        "leading overscroll pins and compresses the indicator thumb"
    );

    expectInvalidArgument(
        [scrollView]() {
            scrollView->setIndicatorStyle(
                static_cast<UIScrollViewIndicatorStyle>(999)
            );
        },
        "invalid indicator styles are rejected at the public boundary"
    );
    expectInvalidArgument(
        [scrollView]() {
            scrollView->setVerticalScrollIndicatorInsets(UIEdgeInsets(
                std::numeric_limits<NXFloat>::infinity(),
                0,
                0,
                0
            ));
        },
        "non-finite indicator insets are rejected at the public boundary"
    );
    expectInvalidArgument(
        [scrollView]() {
            scrollView->withScrollIndicatorsShownForContentOffsetChanges({});
        },
        "an empty content-offset changes block is rejected"
    );

    if (failures == 0) {
        std::cout << "UIScrollView indicator tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
