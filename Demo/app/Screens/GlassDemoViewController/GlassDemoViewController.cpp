#include <Screens/GlassDemoViewController/GlassDemoViewController.hpp>

#include <UITouch.h>

#include <algorithm>
#include <cmath>
#include <string>

using namespace NXKit;

namespace {

constexpr NXFloat glassWidth = 120;
constexpr NXFloat glassHeight = 60;
constexpr NXFloat regularGlassWidth = 300;
constexpr NXFloat regularGlassHeight = 160;
constexpr NXFloat minimumRegularGlassWidth = 120;
constexpr NXFloat minimumRegularGlassHeight = 64;
constexpr NXFloat maximumRegularGlassWidth = 450;
constexpr NXFloat maximumRegularGlassHeight = 240;
constexpr NXFloat sliderHorizontalMargin = 24;
constexpr NXFloat sliderBottomMargin = 20;
constexpr NXFloat sliderHeight = 96;
constexpr NXFloat glassSliderSpacing = 16;

NXFloat interpolate(NXFloat lower, NXFloat upper, NXFloat progress) {
    return lower + (upper - lower) * progress;
}

class GlassSliderGestureRecognizer final : public UIGestureRecognizer {
public:
    [[nodiscard]] NXPoint location() const { return _location; }

    void touchesBegan(
        std::vector<std::shared_ptr<UITouch>> touches,
        std::shared_ptr<UIEvent> event
    ) override {
        UIGestureRecognizer::touchesBegan(touches, event);
        if (_trackingTouch || touches.empty()) return;
        _trackingTouch = touches.front();
        updateLocation();
        setState(UIGestureRecognizerState::began);
    }

    void touchesMoved(
        std::vector<std::shared_ptr<UITouch>> touches,
        std::shared_ptr<UIEvent> event
    ) override {
        UIGestureRecognizer::touchesMoved(touches, event);
        if (!_trackingTouch
            || touches.empty()
            || touches.front() != _trackingTouch)
        {
            return;
        }

        updateLocation();
        if (_state != UIGestureRecognizerState::changed) {
            setState(UIGestureRecognizerState::changed);
        } else {
            onStateChanged(shared_from_this());
        }
    }

    void touchesEnded(
        std::vector<std::shared_ptr<UITouch>> touches,
        std::shared_ptr<UIEvent> event
    ) override {
        UIGestureRecognizer::touchesEnded(touches, event);
        if (!_trackingTouch
            || touches.empty()
            || touches.front() != _trackingTouch)
        {
            return;
        }

        updateLocation();
        setState(UIGestureRecognizerState::ended);
        _trackingTouch.reset();
    }

    void touchesCancelled(
        std::vector<std::shared_ptr<UITouch>> touches,
        std::shared_ptr<UIEvent> event
    ) override {
        UIGestureRecognizer::touchesCancelled(touches, event);
        if (!_trackingTouch) return;
        setState(UIGestureRecognizerState::cancelled);
        _trackingTouch.reset();
    }

protected:
    void reset() override { _trackingTouch.reset(); }

private:
    void updateLocation() {
        if (!_trackingTouch) return;
        _location = _trackingTouch->locationIn(view().lock());
    }

    std::shared_ptr<UITouch> _trackingTouch;
    NXPoint _location;
};

}

GlassDemoViewController::GlassDemoViewController() {
    setTitle("Glass");
}

void GlassDemoViewController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::black);
    rootView->setClipsToBounds(true);

    _backgroundImageView = new_shared<UIImageView>(
        UIImage::fromRes("img/glass_demo_background.png")
    );
    _backgroundImageView->setContentMode(UIViewContentMode::scaleAspectFill);
    rootView->addSubview(_backgroundImageView);

    _clearGlassView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::clear)
    );
    _clearGlassView->setFrame(NXRect(0, 0, glassWidth, glassHeight));
    _clearGlassView->layer()->setCornerRadius(glassHeight * 0.5f);
    rootView->addSubview(_clearGlassView);
    makeDraggable(_clearGlassView);

    _regularGlassView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::regular)
    );
    _regularGlassView->setFrame(NXRect(
        0,
        0,
        regularGlassWidth,
        regularGlassHeight
    ));
    _regularGlassView->layer()->setCornerRadius(52);
    rootView->addSubview(_regularGlassView);
    makeDraggable(_regularGlassView);

    _sizeSliderView = new_shared<UIView>();
    _sizeSliderView->setBackgroundColor(
        UIColor::systemBackground.withAlphaComponent(0.88f)
    );
    _sizeSliderView->layer()->setCornerRadius(22);
    _sizeSliderView->layer()->setBorderColor(
        UIColor::separator.withAlphaComponent(0.7f)
    );
    _sizeSliderView->layer()->setBorderWidth(1);
    _sizeSliderView->layer()->setShadowColor(
        UIColor::black.withAlphaComponent(0.22f)
    );
    _sizeSliderView->layer()->setShadowOffset(NXPoint(0, 5));
    _sizeSliderView->layer()->setShadowRadius(12);

    _sizeSliderTitleLabel = new_shared<UILabel>();
    _sizeSliderTitleLabel->setText("Regular glass size");
    _sizeSliderTitleLabel->setTextColor(UIColor::label);
    _sizeSliderTitleLabel->setFontSize(17);
    _sizeSliderTitleLabel->setFontWeight(600);
    _sizeSliderView->addSubview(_sizeSliderTitleLabel);

    _sizeSliderValueLabel = new_shared<UILabel>();
    _sizeSliderValueLabel->setTextColor(UIColor::secondaryLabel);
    _sizeSliderValueLabel->setFontSize(15);
    _sizeSliderValueLabel->setTextAlignment(NSTextAlignment::right);
    _sizeSliderView->addSubview(_sizeSliderValueLabel);

    _sizeSliderTrackView = new_shared<UIView>();
    _sizeSliderTrackView->setBackgroundColor(UIColor::systemFill);
    _sizeSliderTrackView->setClipsToBounds(true);
    _sizeSliderTrackView->layer()->setCornerRadius(4);
    _sizeSliderView->addSubview(_sizeSliderTrackView);

    _sizeSliderFillView = new_shared<UIView>();
    _sizeSliderFillView->setBackgroundColor(UIColor::systemBlue);
    _sizeSliderTrackView->addSubview(_sizeSliderFillView);

    _sizeSliderThumbView = new_shared<UIView>();
    _sizeSliderThumbView->setBackgroundColor(UIColor::white);
    _sizeSliderThumbView->layer()->setCornerRadius(13);
    _sizeSliderThumbView->layer()->setBorderColor(UIColor::systemBlue);
    _sizeSliderThumbView->layer()->setBorderWidth(3);
    _sizeSliderThumbView->layer()->setShadowColor(
        UIColor::black.withAlphaComponent(0.22f)
    );
    _sizeSliderThumbView->layer()->setShadowOffset(NXPoint(0, 2));
    _sizeSliderThumbView->layer()->setShadowRadius(4);
    _sizeSliderView->addSubview(_sizeSliderThumbView);

    auto sliderGesture = new_shared<GlassSliderGestureRecognizer>();
    sliderGesture->onStateChanged = [this](const auto& gesture) {
        const auto slider = std::static_pointer_cast<
            GlassSliderGestureRecognizer
        >(gesture);
        switch (slider->state()) {
            case UIGestureRecognizerState::began:
            case UIGestureRecognizerState::changed:
            case UIGestureRecognizerState::ended: {
                const auto trackFrame = _sizeSliderTrackView->frame();
                if (trackFrame.width() <= 0) return;
                setRegularGlassSizeProgress(
                    (slider->location().x - trackFrame.minX())
                        / trackFrame.width()
                );
                break;
            }
            default:
                break;
        }
    };
    _sizeSliderView->addGestureRecognizer(sliderGesture);
    rootView->addSubview(_sizeSliderView);

    setView(rootView);
}

void GlassDemoViewController::makeDraggable(
    const std::shared_ptr<BackdropEffectView>& glassView
) {
    auto panGesture = new_shared<UIPanGestureRecognizer>();
    panGesture->onStateChanged = [this](const auto& gesture) {
        const auto pan = std::static_pointer_cast<UIPanGestureRecognizer>(gesture);
        const auto glassView = pan->view().lock();
        if (!glassView) return;

        switch (pan->state()) {
            case UIGestureRecognizerState::began:
                _dragOrigin = glassView->frame().origin;
                break;
            case UIGestureRecognizerState::changed: {
                auto frame = glassView->frame();
                frame.origin = _dragOrigin + pan->translationInView(view());
                glassView->setFrame(frame);
                clampGlassToBounds(
                    std::static_pointer_cast<BackdropEffectView>(glassView)
                );
                break;
            }
            default:
                break;
        }
    };
    glassView->addGestureRecognizer(panGesture);
}

void GlassDemoViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    const auto rootBounds = view()->bounds();
    const auto safeArea = view()->safeAreaInsets();
    _backgroundImageView->setFrame(rootBounds);

    const auto panelWidth = std::max<NXFloat>(
        0,
        rootBounds.width()
            - safeArea.left
            - safeArea.right
            - sliderHorizontalMargin * 2
    );
    _sizeSliderView->setFrame(NXRect(
        rootBounds.minX() + safeArea.left + sliderHorizontalMargin,
        rootBounds.maxY() - safeArea.bottom - sliderBottomMargin - sliderHeight,
        panelWidth,
        sliderHeight
    ));
    setRegularGlassSizeProgress(_regularGlassSizeProgress);

    if (!_hasPositionedGlass) {
        const auto glassTop = rootBounds.minY() + safeArea.top;
        const auto glassBottom = std::max(
            glassTop,
            _sizeSliderView->frame().minY() - glassSliderSpacing
        );
        const auto glassCenterY = (glassTop + glassBottom) * 0.5f;
        const auto usableWidth = std::max<NXFloat>(
            0,
            rootBounds.width() - safeArea.left - safeArea.right
        );
        _clearGlassView->setCenter(NXPoint(
            rootBounds.minX() + safeArea.left + usableWidth / 3.0f,
            glassCenterY
        ));
        _regularGlassView->setCenter(NXPoint(
            rootBounds.minX() + safeArea.left + usableWidth * 2.0f / 3.0f,
            glassCenterY
        ));
        _hasPositionedGlass = true;
    }
    clampGlassToBounds(_clearGlassView);
    clampGlassToBounds(_regularGlassView);
}

void GlassDemoViewController::layoutSizeSlider() {
    if (!_sizeSliderView) return;

    const auto bounds = _sizeSliderView->bounds();
    constexpr NXFloat horizontalInset = 20;
    constexpr NXFloat labelTop = 13;
    constexpr NXFloat labelHeight = 24;
    constexpr NXFloat valueWidth = 110;
    constexpr NXFloat trackHeight = 8;
    constexpr NXFloat trackTop = 60;
    constexpr NXFloat thumbSize = 26;
    const auto contentWidth = std::max<NXFloat>(
        0,
        bounds.width() - horizontalInset * 2
    );

    _sizeSliderTitleLabel->setFrame(NXRect(
        horizontalInset,
        labelTop,
        std::max<NXFloat>(0, contentWidth - valueWidth),
        labelHeight
    ));
    _sizeSliderValueLabel->setFrame(NXRect(
        bounds.width() - horizontalInset - valueWidth,
        labelTop,
        valueWidth,
        labelHeight
    ));
    _sizeSliderTrackView->setFrame(NXRect(
        horizontalInset,
        trackTop,
        contentWidth,
        trackHeight
    ));
    _sizeSliderFillView->setFrame(NXRect(
        0,
        0,
        contentWidth * _regularGlassSizeProgress,
        trackHeight
    ));
    _sizeSliderThumbView->setFrame(NXRect(
        horizontalInset
            + contentWidth * _regularGlassSizeProgress
            - thumbSize * 0.5f,
        trackTop + trackHeight * 0.5f - thumbSize * 0.5f,
        thumbSize,
        thumbSize
    ));
}

void GlassDemoViewController::setRegularGlassSizeProgress(NXFloat progress) {
    _regularGlassSizeProgress = std::clamp(progress, 0.0f, 1.0f);
    if (!_regularGlassView) return;

    auto size = NXSize(
        interpolate(
            minimumRegularGlassWidth,
            maximumRegularGlassWidth,
            _regularGlassSizeProgress
        ),
        interpolate(
            minimumRegularGlassHeight,
            maximumRegularGlassHeight,
            _regularGlassSizeProgress
        )
    );
    if (viewIsLoaded()) {
        const auto rootBounds = view()->bounds();
        const auto safeArea = view()->safeAreaInsets();
        const auto availableWidth = std::max<NXFloat>(
            minimumRegularGlassWidth,
            rootBounds.width()
                - safeArea.left
                - safeArea.right
                - sliderHorizontalMargin
        );
        size.width = std::min(size.width, availableWidth);
    }

    const auto center = _regularGlassView->center();
    _regularGlassView->setFrame(NXRect(0, 0, size.width, size.height));
    _regularGlassView->setCenter(center);
    _regularGlassView->layer()->setCornerRadius(
        std::min<NXFloat>(52, size.height * 0.42f)
    );
    _sizeSliderValueLabel->setText(
        std::to_string(static_cast<int>(std::lround(size.width)))
            + " x "
            + std::to_string(static_cast<int>(std::lround(size.height)))
    );
    layoutSizeSlider();
    clampGlassToBounds(_regularGlassView);
}

void GlassDemoViewController::clampGlassToBounds(
    const std::shared_ptr<BackdropEffectView>& glassView
) {
    if (!glassView || !viewIsLoaded()) return;

    const auto bounds = view()->bounds();
    const auto safeArea = view()->safeAreaInsets();
    auto frame = glassView->frame();
    const auto minimumX = bounds.minX() + safeArea.left;
    const auto minimumY = bounds.minY() + safeArea.top;
    const auto maximumRight = bounds.maxX() - safeArea.right;
    const auto maximumBottom = _sizeSliderView
        ? std::min(
            bounds.maxY() - safeArea.bottom,
            _sizeSliderView->frame().minY() - glassSliderSpacing
        )
        : bounds.maxY() - safeArea.bottom;
    const auto maximumX = std::max(minimumX, maximumRight - frame.width());
    const auto maximumY = std::max(minimumY, maximumBottom - frame.height());
    frame.origin.x = std::clamp(frame.origin.x, minimumX, maximumX);
    frame.origin.y = std::clamp(frame.origin.y, minimumY, maximumY);
    glassView->setFrame(frame);
}
