#pragma once

#include <UIGestureRecognizer.h>
#include <Timer.h>
#include <Geometry.h>

namespace NXKit {

class UIWindow;

class UIPanGestureRecognizer: public UIGestureRecognizer {
public:
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    NXPoint translationInView(const std::shared_ptr<UIView>& view);
    void setTranslation(NXPoint translation, const std::shared_ptr<UIView>& inView);

    NXPoint velocityIn(const std::shared_ptr<UIView>& view);

private:
    std::shared_ptr<UITouch> trackingTouch;
    std::weak_ptr<UIWindow> trackingWindow;
    NXPoint initialTouchPoint;

    bool hasProcessedMovementSample = false;
    bool hasVelocitySample = false;
    NXPoint lastProcessedLocation;
    Timer lastProcessedTimestamp;
    NXPoint lastVelocitySampleStartLocation;
    NXPoint lastVelocitySampleEndLocation;
    float lastVelocitySampleDurationSeconds = 0;

    NXPoint velocityIn(const std::shared_ptr<UIView>& view, float timeDiffSeconds);
    void resetVelocityTracking();
    void beginVelocityTracking(const std::shared_ptr<UITouch>& touch);
    void recordVelocitySample();
    NXPoint convertTrackedPointToView(NXPoint point, const std::shared_ptr<UIView>& view) const;
};

}
