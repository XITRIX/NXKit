#pragma once
#include <UIView.h>
#include <UIGlassContainerEffect.h>

namespace NXKit {
/** Portable host for UIGlassContainerEffect. Add BackdropEffectView glass
 items to contentView(). Their material merges behind all container content;
 their layout, hit testing, controls and interactive feedback stay independent.
 Ordinary affine wrapper views are supported. Nested containers form separate
 groups. Masked/perspective wrapper branches render independently. */
class GlassContainerView : public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<GlassContainerView>(); }
    explicit GlassContainerView(const UIGlassContainerEffect& effect = {});
    [[nodiscard]] std::shared_ptr<UIView> contentView() const { return _contentView; }
    [[nodiscard]] UIGlassContainerEffect effect() const;
    void setEffect(const UIGlassContainerEffect& effect);
    void setFrame(NXRect frame) override;
    void setBounds(NXRect bounds) override;
    void layoutSubviews() override;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;
private:
    void updateContentFrame();
    void updateDisplayScale();
    std::shared_ptr<UIView> _contentView;
    NXFloat _resolvedDisplayScale = 0;
};
}
