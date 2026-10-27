#include <GlassContainerView.h>
#include "CAGlassContainerLayer.hpp"
#include <tools/IBTools.h>
#include <cmath>
#include <stdexcept>

using namespace NXKit;
void UIGlassContainerEffect::setSpacing(NXFloat value) {
    if (!std::isfinite(value) || value < 0) throw std::invalid_argument("Glass spacing must be finite and nonnegative");
    _spacing = value;
}
GlassContainerView::GlassContainerView(const UIGlassContainerEffect& effect):
    UIView(NXRect(), new_shared<CAGlassContainerLayer>(effect)), _contentView(new_shared<UIView>()) {
    addSubview(_contentView);
}
UIGlassContainerEffect GlassContainerView::effect() const {
    return std::static_pointer_cast<CAGlassContainerLayer>(layer())->effect();
}
void GlassContainerView::setEffect(const UIGlassContainerEffect& effect) {
    std::static_pointer_cast<CAGlassContainerLayer>(layer())->setEffect(effect);
}
void GlassContainerView::updateContentFrame() { _contentView->setFrame(bounds()); }
void GlassContainerView::setFrame(NXRect frame) { UIView::setFrame(frame); updateContentFrame(); }
void GlassContainerView::setBounds(NXRect bounds) { UIView::setBounds(bounds); updateContentFrame(); }
void GlassContainerView::layoutSubviews() { UIView::layoutSubviews(); updateContentFrame(); }
bool GlassContainerView::applyXMLAttribute(const std::string& name, const std::string& value) {
    if (name == "spacing") {
        const auto parsed = valueToFloat(value);
        if (!parsed || !std::isfinite(*parsed) || *parsed < 0) return false;
        auto copy = effect(); copy.setSpacing(*parsed); setEffect(copy); return true;
    }
    return UIView::applyXMLAttribute(name, value);
}
