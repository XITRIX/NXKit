//
//  IBTools.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 14.03.2023.
//

#include <tools/IBTools.h>
#include <tools/Tools.hpp>
#include <UIColor.h>
#include <UILabel.h>

namespace NXKit {

std::optional<std::string> valueToString(std::string value) {
    return value;
}

std::optional<bool> valueToBool(const std::string& value) {
    if (value == "true") {
        return true;
    }
    if (value == "false") {
        return false;
    }
    return std::nullopt;
}

std::optional<NXFloat> valueToFloat(const std::string& value) {
    try {
        return std::stof(value);
    }
    catch (const std::invalid_argument& exception) {
        return std::nullopt;
    }
}

std::optional<YGValue> valueToMetric(const std::string& value) {
    // Percents
    if (endsWith(value, "%")) {
        // Strip the % and parse the float value
        std::string newFloat = value.substr(0, value.length() - 1);
        try {
            float floatValue = std::stof(newFloat);
            return YGValue { floatValue, YGUnitPercent };
        }
        catch (const std::invalid_argument& exception) {
            return std::nullopt;
        }
    }

    // Points
    try {
        float floatValue = std::stof(value);
        return YGValue { floatValue, YGUnitPoint };
    }
    catch (const std::invalid_argument& exception) {
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<UIColor> valueToColor(const std::string& value) {
    if (startsWith(value, "#")) {
        // Parse the color
        // #RRGGBB format
        if (value.size() == 7) {
            unsigned char r, g, b;
            int result = sscanf(value.c_str(), "#%02hhx%02hhx%02hhx", &r, &g, &b);

            if (result != 3)
                return std::nullopt;

            return UIColor(r, g, b);
        }
            // #RRGGBBAA format
        else if (value.size() == 9) {
            unsigned char r, g, b, a;
            int result = sscanf(value.c_str(), "#%02hhx%02hhx%02hhx%02hhx", &r, &g, &b, &a);

            if (result != 4)
                return std::nullopt;

            return UIColor(r, g, b, a);
        }
    }

    return std::nullopt;
}

std::optional<YGJustify> valueToJustify(const std::string& value) {
    if (value == "flexStart") {
        return YGJustifyFlexStart;
    }
    if (value == "center") {
        return YGJustifyCenter;
    }
    if (value == "flexEnd") {
        return YGJustifyFlexEnd;
    }
    if (value == "spaceBetween") {
        return YGJustifySpaceBetween;
    }
    if (value == "spaceAround") {
        return YGJustifySpaceAround;
    }
    if (value == "spaceEvenly") {
        return YGJustifySpaceEvenly;
    }
    return std::nullopt;
}

std::optional<YGAlign> valueToAlign(const std::string& value) {
    if (value == "auto") {
        return YGAlignAuto;
    }
    if (value == "flexStart") {
        return YGAlignFlexStart;
    }
    if (value == "center") {
        return YGAlignCenter;
    }
    if (value == "flexEnd") {
        return YGAlignFlexEnd;
    }
    if (value == "stretch") {
        return YGAlignStretch;
    }
    if (value == "baseline") {
        return YGAlignBaseline;
    }
    if (value == "spaceBetween") {
        return YGAlignSpaceBetween;
    }
    if (value == "spaceAround") {
        return YGAlignSpaceAround;
    }
    return std::nullopt;
}

std::optional<YGDirection> valueToDirection(const std::string& value) {
    if (value == "inherit") {
        return YGDirectionInherit;
    }
    if (value == "LTR") {
        return YGDirectionLTR;
    }
    if (value == "RTL") {
        return YGDirectionRTL;
    }
    return std::nullopt;
}

std::optional<YGFlexDirection> valueToFlexDirection(const std::string& value) {
    if (value == "column") {
        return YGFlexDirectionColumn;
    }
    if (value == "columnReverse") {
        return YGFlexDirectionColumnReverse;
    }
    if (value == "row") {
        return YGFlexDirectionRow;
    }
    if (value == "rowReverse") {
        return YGFlexDirectionRowReverse;
    }
    return std::nullopt;
}

std::optional<YGWrap> valueToWrap(const std::string& value) {
    if (value == "noWrap") {
        return YGWrapNoWrap;
    }
    if (value == "wrap") {
        return YGWrapWrap;
    }
    if (value == "wrapReverse") {
        return YGWrapWrapReverse;
    }
    return std::nullopt;
}

//std::optional<std::string> valueToPath(std::string value) {
//    if (startsWith(value, "@path/")) {
//        std::string resPath = value.substr(6); // length of "@path/"
//        return Utils::resourcePath + resPath;
//    }
//    return std::nullopt;
//}

std::shared_ptr<NXData> valueToRes(const std::string& value) {
    if (startsWith(value, "@res/")) {
        std::string resPath = value.substr(5); // length of "@res/"
        return NXData::fromPath(resPath);
    }
    return nullptr;
}

std::optional<std::shared_ptr<UIImage>> valueToImage(const std::string& value) {
    if (startsWith(value, "@image/")) {
        std::string resPath = value.substr(7); // length of "@image/"
        auto image = UIImage::fromPath(resPath);
        if (!image) return std::nullopt;
        return image;
    }
    return std::nullopt;
}

std::optional<UIViewContentMode> valueToContentMode(const std::string& value) {
    if (value == "scaleToFill") {
        return UIViewContentMode::scaleToFill;
    }
    if (value == "scaleAspectFit") {
        return UIViewContentMode::scaleAspectFit;
    }
    if (value == "scaleAspectFill") {
        return UIViewContentMode::scaleAspectFill;
    }
    if (value == "redraw") {
        return UIViewContentMode::redraw;
    }
    if (value == "center") {
        return UIViewContentMode::center;
    }
    if (value == "top") {
        return UIViewContentMode::top;
    }
    if (value == "bottom") {
        return UIViewContentMode::bottom;
    }
    if (value == "left") {
        return UIViewContentMode::left;
    }
    if (value == "right") {
        return UIViewContentMode::right;
    }
    if (value == "topLeft") {
        return UIViewContentMode::topLeft;
    }
    if (value == "topRight") {
        return UIViewContentMode::topRight;
    }
    if (value == "bottomLeft") {
        return UIViewContentMode::bottomLeft;
    }
    if (value == "bottomRight") {
        return UIViewContentMode::bottomRight;
    }
    return std::nullopt;
}

std::optional<YGPositionType> valueToPositionType(const std::string& value) {
    if (value == "static") {
        return YGPositionTypeStatic;
    }
    if (value == "relative") {
        return YGPositionTypeRelative;
    }
    if (value == "absolute") {
        return YGPositionTypeAbsolute;
    }
    return std::nullopt;
}

std::optional<NSTextAlignment> valueToTextAlignment(const std::string& value) {
    if (value == "center") {
        return NSTextAlignment::center;
    }
    if (value == "left") {
        return NSTextAlignment::left;
    }
    if (value == "right") {
        return NSTextAlignment::right;
    }
    return std::nullopt;
}

//std::optional<UIViewEdgeRespects> valueToEdgeRespects(std::string value) {
//    if (value == "none") {
//        return UIViewEdgeRespects::none;
//    }
//    if (value == "layoutMargin") {
//        return UIViewEdgeRespects::layoutMargin;
//    }
//    if (value == "safeArea") {
//        return UIViewEdgeRespects::safeArea;
//    }
//    return std::nullopt;
//}

}
