#include "NXControllerIconProvider.h"

#include <SkiaCtx.h>

#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>
#include <include/core/SkTypeface.h>

#include <string_view>

namespace NXKit {

namespace {

struct SwitchGlyph {
    std::string_view utf8;
    SkUnichar codePoint = 0;
};

SwitchGlyph switchGlyphForButton(NXActionButton button) {
    // Nintendo's extended shared font mappings, verified against Borealis Hint:
    // https://github.com/XITRIX/borealis/blob/8c1f1093ee41a461d9d3bdd6dd2f6dfec166ab7a/library/lib/views/hint.cpp#L91
    switch (button) {
        case NXActionButton::a: return {"\uE0E0", 0xE0E0};
        case NXActionButton::b: return {"\uE0E1", 0xE0E1};
        case NXActionButton::x: return {"\uE0E2", 0xE0E2};
        case NXActionButton::y: return {"\uE0E3", 0xE0E3};
        case NXActionButton::leftThumbstick: return {"\uE104", 0xE104};
        case NXActionButton::rightThumbstick: return {"\uE105", 0xE105};
        case NXActionButton::leftShoulder: return {"\uE0E4", 0xE0E4};
        case NXActionButton::rightShoulder: return {"\uE0E5", 0xE0E5};
        case NXActionButton::leftTrigger: return {"\uE0E6", 0xE0E6};
        case NXActionButton::rightTrigger: return {"\uE0E7", 0xE0E7};
        case NXActionButton::plus: return {"\uE0EF", 0xE0EF};
        case NXActionButton::minus: return {"\uE0F0", 0xE0F0};
        case NXActionButton::dpadUp: return {"\uE0EB", 0xE0EB};
        case NXActionButton::dpadDown: return {"\uE0EC", 0xE0EC};
        case NXActionButton::dpadLeft: return {"\uE0ED", 0xE0ED};
        case NXActionButton::dpadRight: return {"\uE0EE", 0xE0EE};
        case NXActionButton::home:
        case NXActionButton::misc1:
        case NXActionButton::rightPaddle1:
        case NXActionButton::leftPaddle1:
        case NXActionButton::rightPaddle2:
        case NXActionButton::leftPaddle2:
        case NXActionButton::touchpad:
        case NXActionButton::misc2:
        case NXActionButton::misc3:
        case NXActionButton::misc4:
        case NXActionButton::misc5:
        case NXActionButton::misc6:
            return {};
    }
    return {};
}

sk_sp<SkTypeface> typefaceForCharacter(
    const sk_sp<SkFontMgr>& fontManager,
    SkUnichar character
) {
    if (!fontManager || character == 0) {
        return nullptr;
    }

    // SkFontMgr_New_Custom_Data does not implement character fallback, so
    // inspect its small set of Switch system families directly.
    for (int familyIndex = 0;
         familyIndex < fontManager->countFamilies();
         ++familyIndex) {
        const auto styles = fontManager->createStyleSet(familyIndex);
        if (!styles) {
            continue;
        }
        for (int styleIndex = 0; styleIndex < styles->count(); ++styleIndex) {
            auto candidate = styles->createTypeface(styleIndex);
            if (candidate && candidate->unicharToGlyph(character) != 0) {
                return candidate;
            }
        }
    }
    return nullptr;
}

class SwitchControllerIconProvider final : public NXControllerIconProvider {
public:
    std::shared_ptr<UIImage> iconForButton(
        NXActionButton button,
        NXControllerType,
        NXFloat pointSize,
        NXFloat scale
    ) const override {
        const auto context = SkiaCtx::main();
        if (!context) {
            return nullptr;
        }

        const auto glyph = switchGlyphForButton(button);
        if (glyph.utf8.empty()) {
            return nullptr;
        }

        const auto typeface = typefaceForCharacter(
            context->getFontMgr(),
            glyph.codePoint
        );
        if (!typeface) {
            return nullptr;
        }

        SkFont font(typeface, pointSize);
        return NXMakeControllerIconImage(
            pointSize,
            scale,
            [font, glyph](SkCanvas& canvas, NXFloat size) {
                SkRect bounds;
                font.measureText(
                    glyph.utf8.data(),
                    glyph.utf8.size(),
                    SkTextEncoding::kUTF8,
                    &bounds
                );

                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(SK_ColorBLACK);

                const auto x = (size - bounds.width()) / 2.0f - bounds.left();
                const auto y = (size - bounds.height()) / 2.0f - bounds.top();
                canvas.drawSimpleText(
                    glyph.utf8.data(),
                    glyph.utf8.size(),
                    SkTextEncoding::kUTF8,
                    x,
                    y,
                    font,
                    paint
                );
            }
        );
    }
};

} // namespace

std::unique_ptr<NXControllerIconProvider>
NXMakePlatformControllerIconProvider() {
    return std::make_unique<SwitchControllerIconProvider>();
}

} // namespace NXKit
