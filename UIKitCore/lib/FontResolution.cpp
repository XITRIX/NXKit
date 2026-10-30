#include <SkiaCtx.h>
using namespace NXKit;

SkString SkiaCtx::getDefaultFontFamilyName() const {
    SkString familyName;
    if (!fontMgr) {
        return familyName;
    }

    if (auto typeface = getDefaultTypeface()) {
        typeface->getFamilyName(&familyName);
        return familyName;
    }

    if (fontMgr->countFamilies() > 0) {
        fontMgr->getFamilyName(0, &familyName);
    }
    return familyName;
}

sk_sp<SkTypeface> SkiaCtx::getDefaultTypeface(const SkFontStyle& style, NXFloat) const {
    if (!fontMgr) {
        return nullptr;
    }

    if (auto typeface = fontMgr->matchFamilyStyle(nullptr, style)) {
        return typeface;
    }

    // Custom-data managers enumerate their fonts but may not expose a
    // null-named default family. The first enumerated family is the manager's
    // own fallback in that case, so resolve it directly through its style set.
    if (fontMgr->countFamilies() > 0) {
        auto styles = fontMgr->createStyleSet(0);
        if (styles && styles->count() > 0) {
            return styles->matchStyle(style);
        }
    }
    return nullptr;
}

