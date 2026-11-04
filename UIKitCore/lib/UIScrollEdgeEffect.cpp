#include <UIScrollEdgeEffect.h>
#include <stdexcept>
using namespace NXKit;
void UIScrollEdgeEffect::setStyle(UIScrollEdgeEffectStyle style) {
    if(style!=UIScrollEdgeEffectStyle::automatic&&style!=UIScrollEdgeEffectStyle::soft&&style!=UIScrollEdgeEffectStyle::hard)
        throw std::invalid_argument("Unknown scroll edge effect style");
    if(_style==style)return;_style=style;if(_invalidate)_invalidate();
}
void UIScrollEdgeEffect::setHidden(bool hidden) {
    if(_hidden==hidden)return;_hidden=hidden;if(_invalidate)_invalidate();
}
