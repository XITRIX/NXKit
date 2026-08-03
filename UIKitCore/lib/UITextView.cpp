#include <UITextView.h>

#include <SkiaCtx.h>
#include <tools/IBTools.h>

#include <include/core/SkFontStyle.h>
#include <include/core/SkPaint.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skunicode/include/SkUnicode_icu.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace skia::textlayout;

namespace NXKit {
namespace {

constexpr NXFloat kLineFragmentPadding = 5;
constexpr NXFloat kTileHeight = 1024;

struct TextLayout {
    std::shared_ptr<Paragraph> paragraph;
    NXFloat width = 0;
    NXFloat height = 0;
    NXFloat longestLine = 0;
};

ParagraphStyle makeParagraphStyle(
    const UIColor& textColor,
    NXFloat fontSize,
    NXFloat fontWeight,
    NSTextAlignment textAlignment,
    const SkString& defaultFamilyName,
    const sk_sp<SkTypeface>& typeface
) {
    SkFontStyle fontStyle(
        static_cast<int>(fontWeight),
        SkFontStyle::kNormal_Width,
        SkFontStyle::kUpright_Slant
    );

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(textColor.raw());

    TextStyle textStyle;
    textStyle.setForegroundColor(paint);
    textStyle.setTypeface(typeface);
    if (!defaultFamilyName.isEmpty()) {
        textStyle.setFontFamilies({defaultFamilyName});
    }
    textStyle.setFontSize(fontSize);
    textStyle.setFontStyle(fontStyle);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(textStyle);
    switch (textAlignment) {
        case NSTextAlignment::left:
            paragraphStyle.setTextAlign(TextAlign::kLeft);
            break;
        case NSTextAlignment::right:
            paragraphStyle.setTextAlign(TextAlign::kRight);
            break;
        case NSTextAlignment::center:
            paragraphStyle.setTextAlign(TextAlign::kCenter);
            break;
        case NSTextAlignment::justified:
            paragraphStyle.setTextAlign(TextAlign::kJustify);
            break;
        case NSTextAlignment::natural:
            paragraphStyle.setTextAlign(TextAlign::kStart);
            break;
    }
    return paragraphStyle;
}

std::shared_ptr<TextLayout> buildTextLayout(
    const std::string& text,
    const UIColor& textColor,
    NXFloat fontSize,
    NXFloat fontWeight,
    NSTextAlignment textAlignment,
    NXFloat width,
    bool horizontalScrollEnabled,
    const sk_sp<SkUnicode>& unicode
) {
    auto result = std::make_shared<TextLayout>();
    result->width = std::max<NXFloat>(0, width);
    if (result->width <= 0 || !unicode) return result;

    const auto skiaContext = SkiaCtx::main();
    if (!skiaContext) return result;

    const auto fontManager = skiaContext->getFontMgr();
    const auto defaultFamilyName = skiaContext->getDefaultFontFamilyName();
    const SkFontStyle fontStyle(
        static_cast<int>(fontWeight),
        SkFontStyle::kNormal_Width,
        SkFontStyle::kUpright_Slant
    );
    const auto typeface = skiaContext->getDefaultTypeface(fontStyle);

    auto fontCollection = sk_make_sp<FontCollection>();
    if (defaultFamilyName.isEmpty()) {
        fontCollection->setDefaultFontManager(fontManager);
    } else {
        fontCollection->setDefaultFontManager(
            fontManager,
            defaultFamilyName.c_str()
        );
    }

    auto paragraphStyle = makeParagraphStyle(
        textColor,
        fontSize,
        fontWeight,
        textAlignment,
        defaultFamilyName,
        typeface
    );
    auto builder = ParagraphBuilder::make(
        paragraphStyle,
        fontCollection,
        unicode
    );
    if (!builder) return result;

    builder->addText(text.c_str());
    auto paragraph = builder->Build();
    if (!paragraph) return result;

    if (horizontalScrollEnabled) {
        paragraph->layout(std::numeric_limits<NXFloat>::infinity());
        const auto intrinsicWidth = static_cast<NXFloat>(
            std::ceil(paragraph->getMaxIntrinsicWidth())
        );
        if (std::isfinite(intrinsicWidth)) {
            result->width = std::max(result->width, intrinsicWidth);
        }
    }
    paragraph->layout(result->width);
    result->height = std::max<NXFloat>(
        0,
        static_cast<NXFloat>(std::ceil(paragraph->getHeight()))
    );
    result->longestLine = std::max<NXFloat>(
        0,
        static_cast<NXFloat>(std::ceil(paragraph->getLongestLine()))
    );
    result->paragraph = std::shared_ptr<Paragraph>(std::move(paragraph));
    return result;
}

class TextTileLayer final: public CALayer {
public:
    TextTileLayer(
        std::shared_ptr<TextLayout> textLayout,
        NXFloat paragraphOriginY
    ):
        _textLayout(std::move(textLayout)),
        _paragraphOriginY(paragraphOriginY)
    {
        setMasksToBounds(true);
    }

    explicit TextTileLayer(TextTileLayer* layer):
        CALayer(layer),
        _textLayout(layer->_textLayout),
        _paragraphOriginY(layer->_paragraphOriginY)
    { }

    void draw(SkCanvas* canvas) override {
        if (!_textLayout || !_textLayout->paragraph) return;
        _textLayout->paragraph->paint(canvas, 0, -_paragraphOriginY);
    }

    std::shared_ptr<CALayer> copy() override {
        return new_shared<TextTileLayer>(this);
    }

private:
    std::shared_ptr<TextLayout> _textLayout;
    NXFloat _paragraphOriginY = 0;
};

class TextTileView final: public UIView {
public:
    TextTileView(
        const std::shared_ptr<TextLayout>& textLayout,
        NXFloat paragraphOriginY
    ):
        UIView(
            NXRect(),
            new_shared<TextTileLayer>(textLayout, paragraphOriginY)
        )
    { }
};

bool finiteInsets(const UIEdgeInsets& insets) {
    return std::isfinite(insets.top)
        && std::isfinite(insets.left)
        && std::isfinite(insets.bottom)
        && std::isfinite(insets.right);
}

} // namespace

struct UITextViewState {
    std::string text;
    UIColor textColor = UIColor::label;
    NXFloat fontSize = 17;
    NXFloat fontWeight = SkFontStyle::kNormal_Weight;
    NSTextAlignment textAlignment = NSTextAlignment::natural;
    UIEdgeInsets textContainerInset = UIEdgeInsets(8, 0, 8, 0);
    bool horizontalScrollEnabled = false;

    sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
    std::shared_ptr<TextLayout> textLayout;
    NXFloat textLayoutWidth = -1;

    std::shared_ptr<UIView> contentView;
    std::map<size_t, std::shared_ptr<UIView>> tiles;
};

UITextView::UITextView(NXRect frame):
    UIScrollView(frame),
    _textViewState(std::make_unique<UITextViewState>())
{
    _textViewState->contentView = new_shared<UIView>();
    UIScrollView::addSubview(_textViewState->contentView);
    setBounceHorizontally(false);
    setBounceVertically(true);
}

UITextView::~UITextView() = default;

void UITextView::setText(const std::string& text) {
    if (_textViewState->text == text) return;
    _textViewState->text = text;
    invalidateTextLayout();
}

std::string UITextView::text() const {
    return _textViewState->text;
}

void UITextView::setTextColor(const UIColor& textColor) {
    if (_textViewState->textColor == textColor) return;
    _textViewState->textColor = textColor;
    invalidateTextLayout();
}

UIColor UITextView::textColor() const {
    return _textViewState->textColor;
}

void UITextView::setFontSize(NXFloat fontSize) {
    if (!std::isfinite(fontSize) || fontSize <= 0) {
        throw std::invalid_argument("UITextView font size must be finite and positive");
    }
    if (_textViewState->fontSize == fontSize) return;
    _textViewState->fontSize = fontSize;
    invalidateTextLayout();
}

NXFloat UITextView::fontSize() const {
    return _textViewState->fontSize;
}

void UITextView::setFontWeight(NXFloat fontWeight) {
    if (!std::isfinite(fontWeight)) {
        throw std::invalid_argument("UITextView font weight must be finite");
    }
    if (_textViewState->fontWeight == fontWeight) return;
    _textViewState->fontWeight = fontWeight;
    invalidateTextLayout();
}

NXFloat UITextView::fontWeight() const {
    return _textViewState->fontWeight;
}

void UITextView::setTextAlignment(NSTextAlignment textAlignment) {
    if (_textViewState->textAlignment == textAlignment) return;
    _textViewState->textAlignment = textAlignment;
    invalidateTextLayout();
}

NSTextAlignment UITextView::textAlignment() const {
    return _textViewState->textAlignment;
}

void UITextView::setTextContainerInset(UIEdgeInsets textContainerInset) {
    if (!finiteInsets(textContainerInset)) {
        throw std::invalid_argument("UITextView text container insets must be finite");
    }
    if (_textViewState->textContainerInset == textContainerInset) return;
    _textViewState->textContainerInset = textContainerInset;
    invalidateTextLayout();
}

UIEdgeInsets UITextView::textContainerInset() const {
    return _textViewState->textContainerInset;
}

void UITextView::setHorizontalScrollEnabled(bool horizontalScrollEnabled) {
    if (_textViewState->horizontalScrollEnabled == horizontalScrollEnabled) {
        return;
    }
    _textViewState->horizontalScrollEnabled = horizontalScrollEnabled;
    setBounceHorizontally(horizontalScrollEnabled);
    invalidateTextLayout();
}

bool UITextView::isHorizontalScrollEnabled() const {
    return _textViewState->horizontalScrollEnabled;
}

NXSize UITextView::sizeThatFits(NXSize size) {
    const auto insets = _textViewState->textContainerInset;
    const auto textWidth = std::max<NXFloat>(
        0,
        size.width - insets.left - insets.right - 2 * kLineFragmentPadding
    );
    const auto layout = buildTextLayout(
        _textViewState->text,
        _textViewState->textColor,
        _textViewState->fontSize,
        _textViewState->fontWeight,
        _textViewState->textAlignment,
        textWidth,
        _textViewState->horizontalScrollEnabled,
        _textViewState->unicode
    );

    const auto fittedWidth = layout
        ? layout->longestLine + insets.left + insets.right
            + 2 * kLineFragmentPadding
        : insets.left + insets.right + 2 * kLineFragmentPadding;
    const auto fittedHeight = layout
        ? layout->height + insets.top + insets.bottom
        : insets.top + insets.bottom;
    return {
        std::min(std::max<NXFloat>(0, size.width), fittedWidth),
        fittedHeight
    };
}

void UITextView::layoutSubviews() {
    UIScrollView::layoutSubviews();
    // Yoga and safe-area propagation finalize the viewport in the superclass.
    // Text wrapping must use that final width, never the pre-layout frame.
    updateTextLayout();
    updateVisibleTiles();
}

void UITextView::setContentOffset(NXPoint offset, bool animated) {
    UIScrollView::setContentOffset(offset, animated);
    updateVisibleTiles();
}

void UITextView::traitCollectionDidChange(
    std::shared_ptr<UITraitCollection> previousTraitCollection
) {
    UIScrollView::traitCollectionDidChange(std::move(previousTraitCollection));
    invalidateTextLayout();
}

bool UITextView::applyXMLAttribute(
    const std::string& name,
    const std::string& value
) {
    if (UIScrollView::applyXMLAttribute(name, value)) return true;

    REGISTER_XIB_ATTRIBUTE(text, valueToString, setText)
    REGISTER_XIB_ATTRIBUTE(textColor, valueToColor, setTextColor)
    REGISTER_XIB_ATTRIBUTE(fontSize, valueToFloat, setFontSize)
    REGISTER_XIB_ATTRIBUTE(fontWeight, valueToFloat, setFontWeight)
    REGISTER_XIB_ATTRIBUTE(textAlignment, valueToTextAlignment, setTextAlignment)
    REGISTER_XIB_ATTRIBUTE(
        horizontalScrollEnabled,
        valueToBool,
        setHorizontalScrollEnabled
    )

    return false;
}

void UITextView::invalidateTextLayout() {
    for (auto& [_, tile] : _textViewState->tiles) {
        tile->removeFromSuperview();
    }
    _textViewState->tiles.clear();
    _textViewState->textLayout.reset();
    _textViewState->textLayoutWidth = -1;
    setNeedsLayout();
}

void UITextView::updateTextLayout() {
    const auto scrollInsets = adjustedContentInset();
    const auto viewportWidth = std::max<NXFloat>(
        0,
        bounds().width() - scrollInsets.left - scrollInsets.right
    );
    const auto insets = _textViewState->textContainerInset;
    const auto textWidth = std::max<NXFloat>(
        0,
        viewportWidth - insets.left - insets.right - 2 * kLineFragmentPadding
    );

    if (!_textViewState->textLayout
        || _textViewState->textLayoutWidth != textWidth) {
        for (auto& [_, tile] : _textViewState->tiles) {
            tile->removeFromSuperview();
        }
        _textViewState->tiles.clear();
        _textViewState->textLayout = buildTextLayout(
            _textViewState->text,
            _textViewState->textColor,
            _textViewState->fontSize,
            _textViewState->fontWeight,
            _textViewState->textAlignment,
            textWidth,
            _textViewState->horizontalScrollEnabled,
            _textViewState->unicode
        );
        _textViewState->textLayoutWidth = textWidth;
    }

    const auto textHeight = _textViewState->textLayout
        ? _textViewState->textLayout->height
        : 0;
    const auto paragraphWidth = _textViewState->textLayout
        ? _textViewState->textLayout->width
        : 0;
    const auto fittedContentWidth = paragraphWidth
        + insets.left + insets.right + 2 * kLineFragmentPadding;
    const auto contentFrame = NXRect(
        0,
        0,
        _textViewState->horizontalScrollEnabled
            ? std::max(viewportWidth, fittedContentWidth)
            : viewportWidth,
        std::max<NXFloat>(0, textHeight + insets.top + insets.bottom)
    );
    if (_textViewState->contentView->frame() != contentFrame) {
        _textViewState->contentView->setFrame(contentFrame);
    }
    setContentSize(contentFrame.size);
}

void UITextView::updateVisibleTiles() {
    updateTextLayout();
    const auto layout = _textViewState->textLayout;
    if (!layout || !layout->paragraph || layout->height <= 0) return;

    const auto insets = _textViewState->textContainerInset;
    const auto visibleTop = contentOffset().y - insets.top - kTileHeight;
    const auto visibleBottom = contentOffset().y + bounds().height()
        - insets.top + kTileHeight;
    const auto tileCount = static_cast<size_t>(
        std::ceil(layout->height / kTileHeight)
    );
    const auto firstTile = static_cast<size_t>(std::max<NXFloat>(
        0,
        std::floor(visibleTop / kTileHeight)
    ));
    const auto lastTile = static_cast<size_t>(std::min<NXFloat>(
        static_cast<NXFloat>(tileCount - 1),
        std::max<NXFloat>(0, std::floor(visibleBottom / kTileHeight))
    ));

    std::vector<size_t> tilesToRemove;
    for (const auto& [index, _] : _textViewState->tiles) {
        if (index < firstTile || index > lastTile) {
            tilesToRemove.push_back(index);
        }
    }
    for (const auto index : tilesToRemove) {
        _textViewState->tiles[index]->removeFromSuperview();
        _textViewState->tiles.erase(index);
    }

    for (auto index = firstTile; index <= lastTile; ++index) {
        if (_textViewState->tiles.contains(index)) continue;

        const NXFloat paragraphOriginY = static_cast<NXFloat>(index) * kTileHeight;
        const NXFloat tileHeight = std::min(
            kTileHeight,
            layout->height - paragraphOriginY
        );
        auto tile = new_shared<TextTileView>(layout, paragraphOriginY);
        tile->setFrame(NXRect(
            insets.left + kLineFragmentPadding,
            insets.top + paragraphOriginY,
            layout->width,
            tileHeight
        ));
        _textViewState->contentView->addSubview(tile);
        _textViewState->tiles.emplace(index, std::move(tile));
    }
}

}
