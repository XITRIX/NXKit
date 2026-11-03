#include "IOSWindowGeometry.hpp"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/message.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
struct GeometryState {
    CGSize displayed = CGSizeZero, from = CGSizeZero, target = CGSizeZero;
    UIEdgeInsets insets = UIEdgeInsetsZero, fromInsets = UIEdgeInsetsZero;
    bool transitioning = false;
    std::uint64_t generation = 0;
};
bool usable(CGSize size) {
    return std::isfinite(size.width) && std::isfinite(size.height) && size.width > 0 && size.height > 0;
}
CGSize windowSize(UIWindow* window) {
    const auto size = window.rootViewController.view.bounds.size;
    return usable(size) ? size : window.bounds.size;
}
}

// Containment receives UIKit's actual transition coordinator without replacing
// SDL's root controller, method swizzling, notifications or guessed durations.
@interface NXWindowGeometryObserver : UIViewController {
@public
    std::shared_ptr<GeometryState> state;
}
@property(nonatomic, weak) UIWindow* hostWindow;
@end

@implementation NXWindowGeometryObserver
- (void)loadView {
    UIView* probe = [[UIView alloc] initWithFrame:CGRectZero];
    probe.userInteractionEnabled = NO;
    probe.opaque = NO;
    probe.backgroundColor = UIColor.clearColor;
    self.view = probe;
}
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    if (!state || !usable(size)) return;
    if (!coordinator.isAnimated) {
        ++state->generation;
        state->transitioning = false;
        [self.view.layer removeAllAnimations];
        return;
    }
    auto sharedState = state;
    const auto visible = self.view.layer.presentationLayer;
    const auto from = state->transitioning && visible && usable(visible.bounds.size)
        ? visible.bounds.size : state->displayed;
    if (!usable(from) || CGSizeEqualToSize(from, size)) {
        ++state->generation;
        state->transitioning = false;
        [self.view.layer removeAllAnimations];
        return;
    }
    state->from = from;
    state->target = size;
    state->fromInsets = state->insets;
    state->transitioning = true;
    const auto generation = ++state->generation;
    [UIView performWithoutAnimation:^{
        [self.view.layer removeAllAnimations];
        self.view.bounds = CGRectMake(0, 0, from.width, from.height);
    }];
    const BOOL registered = [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        self.view.bounds = CGRectMake(0, 0, size.width, size.height);
    } completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        if (sharedState->generation != generation) return;
        // On cancellation UIKit's actual model bounds determine the endpoint.
        sharedState->transitioning = false;
        sharedState->displayed = windowSize(self.hostWindow);
        sharedState->insets = self.hostWindow.safeAreaInsets;
    }];
    if (!registered && state->generation == generation) state->transitioning = false;
}
@end

namespace NXKit::Internal {
struct IOSWindowGeometry::Impl {
    __weak UIWindow* window;
    NXWindowGeometryObserver* observer;
    UIView* cornerProbe;
    std::shared_ptr<GeometryState> state = std::make_shared<GeometryState>();
};

IOSWindowGeometry::IOSWindowGeometry(void* nativeWindow): _impl(std::make_unique<Impl>()) {
    auto window = (__bridge UIWindow*)nativeWindow;
    _impl->window = window;
    _impl->state->displayed = windowSize(window);
    _impl->state->insets = window.safeAreaInsets;
    auto root = window.rootViewController;
    if (!root) return;
    auto observer = [[NXWindowGeometryObserver alloc] init];
    observer->state = _impl->state;
    observer.hostWindow = window;
    [root addChildViewController:observer];
    // The transparent, non-interactive probe has no application UI. Its
    // bounds animate under UIKit; its frame never controls the Metal view.
    [root.view addSubview:observer.view];
    [observer didMoveToParentViewController:root];
    _impl->observer = observer;
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 260000
    if (@available(iOS 26.0, *)) {
        auto probe = [[UIView alloc] initWithFrame:root.view.bounds];
        probe.backgroundColor = UIColor.clearColor;
        probe.userInteractionEnabled = NO;
        probe.cornerConfiguration = [UICornerConfiguration configurationWithRadius:UICornerRadius.containerConcentricRadius];
        [root.view addSubview:probe];
        _impl->cornerProbe = probe;
    }
#endif
}

IOSWindowGeometry::~IOSWindowGeometry() {
    [_impl->cornerProbe removeFromSuperview];
    auto observer = _impl->observer;
    if (!observer) return;
    observer->state.reset();
    [observer willMoveToParentViewController:nil];
    [observer.view removeFromSuperview];
    [observer removeFromParentViewController];
}

IOSWindowFrame IOSWindowGeometry::sample() {
    auto window = _impl->window;
    auto& state = *_impl->state;
    auto size = windowSize(window);
    auto insets = window.safeAreaInsets;
    if (state.transitioning) {
        auto presentation = _impl->observer.view.layer.presentationLayer;
        size = presentation && usable(presentation.bounds.size) ? presentation.bounds.size : state.from;
        // Sample the native timing curve once for geometry and safe areas.
        const auto dx = state.target.width-state.from.width;
        const auto dy = state.target.height-state.from.height;
        const auto progress = std::clamp(std::abs(dx) >= std::abs(dy) && dx != 0
            ? (size.width-state.from.width)/dx : (dy != 0 ? (size.height-state.from.height)/dy : 1), 0.0, 1.0);
        const auto interpolate = [progress](CGFloat from, CGFloat to) { return from+(to-from)*progress; };
        insets = UIEdgeInsetsMake(interpolate(state.fromInsets.top,insets.top),
            interpolate(state.fromInsets.left,insets.left), interpolate(state.fromInsets.bottom,insets.bottom),
            interpolate(state.fromInsets.right,insets.right));
    }
    state.displayed = size;
    state.insets = insets;
    IOSWindowFrame frame;
    frame.width = NXFloat(size.width); frame.height = NXFloat(size.height);
    frame.top = NXFloat(insets.top); frame.left = NXFloat(insets.left);
    frame.bottom = NXFloat(insets.bottom); frame.right = NXFloat(insets.right);
    const auto scale = window.screen.nativeScale;
    frame.scale = std::isfinite(scale) && scale > 0 ? NXFloat(scale) : 1;
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 260000
    if (@available(iOS 26.0, *)) {
        auto probe = _impl->cornerProbe;
        [UIView performWithoutAnimation:^{ probe.frame = window.rootViewController.view.bounds; }];
        [probe layoutIfNeeded];
        const UIRectCorner corners[] = {UIRectCornerTopLeft, UIRectCornerTopRight, UIRectCornerBottomLeft, UIRectCornerBottomRight};
        for (size_t i=0;i<4;++i) {
            const auto radius = [probe effectiveRadiusForCorner:corners[i]];
            frame.cornerRadii[i] = std::isfinite(radius) ? NXFloat(std::max(0.0,radius)) : 0;
        }
    } else
#endif
    {
        // Explicit pre-iOS-26 compatibility path. Query this window's screen,
        // never UIScreen.mainScreen (external displays may be square).
        const auto selector = NSSelectorFromString(@"_displayCornerRadius");
        auto screen = window.screen;
        if ([screen respondsToSelector:selector]) {
            const auto radius = reinterpret_cast<CGFloat(*)(id,SEL)>(objc_msgSend)(screen,selector);
            if (std::isfinite(radius) && radius>0) frame.cornerRadii.fill(NXFloat(radius));
        }
    }
    frame.transitioning = state.transitioning;
    return frame;
}
}
