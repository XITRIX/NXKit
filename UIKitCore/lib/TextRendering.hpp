#pragma once
#include "FontTracking.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkTextBlob.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include <SkiaCtx.h>

namespace NXKit::Internal {

template<class Canvas>
inline void drawPlainRun(Canvas* canvas, const skia::textlayout::Paragraph::VisitorInfo& run,
                         SkPoint origin, const SkFont& font, const SkPaint& paint) {
    // Skia changed the public drawGlyphs overload from count/pointers to spans.
    if constexpr (requires { canvas->drawGlyphs(run.count, run.glyphs, run.positions, origin, font, paint); })
        canvas->drawGlyphs(run.count, run.glyphs, run.positions, origin, font, paint);
    else canvas->drawGlyphs(SkSpan<const SkGlyphID>(run.glyphs, run.count),
                           SkSpan<const SkPoint>(run.positions, run.count), origin, font, paint);
}

inline sk_sp<skia::textlayout::FontCollection> plainTextFonts(
    const std::shared_ptr<SkiaCtx>& context, const SkFontStyle& fontStyle,
    NXFloat pointSize, skia::textlayout::TextStyle& style) {
    using namespace skia::textlayout;
    auto fonts = sk_make_sp<FontCollection>();
    auto family = context->getDefaultFontFamilyName();
    if (family.isEmpty()) fonts->setDefaultFontManager(context->getFontMgr());
    else fonts->setDefaultFontManager(context->getFontMgr(), family.c_str());
    if (auto face = context->getDefaultTypeface(fontStyle, pointSize)) {
        // TextStyle::setTypeface alone does not control SkParagraph shaping.
        // Register the resolved face, retaining its real family name for
        // fallback queries. This also preserves weight and optical size.
        SkString resolvedFamily;
        face->getFamilyName(&resolvedFamily);
        if (resolvedFamily.isEmpty()) resolvedFamily = SkString("NXKit.ResolvedDefault");
        auto provider = sk_make_sp<TypefaceFontProvider>();
        provider->registerTypeface(face, resolvedFamily);
        fonts->setAssetFontManager(provider);
        style.setTypeface(face);
        style.setFontFamilies({resolvedFamily});
        style.setLetterSpacing(fontTracking(*face, pointSize));
    } else if (!family.isEmpty()) style.setFontFamilies({family});
    return fonts;
}

// These three text APIs currently support a single foreground color, without
// attributed backgrounds, shadows or decorations. Keep SkParagraph's shaped
// glyphs, fallback runs and positions; only choose the rasterization policy.
inline void paintPlainText(skia::textlayout::Paragraph& paragraph, SkCanvas* canvas,
                           SkScalar x, SkScalar y, SkColor color,
                           const std::vector<SkScalar>& baselines = {}) {
#ifndef __APPLE__
    if (baselines.empty()) { paragraph.paint(canvas, x, y); return; }
#endif
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    paragraph.visit([&](int line, const skia::textlayout::Paragraph::VisitorInfo* run) {
        if (!run || run->count <= 0) return;
        auto font = run->font;
#ifdef __APPLE__
        // In Skia's Core Text backend, hinting requests macOS font smoothing
        // even with grayscale AA. UIKit uses unsmoothed glyph coverage.
        font.setHinting(SkFontHinting::kNone);
        font.setBaselineSnap(false);
#endif
        const auto baseline = size_t(line) < baselines.size() ? baselines[line] : run->origin.y();
        drawPlainRun(canvas, *run, SkPoint::Make(x + run->origin.x(), y + baseline), font, paint);
    });
}

}
