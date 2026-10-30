#pragma once
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
namespace NXKit::Internal {
sk_sp<SkTypeface> appleSystemTypeface(const SkFontStyle& style, float pointSize);
}
