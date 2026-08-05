#include "NXControllerIconProvider.h"

#include <SkiaCtx.h>

#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>

#include <string_view>

namespace NXKit {

namespace {

std::string_view switchGlyphForButton(NXActionButton button) {
    // Nintendo's shared standard font mappings, verified against Borealis Hint:
    // https://github.com/XITRIX/borealis/blob/8c1f1093ee41a461d9d3bdd6dd2f6dfec166ab7a/library/lib/views/hint.cpp#L91
    switch (button) {
        case NXActionButton::a: return "\uE0E0";
        case NXActionButton::b: return "\uE0E1";
        case NXActionButton::x: return "\uE0E2";
        case NXActionButton::y: return "\uE0E3";
        case NXActionButton::leftThumbstick: return "\uE104";
        case NXActionButton::rightThumbstick: return "\uE105";
        case NXActionButton::leftShoulder: return "\uE0E4";
        case NXActionButton::rightShoulder: return "\uE0E5";
        case NXActionButton::leftTrigger: return "\uE0E6";
        case NXActionButton::rightTrigger: return "\uE0E7";
        case NXActionButton::plus: return "\uE0EF";
        case NXActionButton::minus: return "\uE0F0";
        case NXActionButton::dpadUp: return "\uE0EB";
        case NXActionButton::dpadDown: return "\uE0EC";
        case NXActionButton::dpadLeft: return "\uE0ED";
        case NXActionButton::dpadRight: return "\uE0EE";
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

        const auto typeface = context->getDefaultTypeface();
        const auto glyph = switchGlyphForButton(button);
        if (!typeface || glyph.empty()) {
            return nullptr;
        }

        SkFont font(typeface, pointSize);
        SkGlyphID glyphID = 0;
        if (font.textToGlyphs(
                glyph.data(),
                glyph.size(),
                SkTextEncoding::kUTF8,
                SkSpan(&glyphID, 1)
            ) != 1 || glyphID == 0) {
            return nullptr;
        }

        return NXMakeControllerIconImage(
            pointSize,
            scale,
            [font, glyph](SkCanvas& canvas, NXFloat size) {
                SkRect bounds;
                font.measureText(
                    glyph.data(),
                    glyph.size(),
                    SkTextEncoding::kUTF8,
                    &bounds
                );

                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(SK_ColorBLACK);

                const auto x = (size - bounds.width()) / 2.0f - bounds.left();
                const auto y = (size - bounds.height()) / 2.0f - bounds.top();
                canvas.drawSimpleText(
                    glyph.data(),
                    glyph.size(),
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
