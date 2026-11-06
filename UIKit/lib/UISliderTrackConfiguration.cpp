#include <UISliderTrackConfiguration.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace NXKit {
namespace {void finite(NXFloat value){if(!std::isfinite(value))throw std::invalid_argument("Slider configuration values must be finite");}}
UISliderTick::UISliderTick(NXFloat position,std::optional<std::string> title,std::shared_ptr<UIImage> image)
 :_position(position),_title(std::move(title)),_image(std::move(image)){finite(position);}
std::shared_ptr<UISliderTick> UISliderTick::tickWithPosition(NXFloat position,std::optional<std::string> title,std::shared_ptr<UIImage> image) {
 return std::shared_ptr<UISliderTick>(new UISliderTick(position,std::move(title),std::move(image)));
}
std::shared_ptr<UISliderTick> UISliderTick::copy()const{return tickWithPosition(_position,_title,_image);}
std::shared_ptr<UISliderTrackConfiguration> UISliderTrackConfiguration::configurationWithTicks(std::vector<std::shared_ptr<UISliderTick>> ticks) {
 auto result=std::shared_ptr<UISliderTrackConfiguration>(new UISliderTrackConfiguration());
 for(const auto& tick:ticks)if(!tick)throw std::invalid_argument("Slider ticks cannot be null");
 result->_ticks=std::move(ticks);return result;
}
std::shared_ptr<UISliderTrackConfiguration> UISliderTrackConfiguration::configurationWithNumberOfTicks(int count) {
 if(count<2 || count>4096)throw std::invalid_argument("Slider tick count must be between 2 and 4096");
 std::vector<std::shared_ptr<UISliderTick>> ticks;
 for(int i=0;i<count;++i)ticks.push_back(UISliderTick::tickWithPosition(NXFloat(i)/(count-1)));
 return configurationWithTicks(std::move(ticks));
}
void UISliderTrackConfiguration::setNeutralValue(NXFloat value){finite(value);_neutralValue=value;}
void UISliderTrackConfiguration::setMinimumEnabledValue(NXFloat value){finite(value);_minimumEnabledValue=value;}
void UISliderTrackConfiguration::setMaximumEnabledValue(NXFloat value){finite(value);_maximumEnabledValue=value;}
std::shared_ptr<UISliderTrackConfiguration> UISliderTrackConfiguration::copy()const {
 auto result=configurationWithTicks({});*result=*this;result->_ticks.clear();
 for(const auto& tick:_ticks)result->_ticks.push_back(tick->copy());return result;
}
}
