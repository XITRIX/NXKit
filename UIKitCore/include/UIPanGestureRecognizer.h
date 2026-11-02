#pragma once
#include <UIGestureRecognizer.h>
#include <Timer.h>
#include <Geometry.h>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace NXKit {
class UIWindow;
class UIPanGestureRecognizer: public UIGestureRecognizer {
public:
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    [[nodiscard]] std::size_t minimumNumberOfTouches() const { return _minimumTouches; }
    void setMinimumNumberOfTouches(std::size_t count);
    [[nodiscard]] std::size_t maximumNumberOfTouches() const { return _maximumTouches; }
    void setMaximumNumberOfTouches(std::size_t count);
    [[nodiscard]] std::size_t numberOfTouches() const override;
    NXPoint translationInView(const std::shared_ptr<UIView>& view);
    void setTranslation(NXPoint translation, const std::shared_ptr<UIView>& inView);
    NXPoint velocityIn(const std::shared_ptr<UIView>& view);
protected:
    void reset() override;
private:
    friend class UIScrollView;
    // Scroll views reject travel along a non-scrollable axis before this pan
    // can prevent an enclosing orthogonal scroll view from recognizing.
    std::function<bool(NXPoint)> _shouldBegin;
    std::vector<std::shared_ptr<UITouch>> _touches;
    std::weak_ptr<UIWindow> _window;
    std::size_t _minimumTouches=1;
    std::size_t _maximumTouches=std::numeric_limits<std::uint32_t>::max();
    NXPoint _translation, _recognitionTravel, _centroid, _velocity, _previousVelocity;
    Timer _sampleTime;
    bool _hasSample=false, _hasVelocity=false;
    NXPoint centroid() const;
    Timer timestamp(const std::vector<std::shared_ptr<UITouch>>& changed) const;
    NXPoint convertVector(NXPoint value,const std::shared_ptr<UIView>& view,bool toWindow=false) const;
    void rebase(const std::vector<std::shared_ptr<UITouch>>& changed);
};
}
