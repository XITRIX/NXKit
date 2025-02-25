add_library(UIKit
        lib/tools/Logger.cpp
        lib/tools/IBTools.cpp
        lib/tools/Fatal.cpp
        lib/tools/Tools.cpp
        lib/CABasicAnimation.cpp
        lib/CABasicAnimationPrototype.cpp
        lib/CABlurLayer.cpp
        lib/CASpringAnimation.cpp
        lib/CASpringAnimationPrototype.cpp
        lib/CAMediaTimingFunction.cpp
        lib/CATextLayer.cpp
        lib/CATransaction.cpp
        lib/ContentsGravityTransformation.cpp
        lib/platforms/SkiaCtx.cpp
        lib/Application.cpp
        lib/DispatchQueue.cpp
        lib/Timer.cpp
        lib/Geometry.cpp
        lib/IndexPath.cpp
        lib/CGImage.cpp
        lib/CADisplayLink.cpp
        lib/CALayer.cpp
        lib/YGLayout.cpp
        lib/UIAction.cpp
        lib/UIApplication.cpp
        lib/UIApplicationDelegateImpl.cpp
        lib/UIApplicationMain.cpp
        lib/UIBlurView.cpp
        lib/UIButton.cpp
        lib/UIColor.cpp
        lib/UIControl.cpp
        lib/UIControlGestureRecognizer.cpp
        lib/UIEdgeInsets.cpp
        lib/UIEvent.cpp
        lib/UIFocus.cpp
        lib/UIFocusAnimationCoordinator.cpp
        lib/UIFocusSystem.cpp
        lib/UIGestureRecognizer.cpp
        lib/UIImage.cpp
        lib/UIImageView.cpp
        lib/UIKey.cpp
        lib/UILabel.cpp
        lib/UINib.cpp
        lib/UIPanGestureRecognizer.cpp
        lib/UIPress.cpp
        lib/UIPressesEvent.cpp
        lib/UIResponder.cpp
        lib/UIScrollView.cpp
        lib/UITapGestureRecognizer.cpp
        lib/UITouch.cpp
        lib/UITraitCollection.cpp
        lib/UITraitEnvironment.cpp
        lib/UIView.cpp
        lib/UIViewAnimationGroup.cpp
        lib/UIViewAnimationOptions.cpp
        lib/UIViewController.cpp
        lib/UIWindow.cpp
        lib/NXAffineTransform.cpp
        lib/NXTransform3D.cpp
        lib/NXData.cpp
        lib/UIScrollViewExtensions/DecelerationTimingParameters.cpp
        lib/UIScrollViewExtensions/RubberBand.cpp
        lib/UIScrollViewExtensions/SpringTimingParameters.cpp
        lib/UIScrollViewExtensions/TimerAnimation.cpp
        lib/SkTools/EventTracingPriv.cpp
        lib/SkTools/SkDebugfTracer.cpp
)

target_include_directories(UIKit PUBLIC
        include
        lib
)


if (APPLE)
    target_sources(UIKit PRIVATE
            lib/platforms/SkiaCtx_sdlBase.cpp
    )
    if (PLATFORM_DESKTOP)
        target_sources(UIKit PRIVATE
                lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    elseif (PLATFORM_IOS)
        target_sources(UIKit PRIVATE
                lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
elseif (PLATFORM_SWITCH)
    target_sources(UIKit PUBLIC
            lib/platforms/SkiaCtx_sdlBase.cpp
            lib/platforms/switch/SkiaCtx_switch.cpp
            lib/platforms/switch/switch_wrapper.c
    )
endif ()