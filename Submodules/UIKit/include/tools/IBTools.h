//
//  IBTools.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 14.03.2023.
//

#pragma once

#include <optional>
#include <string>
#include <yoga/Yoga.h>
#include <UIImage.h>
#include <UIButton.h>
#include <UIEdgeInsets.h>
#include <UIViewContentMode.h>

namespace NXKit {

class UIColor;
class UIFont;
enum class NSTextAlignment;
enum class UIViewEdgeRespects;

std::optional<std::string> valueToString(std::string value);
std::optional<bool> valueToBool(const std::string& value);
std::optional<YGValue> valueToMetric(const std::string& value);
std::optional<UIColor> valueToColor(const std::string& value);
std::optional<float> valueToFloat(const std::string& value);
std::optional<YGJustify> valueToJustify(const std::string& value);
std::optional<YGAlign> valueToAlign(const std::string& value);
std::optional<YGDirection> valueToDirection(const std::string& value);
std::optional<YGFlexDirection> valueToFlexDirection(const std::string& value);
std::optional<YGWrap> valueToWrap(const std::string& value);
//std::optional<std::string> valueToPath(std::string value);
std::shared_ptr<NXData> valueToRes(const std::string& value);
std::optional<std::shared_ptr<UIImage>> valueToImage(const std::string& value);
std::optional<UIViewContentMode> valueToContentMode(const std::string& value);
std::optional<YGPositionType> valueToPositionType(const std::string& value);
std::optional<NSTextAlignment> valueToTextAlignment(const std::string& value);
//std::optional<UIEdgeInsets> valueToEdgeInsets(std::string value);
//std::optional<UIViewEdgeRespects> valueToEdgeRespects(std::string value);
std::optional<UIButtonStyle> valueToButtonStyle(const std::string& value);

}
