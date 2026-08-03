#pragma once

#include <NSTextAlignment.h>
#include <UIScrollView.h>

#include <memory>
#include <string>

namespace NXKit {

struct UITextViewState;

class UITextView: public UIScrollView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UITextView>(); }

    explicit UITextView(NXRect frame = NXRect());
    ~UITextView() override;

    void setText(const std::string& text);
    [[nodiscard]] std::string text() const;

    void setTextColor(const UIColor& textColor);
    [[nodiscard]] UIColor textColor() const;

    void setFontSize(NXFloat fontSize);
    [[nodiscard]] NXFloat fontSize() const;

    void setFontWeight(NXFloat fontWeight);
    [[nodiscard]] NXFloat fontWeight() const;

    void setTextAlignment(NSTextAlignment textAlignment);
    [[nodiscard]] NSTextAlignment textAlignment() const;

    void setTextContainerInset(UIEdgeInsets textContainerInset);
    [[nodiscard]] UIEdgeInsets textContainerInset() const;

    void setHorizontalScrollEnabled(bool horizontalScrollEnabled);
    [[nodiscard]] bool isHorizontalScrollEnabled() const;

    NXSize sizeThatFits(NXSize size) override;
    void layoutSubviews() override;
    void setContentOffset(NXPoint offset, bool animated) override;
    void traitCollectionDidChange(
        std::shared_ptr<UITraitCollection> previousTraitCollection
    ) override;

    bool applyXMLAttribute(const std::string& name, const std::string& value) override;

private:
    std::unique_ptr<UITextViewState> _textViewState;

    void invalidateTextLayout();
    void updateTextLayout();
    void updateVisibleTiles();
};

}
