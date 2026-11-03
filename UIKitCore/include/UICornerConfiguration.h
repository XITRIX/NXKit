#pragma once
#include <UICornerRadius.h>
#include <array>
namespace NXKit {
/// Immutable value configuration. Corner order is top-left, top-right,
/// bottom-left, bottom-right; omitted radii describe square corners.
class UICornerConfiguration {
public:
    static UICornerConfiguration corners(UICornerRadius radius) { return {{radius,radius,radius,radius}}; }
    static UICornerConfiguration corners(std::optional<UICornerRadius> topLeftRadius,
        std::optional<UICornerRadius> topRightRadius, std::optional<UICornerRadius> bottomLeftRadius,
        std::optional<UICornerRadius> bottomRightRadius) {
        return {{topLeftRadius,topRightRadius,bottomLeftRadius,bottomRightRadius}};
    }
    static UICornerConfiguration capsule(std::optional<NXFloat> maximumRadius = std::nullopt) {
        if (maximumRadius) (void)UICornerRadius::fixed(*maximumRadius);
        auto result = corners(UICornerRadius::fixed(0)); result._capsule = true;
        result._maximumRadius = maximumRadius; return result;
    }
    static UICornerConfiguration uniformCorners(UICornerRadius radius) {
        auto result = corners(radius); result._groups = {15,0}; return result;
    }
    static UICornerConfiguration uniformTopRadius(UICornerRadius radius,
        std::optional<UICornerRadius> bottomLeftRadius = std::nullopt,
        std::optional<UICornerRadius> bottomRightRadius = std::nullopt) {
        auto result = corners(radius,radius,bottomLeftRadius,bottomRightRadius); result._groups = {3,0}; return result;
    }
    static UICornerConfiguration uniformBottomRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topLeftRadius = std::nullopt,
        std::optional<UICornerRadius> topRightRadius = std::nullopt) {
        auto result = corners(topLeftRadius,topRightRadius,radius,radius); result._groups = {12,0}; return result;
    }
    static UICornerConfiguration uniformLeftRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topRightRadius = std::nullopt,
        std::optional<UICornerRadius> bottomRightRadius = std::nullopt) {
        auto result = corners(radius,topRightRadius,radius,bottomRightRadius); result._groups = {5,0}; return result;
    }
    static UICornerConfiguration uniformRightRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topLeftRadius = std::nullopt,
        std::optional<UICornerRadius> bottomLeftRadius = std::nullopt) {
        auto result = corners(topLeftRadius,radius,bottomLeftRadius,radius); result._groups = {10,0}; return result;
    }
    // Objective-C selector stems disambiguate Swift's two uniformEdges overloads.
    static UICornerConfiguration configurationWithUniformTopRadius(UICornerRadius top, UICornerRadius bottom) {
        auto result = corners(top,top,bottom,bottom); result._groups = {3,12}; return result;
    }
    static UICornerConfiguration configurationWithUniformLeftRadius(UICornerRadius left, UICornerRadius right) {
        auto result = corners(left,right,left,right); result._groups = {5,10}; return result;
    }
    static UICornerConfiguration configurationWithRadius(UICornerRadius radius) { return corners(radius); }
    static UICornerConfiguration configurationWithTopLeftRadius(std::optional<UICornerRadius> tl,
        std::optional<UICornerRadius> tr, std::optional<UICornerRadius> bl, std::optional<UICornerRadius> br) { return corners(tl,tr,bl,br); }
    static UICornerConfiguration capsuleConfiguration() { return capsule(); }
    static UICornerConfiguration capsuleConfigurationWithMaximumRadius(NXFloat maximum) { return capsule(maximum); }
    static UICornerConfiguration configurationWithUniformRadius(UICornerRadius radius) { return uniformCorners(radius); }
    static UICornerConfiguration configurationWithUniformTopRadius(UICornerRadius radius,
        std::optional<UICornerRadius> bottomLeft, std::optional<UICornerRadius> bottomRight) { return uniformTopRadius(radius,bottomLeft,bottomRight); }
    static UICornerConfiguration configurationWithUniformBottomRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topLeft = std::nullopt, std::optional<UICornerRadius> topRight = std::nullopt) { return uniformBottomRadius(radius,topLeft,topRight); }
    static UICornerConfiguration configurationWithUniformLeftRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topRight, std::optional<UICornerRadius> bottomRight) { return uniformLeftRadius(radius,topRight,bottomRight); }
    static UICornerConfiguration configurationWithUniformRightRadius(UICornerRadius radius,
        std::optional<UICornerRadius> topLeft = std::nullopt, std::optional<UICornerRadius> bottomLeft = std::nullopt) { return uniformRightRadius(radius,topLeft,bottomLeft); }
    bool operator==(const UICornerConfiguration&) const = default;
private:
    friend class UIView;
    UICornerConfiguration(std::array<std::optional<UICornerRadius>,4> radii): _radii(radii) {}
    std::array<std::optional<UICornerRadius>,4> _radii;
    std::array<unsigned,2> _groups = {};
    bool _capsule = false;
    std::optional<NXFloat> _maximumRadius;
};
}
