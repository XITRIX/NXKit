#pragma once
#include <UILabel.h>

// Apple HIG, Typography > iOS/iPadOS > Large (default).
// https://developer.apple.com/design/human-interface-guidelines/typography#Specifications
// Demo-only default-category metrics, not a Dynamic Type implementation.
namespace GlassScreenTypography {
struct Style { NXKit::NXFloat points; int weight; };
inline constexpr Style body{17, 400};
inline constexpr Style headline{17, 600};
inline constexpr Style footnote{13, 400};
inline constexpr Style caption2{11, 400};
// UITabBar's compact captions are a component-specific style, rather than
// the general HIG Caption 2 text style. Selected ink is a separate source row.
inline constexpr Style tabCaption{10, 600};
inline constexpr Style selectedTabCaption{10, 500};
inline void apply(const std::shared_ptr<NXKit::UILabel>& label, Style style) {
    label->setFontSize(style.points);
    label->setFontWeight(style.weight);
}
}
