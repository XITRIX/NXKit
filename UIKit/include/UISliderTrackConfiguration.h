#pragma once
#include <Geometry.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace NXKit {
class UIImage;
class UISliderTick {
public:
    static std::shared_ptr<UISliderTick> tickWithPosition(NXFloat position,
        std::optional<std::string> title = std::nullopt, std::shared_ptr<UIImage> image = nullptr);
    NXFloat position() const { return _position; }
    const std::optional<std::string>& title() const { return _title; }
    void setTitle(std::optional<std::string> title) { _title=std::move(title); }
    std::shared_ptr<UIImage> image() const { return _image; }
    void setImage(std::shared_ptr<UIImage> image) { _image=std::move(image); }
    std::shared_ptr<UISliderTick> copy() const;
private:
    UISliderTick(NXFloat position,std::optional<std::string> title,std::shared_ptr<UIImage> image);
    NXFloat _position;
    std::optional<std::string> _title;
    std::shared_ptr<UIImage> _image;
};
class UISliderTrackConfiguration {
public:
    static std::shared_ptr<UISliderTrackConfiguration> configurationWithTicks(std::vector<std::shared_ptr<UISliderTick>> ticks);
    static std::shared_ptr<UISliderTrackConfiguration> configurationWithNumberOfTicks(int count);
    bool allowsTickValuesOnly() const { return _allowsTickValuesOnly; }
    void setAllowsTickValuesOnly(bool value) { _allowsTickValuesOnly=value; }
    NXFloat neutralValue() const { return _neutralValue; }
    void setNeutralValue(NXFloat value);
    NXFloat minimumEnabledValue() const { return _minimumEnabledValue; }
    void setMinimumEnabledValue(NXFloat value);
    NXFloat maximumEnabledValue() const { return _maximumEnabledValue; }
    void setMaximumEnabledValue(NXFloat value);
    std::vector<std::shared_ptr<UISliderTick>> ticks() const { return _ticks; }
    std::shared_ptr<UISliderTrackConfiguration> copy() const;
private:
    UISliderTrackConfiguration() = default;
    bool _allowsTickValuesOnly=true;
    NXFloat _neutralValue=0, _minimumEnabledValue=0, _maximumEnabledValue=1;
    std::vector<std::shared_ptr<UISliderTick>> _ticks;
};
}
