add_library(XITRIXUIKit
        lib/tools/Logger.cpp
        lib/tools/IBTools.cpp
        lib/tools/Fatal.cpp
        lib/tools/Tools.cpp
        lib/CABasicAnimation.cpp
        lib/CABasicAnimationPrototype.cpp
        lib/CABackdropEffectLayer.cpp
        lib/GlassInteraction.hpp
        lib/GlassAppearance.hpp
        lib/GlassBackdrop.hpp
        lib/CABlurLayer.cpp
        lib/CASpringAnimation.cpp
        lib/CASpringAnimationPrototype.cpp
        lib/CAMediaTimingFunction.cpp
        lib/CATextLayer.cpp
        lib/CATransaction.cpp
        lib/ContentsGravityTransformation.cpp
        lib/platforms/SkiaCtx.cpp
        lib/Application.cpp
        lib/BackdropEffect.cpp
        lib/BackdropEffectView.cpp
        lib/LensView.cpp
        lib/LensMaterial.cpp
        lib/LensMaterial.hpp
        lib/LensInteraction.hpp
        include/LensView.h
        lib/GlassContainerView.cpp
        lib/CAGlassContainerLayer.cpp
        lib/CAGlassContainerLayer.hpp
        include/GlassContainerView.h
        include/UIGlassContainerEffect.h
        lib/DispatchQueue.cpp
        lib/Timer.cpp
        lib/Geometry.cpp
        lib/IndexPath.cpp
        lib/CGImage.cpp
        lib/CADisplayLink.cpp
        lib/CALayer.cpp
        lib/LayerCornerGeometry.cpp
        lib/LayerCornerGeometry.hpp
        include/CALayerCornerCurve.h
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
        lib/UIGlassEffect.cpp
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
        lib/UITextView.cpp
        lib/UITouch.cpp
        include/UIUserInterfaceSizeClass.h
        lib/UITraitCollection.cpp
        lib/UITraitEnvironment.cpp
        lib/UIView.cpp
        lib/UIViewAnimationGroup.cpp
        lib/UIViewAnimationOptions.cpp
        lib/UIViewController.cpp
        lib/UIPresentationController.cpp
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

add_library(Xitrix::UIKit ALIAS XITRIXUIKit)

target_include_directories(XITRIXUIKit PUBLIC
        include
        lib
)


if (APPLE)
    target_sources(XITRIXUIKit PRIVATE
            lib/platforms/SkiaCtx_sdlBase.cpp
    )
    if (PLATFORM_DESKTOP)
        target_sources(XITRIXUIKit PRIVATE
                lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    elseif (PLATFORM_IOS)
        target_sources(XITRIXUIKit PRIVATE
                lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
elseif (PLATFORM_SWITCH)
    target_sources(XITRIXUIKit PRIVATE
            lib/platforms/switch/SkiaCtx_switch.cpp
            lib/platforms/switch/switch_wrapper.c
    )
endif ()
