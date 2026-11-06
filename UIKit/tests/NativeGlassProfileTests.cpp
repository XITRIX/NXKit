#include "NativeGlassProfile.hpp"
#include "NativeGlassProfileData.hpp"
#include "NativeGlassSizeProfileData.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (condition) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}
bool near(NXFloat a, NXFloat b) { return std::abs(a - b) < .0001f; }
bool sameColor(NativeGlassColor a, NativeGlassColor b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
bool hash(std::string_view value) {
    return value.size() == 64 && value.find_first_not_of("0123456789abcdef") == std::string_view::npos;
}

void measuredAnchors() {
    // 63 numeric + 3 flag + 2 color + 1 offset + 1 source identity inputs.
    static_assert(nativeGlassScalarFields.size() + nativeGlassBoolFields.size()
        + nativeGlassColorFields.size() + 2 == 70);
    size_t index = 0;
    for (auto theme : {UIUserInterfaceStyle::light, UIUserInterfaceStyle::dark}) {
        for (auto style : {UIGlassEffect::Style::regular, UIGlassEffect::Style::clear}) {
            for (NXFloat amount : {0.f, .5f, 1.f}) {
                const auto actual = nativeGlassProfile(style, theme, amount);
                const auto& measured = nativeGlassProfiles[index++];
                for (auto field : nativeGlassScalarFields)
                    expect(actual.*field == measured.*field, "all numeric inputs preserve exact measured anchors");
                for (auto field : nativeGlassBoolFields)
                    expect(actual.*field == measured.*field, "all native modes preserve exact measured anchors");
                for (auto field : nativeGlassColorFields)
                    expect(sameColor(actual.*field, measured.*field), "color anchors preserve even invisible source RGB");
                for (auto field : nativeGlassHighlightScalarFields)
                    expect(actual.highlight.*field == measured.highlight.*field, "all applied highlight settings retain native anchors");
                expect(actual.accent.vibrant.matrix.values == measured.accent.vibrant.matrix.values
                    && actual.highlight.vibrant.matrix.values == measured.highlight.vibrant.matrix.values,
                    "applied accent and highlight matrices retain every coefficient");
                expect(actual.shadowOffset.x == measured.shadowOffset.x && actual.shadowOffset.y == measured.shadowOffset.y,
                    "native offset retains both coordinates");
                expect(actual.context.runtime == "iOS 27.0" && actual.context.build == "24A434"
                    && actual.context.width == 280 && actual.context.height == 80
                    && actual.context.cornerRadius == 40 && actual.context.displayScale == 3,
                    "profile context identifies the captured runtime and geometry");
                const bool dark = theme == UIUserInterfaceStyle::dark;
                expect(actual.context.background == (dark ? "systemBlack" : "systemWhite")
                    && actual.context.capturedTintColor == "systemBlue", "background and accent evidence remain explicit");
                expect(actual.context.backdropScale == (style == UIGlassEffect::Style::regular && amount == 1 ? .125f : .5f),
                    "full-density regular capture uses its measured lower-resolution backdrop");
                const std::string expectedName = std::string(dark ? "dark-" : "light-")
                    + (amount == 0 ? "0" : amount == 1 ? "1" : "system");
                expect(actual.captureName == expectedName && actual.sourceSublayerName == "@0"
                    && hash(actual.backdropArchiveSHA256) && hash(actual.layerSourceSHA256),
                    "profiles retain source capture, archive and normalized-layer identities");
                expect(actual.accent.premultiplied && actual.accent.distances == std::array<NXFloat, 3>{-1, 0, 10}
                    && actual.accent.fillCompositingFilter == "destIn", "accent composition retains native SDF distance and mask settings");
                expect(actual.accent.hasInterpolationControlPoints == std::array<bool, 2>{false, false},
                    "original fixture does not invent unavailable timing control points");
                expect(actual.accent.interpolationClasses[0] == "CAMediaTimingFunctionBuiltin"
                    && actual.accent.interpolationClasses[1] == "CAMediaTimingFunction",
                    "unrecovered gradient easing is identified by archived class instead of fabricated coefficients");
            }
        }
    }
    expect(nativeGlassProfiles[0].backdropArchiveSHA256 == "8a18c9afb8a5d3154437505e63c94a163eb1c888484c23dbb7249c4a5c41687a",
        "light-0 regular identifies the original archived filter exactly");
    expect(nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, 0).blurRadius == 5
        && nativeGlassProfile(UIGlassEffect::Style::clear, UIUserInterfaceStyle::dark, 0).blurRadius == 0,
        "zero system tint preserves regular blur while clear's measured main blur is zero");
}

void interpolationCoverage() {
    auto a = nativeGlassProfiles[0];
    auto b = a;
    // Distinct per-field sentinels make an omitted field observable even when
    // the real fixture happens to keep that setting constant across tint levels.
    NXFloat sentinel = 0;
    for (auto field : nativeGlassScalarFields) {
        a.*field = sentinel;
        b.*field = sentinel + 8;
        sentinel += 16;
    }
    for (auto field : nativeGlassBoolFields) { a.*field = false; b.*field = true; }
    for (auto field : nativeGlassHighlightScalarFields) {
        a.highlight.*field = sentinel;
        b.highlight.*field = sentinel + 8;
        sentinel += 16;
    }
    for (size_t i = 0; i < a.accent.vibrant.matrix.values.size(); ++i) {
        a.accent.vibrant.matrix.values[i] = a.highlight.vibrant.matrix.values[i] = NXFloat(i) - 10;
        b.accent.vibrant.matrix.values[i] = b.highlight.vibrant.matrix.values[i] = NXFloat(i) + 10;
    }
    for (auto field : nativeGlassColorFields) { a.*field = {1, 0, 0, 0}; b.*field = {0, 0, 1, 1}; }
    a.shadowOffset = {-2, 6}; b.shadowOffset = {2, -6};
    a.output = {-8, 4}; b.output = {-4, 12};
    a.highlight.keyColor = a.highlight.fillColor = a.accent.fillColor = {1, 0, 0, 0};
    b.highlight.keyColor = b.highlight.fillColor = b.accent.fillColor = {0, 0, 1, 1};
    a.highlight.vibrant.clamp = a.accent.vibrant.clamp = 0;
    b.highlight.vibrant.clamp = b.accent.vibrant.clamp = 2;
    a.highlight.global = a.highlight.vibrant.clampPreserveHue = a.accent.vibrant.clampPreserveHue = false;
    b.highlight.global = b.highlight.vibrant.clampPreserveHue = b.accent.vibrant.clampPreserveHue = true;
    a.accent.hasInterpolationControlPoints = b.accent.hasInterpolationControlPoints = {true, true};
    for (size_t i = 0; i < a.accent.interpolationControlPoints.size(); ++i) {
        a.accent.interpolationControlPoints[i] = {0, 0, .5f, .5f};
        b.accent.interpolationControlPoints[i] = {.5f, 1, 1, .75f};
    }
    a.accent.premultiplied = a.accent.vibrant.backdropAware = a.accent.vibrant.hasBackdropAware = false;
    b.accent.premultiplied = b.accent.vibrant.backdropAware = b.accent.vibrant.hasBackdropAware = true;
    a.sourceSublayerName = "a"; b.sourceSublayerName = "b";
    for (size_t i = 0; i < a.accent.colors.size(); ++i) {
        a.accent.colors[i] = {1, 0, 0, 0}; b.accent.colors[i] = {0, 0, 1, 1};
        a.accent.distances[i] = NXFloat(i); b.accent.distances[i] = NXFloat(i) + 4;
    }
    const auto quarter = interpolateNativeGlassProfiles(a, b, .25f);
    for (auto field : nativeGlassScalarFields)
        expect(near(quarter.*field, a.*field + 2), "every native numeric input participates in interpolation");
    for (auto field : nativeGlassBoolFields)
        expect(!(quarter.*field) && interpolateNativeGlassProfiles(a, b, .5f).*field,
            "flags remain discrete and choose the upper measured setting at ties");
    expect(sameColor(quarter.faceColorMatrixFillColor, {.75f, 0, .25f, .25f}),
        "face-fill filter coefficients interpolate straight RGBA independently of their alpha");
    expect(sameColor(quarter.shadowColorMatrixFillColor, {0, 0, 1, .25f}),
        "transparent RGB never leaks through premultiplied shadow color interpolation");
    for (auto field : nativeGlassHighlightScalarFields)
        expect(near(quarter.highlight.*field, a.highlight.*field + 2), "all applied highlight scalar settings interpolate");
    for (size_t i = 0; i < a.accent.vibrant.matrix.values.size(); ++i)
        expect(quarter.accent.vibrant.matrix.values[i] == NXFloat(i) - 5
            && quarter.highlight.vibrant.matrix.values[i] == NXFloat(i) - 5, "both vibrant matrices interpolate all 20 coefficients");
    expect(quarter.shadowOffset.x == -1 && quarter.shadowOffset.y == 3
        && quarter.output.minimum == -7 && quarter.output.maximum == 6,
        "SDF output range and native shadow vector interpolate independently");
    expect(sameColor(quarter.highlight.keyColor, {0, 0, 1, .25f})
        && sameColor(quarter.highlight.fillColor, {0, 0, 1, .25f})
        && sameColor(quarter.accent.fillColor, {0, 0, 1, .25f}), "highlight and mask colors use premultiplied interpolation too");
    for (size_t i = 0; i < a.accent.colors.size(); ++i)
        expect(sameColor(quarter.accent.colors[i], {.75f, 0, .25f, .25f})
            && quarter.accent.distances[i] == NXFloat(i) + 1,
            "straight gradient mode interpolates color coefficients and SDF distance independently");
    a.accent.premultiplied = b.accent.premultiplied = true;
    for (const auto& color : interpolateNativeGlassProfiles(a, b, .25f).accent.colors)
        expect(sameColor(color, {0, 0, 1, .25f}), "premultiplied gradient mode excludes invisible RGB");
    a.accent.premultiplied = false;
    for (const auto& controls : quarter.accent.interpolationControlPoints)
        expect(controls == std::array<NXFloat, 4>{.125f, .25f, .625f, .5625f},
            "measured cubic timing functions interpolate every control coordinate");
    a.accent.hasInterpolationControlPoints[0] = false;
    expect(interpolateNativeGlassProfiles(a, b, .5f).accent.interpolationControlPoints[0]
        == b.accent.interpolationControlPoints[0], "unknown timing data never acts as a fabricated zero curve");
    expect(quarter.highlight.vibrant.clamp == .5f && quarter.accent.vibrant.clamp == .5f,
        "highlight and accent clamp scalars remain continuous");
    expect(!quarter.highlight.global && !quarter.accent.premultiplied
        && !quarter.accent.vibrant.clampPreserveHue && !quarter.accent.vibrant.backdropAware
        && !quarter.accent.vibrant.hasBackdropAware, "composite pass flags use the explicit discrete policy");
    expect(quarter.sourceSublayerName == "a" && interpolateNativeGlassProfiles(a, b, .5f).sourceSublayerName == "b",
        "nonvisual source identities select an endpoint without numeric conversion");
    expect(sameColor(interpolateNativeGlassProfiles(a, b, 0).faceColorMatrixFillColor, {1, 0, 0, 0})
        && sameColor(interpolateNativeGlassProfiles(a, b, 1).faceColorMatrixFillColor, {0, 0, 1, 1}),
        "exact color anchors preserve invisible source values");
    b.faceColorMatrixFillColor = {0, 1, 0, 0};
    expect(sameColor(interpolateNativeGlassProfiles(a, b, .5f).faceColorMatrixFillColor, {.5f, .5f, 0, 0}),
        "zero-alpha face-fill coefficients retain their independent native RGB interpolation");
    b.shadowColorMatrixFillColor = {0, 1, 0, 0};
    expect(sameColor(interpolateNativeGlassProfiles(a, b, .5f).shadowColorMatrixFillColor, {0, 0, 0, 0}),
        "two fully transparent pixel colors do not divide by zero or carry hidden RGB");
}

void measuredSizeProfiles() {
    expect(nativeGlassSizeProfiles.size() == 112, "size data covers 7 size anchors, 2 shapes, 2 styles, 2 appearances and neutral/colored variants");
    for (const auto& set : nativeGlassSizeProfiles) {
        for (size_t anchor = 0; anchor < set.profiles.size(); ++anchor) {
            const auto& expected = set.profiles[anchor];
            const NXSize size{expected.context.width, expected.context.height};
            const auto actual = nativeGlassProfile(set.style, set.appearance, nativeGlassSizeTintAnchors[anchor], size, set.tinted);
            for (auto field : nativeGlassScalarFields)
                expect(actual.*field == expected.*field, "all 70 size-resolved inputs preserve captured geometry anchors");
            for (auto field : nativeGlassBoolFields)
                expect(actual.*field == expected.*field, "size-resolved native flags preserve their measured setting");
            for (auto field : nativeGlassColorFields)
                expect(sameColor(actual.*field, expected.*field), "size-resolved colors preserve measured endpoints");
            for (auto field : nativeGlassHighlightScalarFields)
                expect(actual.highlight.*field == expected.highlight.*field, "size-resolved highlight controls preserve captured values");
            expect(actual.accent.hasInterpolationControlPoints == std::array<bool, 2>{true, true}
                && actual.accent.interpolationControlPoints == expected.accent.interpolationControlPoints,
                "every sized variant retains actual public timing-function control points");
            expect(actual.accent.vibrant.matrix.values == expected.accent.vibrant.matrix.values
                && actual.output.minimum == expected.output.minimum && actual.output.maximum == expected.output.maximum,
                "size-resolved accent/SDF output parameters preserve captured values");
            expect(actual.context.width == size.width && actual.context.height == size.height
                && actual.backdropArchiveSHA256 == expected.backdropArchiveSHA256
                && actual.layerSourceSHA256 == expected.layerSourceSHA256,
                "size query retains the source's actual measured geometry and evidence identity");
            const NXFloat dimension = std::min(size.width, size.height);
            const auto circle = nativeGlassProfile(set.style, set.appearance, nativeGlassSizeTintAnchors[anchor], {dimension, dimension}, set.tinted);
            const auto wide = nativeGlassProfile(set.style, set.appearance, nativeGlassSizeTintAnchors[anchor], {280, dimension}, set.tinted);
            for (auto field : nativeGlassScalarFields)
                expect(circle.*field == wide.*field, "measured circle and wide materials agree on every optical input");
            expect(circle.highlight.vibrant.matrix.values == wide.highlight.vibrant.matrix.values
                && circle.accent.vibrant.matrix.values == wide.accent.vibrant.matrix.values,
                "measured aspect families share applied highlight and accent transfer matrices");
        }
    }
    const auto small = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 64});
    const auto medium = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 80});
    const auto between = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 72});
    const auto large = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 120});
    expect(near(small.faceColorMatrixMaxLumaSDR, .6f) && near(medium.faceColorMatrixMaxLumaSDR, .5f)
        && near(large.faceColorMatrixMaxLumaSDR, .35f), "captured small,medium,large face luminance differs by native material size");
    for (auto field : nativeGlassScalarFields)
        expect(near(between.*field, std::lerp(small.*field, medium.*field, .5f)),
            "every numeric input interpolates across measured shortest dimensions");
    expect(between.context.width == 280 && between.context.height == 80,
        "interpolation identifies its nearest real capture instead of inventing observed geometry");
    const auto outside = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 240});
    const auto end = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {280, 160});
    for (auto field : nativeGlassScalarFields)
        expect(outside.*field == end.*field, "geometry outside the measured range clamps without extrapolating optical values");
    const auto interpolatedFill = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .6f, {44, 44});
    expect(near(interpolatedFill.faceColorMatrixFillColor.r, .025f)
        && near(interpolatedFill.faceColorMatrixFillColor.a, .1f),
        "between measured anchors face-fill RGB and alpha remain independent filter coefficients");
    const auto originalQuarter = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .25f);
    const auto originalThreeQuarter = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .75f);
    expect(near(originalQuarter.faceColorMatrixFillColor.r, .0625f) && originalQuarter.faceColorMatrixFillColor.a == 0
        && near(originalThreeQuarter.faceColorMatrixFillColor.r, .0625f) && near(originalThreeQuarter.faceColorMatrixFillColor.a, .25f),
        "original fixture interpolation agrees with later measured native quarter preferences");
    const auto quarter = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .25f, {44, 44});
    const auto threeQuarter = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .75f, {44, 44});
    expect(near(quarter.faceColorMatrixFillColor.r, .0625f) && quarter.faceColorMatrixFillColor.a == 0
        && near(threeQuarter.faceColorMatrixFillColor.r, .0625f) && near(threeQuarter.faceColorMatrixFillColor.a, .25f),
        "measured intermediate tint anchors preserve native hidden RGB and partially opaque fill independently");
    expect(near(nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::light, .25f, {44, 44}).faceColorMatrixFillColor.a, .1f)
        && near(nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::light, .75f, {44, 44}).faceColorMatrixFillColor.a, .35f),
        "light appearance uses measured quarter-preference fill values rather than inferred anchors");
    const auto original = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f);
    const auto empty = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, {});
    expect(original.backdropArchiveSHA256 == empty.backdropArchiveSHA256,
        "default empty size preserves the original fixture and 3-argument behavior");
    for (const NXSize size : {NXSize{-1, 44}, NXSize{44, -1},
            NXSize{std::numeric_limits<float>::quiet_NaN(), 44},
            NXSize{44, std::numeric_limits<float>::infinity()}}) {
        bool rejected = false;
        try { (void)nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f, size); }
        catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected, "size-resolved profile rejects negative or nonfinite geometry");
    }
}

void invalidProgress() {
    const auto profile = nativeGlassProfiles[0];
    for (NXFloat value : {-1.f, 2.f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        bool profileRejected = false, interpolationRejected = false;
        try { (void)nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, value); }
        catch (const std::invalid_argument&) { profileRejected = true; }
        try { (void)interpolateNativeGlassProfiles(profile, profile, value); }
        catch (const std::invalid_argument&) { interpolationRejected = true; }
        expect(profileRejected && interpolationRejected, "native sampling rejects nonfinite values and extrapolation");
    }
}

void measuredDisplayScales() {
    const auto original = nativeGlassProfile(UIGlassEffect::Style::regular, UIUserInterfaceStyle::dark, .5f);
    for (NXFloat scale : {1.f,2.f,2.5f,3.f}) {
        auto profile=original;
        resolveNativeGlassDisplayScale(profile,scale);
        const auto height=scale==1?1.f:scale==2?.5f:scale==2.5f?.64f:1.6f/3;
        expect(near(profile.keyFillHighlightHeight,height)
            && near(profile.keyFillHighlightEffectOffset,-height)
            && near(profile.keyFillHighlightColorBias,scale<=2?-.25f:-.3f),
            "view display scale resolves independently captured native microcontour inputs");
        for(auto field:nativeGlassScalarFields) {
            if(field==&NativeGlassProfile::keyFillHighlightHeight
                ||field==&NativeGlassProfile::keyFillHighlightEffectOffset
                ||field==&NativeGlassProfile::keyFillHighlightColorBias)continue;
            expect(profile.*field==original.*field,"display scale preserves all other native material inputs");
        }
        expect(profile.context.displayScale==original.context.displayScale,
            "scale adaptation retains the real source capture context");
    }
    for(NXFloat scale:{0.f,-1.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        auto profile=original;bool rejected=false;
        try{resolveNativeGlassDisplayScale(profile,scale);}catch(const std::invalid_argument&){rejected=true;}
        expect(rejected,"invalid display scales reject before changing the profile");
        expect(profile.keyFillHighlightHeight==original.keyFillHighlightHeight,
            "invalid scale resolution preserves earlier valid state");
    }
}
}
int main() {
    measuredAnchors();
    interpolationCoverage();
    measuredSizeProfiles();
    invalidProgress();
    measuredDisplayScales();
    if (failures) return 1;
    std::cout << "Native glass profiles preserve 70 inputs, 560 sized material samples, evidence identities, premultiplied colors and discrete modes\n";
    return 0;
}
