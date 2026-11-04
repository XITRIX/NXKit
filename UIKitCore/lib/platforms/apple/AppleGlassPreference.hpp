#pragma once
#include <optional>

namespace NXKit::Internal {
// Read-only bridge to the OS 27 Liquid Glass preference. Missing/invalid native
// values remain unspecified; the portable backend supplies its own fallback.
std::optional<float> appleGlassTintAmount();
}
