#include "NativeLensProfile.hpp"
#include "NativeLensProfileData.hpp"
#include "NativeGlassProfileData.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {
int failures = 0;

void expect(bool value, std::string_view reason, std::string_view capture = {}) {
    if (value) return;
    ++failures;
    std::cerr << "FAIL: " << reason;
    if (!capture.empty()) std::cerr << " (" << capture << ')';
    std::cerr << '\n';
}

bool near(NXFloat a, NXFloat b) {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= .00002f;
}

void finiteColor(NativeGlassColor color) {
    expect(std::isfinite(color.r) && std::isfinite(color.g)
        && std::isfinite(color.b) && std::isfinite(color.a), "finite native lens color");
}

void finiteProfile(const NativeLensProfile& profile) {
    for (const auto field : nativeGlassScalarFields)
        expect(std::isfinite(profile.glass.*field), "finite glass coefficient");
    for (const auto field : nativeGlassColorFields)
        finiteColor(profile.glass.*field);
    for (const auto field : nativeGlassHighlightScalarFields)
        expect(std::isfinite(profile.glass.highlight.*field), "finite highlight coefficient");
    finiteColor(profile.glass.highlight.keyColor);
    finiteColor(profile.glass.highlight.fillColor);
    finiteColor(profile.bleedColorMatrixFillColor);
    for (const auto value : profile.glass.highlight.vibrant.matrix.values)
        expect(std::isfinite(value), "finite vibrant matrix");
    for (const auto displacement : {profile.backdrop, profile.contentWrapper, profile.source}) {
        for (const auto value : {displacement.amount, displacement.height, displacement.curvature,
                displacement.angle, displacement.maskOffset, displacement.effectOffset,
                displacement.gradientOvalization})
            expect(std::isfinite(value), "finite displacement coefficient");
    }
    const auto& f = profile.foreground;
    for (const auto value : {f.refractionAmount, f.refractionHeight, f.refractionOffset,
            f.aberrationAmount, f.aberrationHeight, f.aberrationOffset, f.aberrationAngle,
            f.edgeStart, f.edgeEnd, f.edgeOpacityStart, f.edgeOpacityEnd,
            f.output.minimum, f.output.maximum})
        expect(std::isfinite(value), "finite foreground coefficient");
}

void capturedSamples() {
    // Expected values are decoded independently from each original native
    // archive. They are not calculated by the implementation under test.
    for (const auto& sample : nativeLensExpectedSamples) {
        const auto profile = nativeLensProfile(sample.selection, sample.dark,
            {sample.size[0], sample.size[1]});
        const auto compare = [&](NXFloat actual, NXFloat expected, std::string_view field) {
            expect(near(actual, expected), field, sample.capture);
        };
        compare(profile.source.amount, sample.sourceAmount, "source displacement amount");
        compare(profile.source.height, sample.sourceHeight, "source displacement height");
        compare(profile.foreground.aberrationAmount, sample.aberrationAmount, "foreground aberration amount");
        compare(profile.foreground.aberrationOffset, sample.aberrationOffset, "foreground aberration offset");
        compare(profile.foreground.edgeStart, sample.edgeStart, "foreground edge start");
        compare(profile.glass.innerRefractionAmount, sample.innerRefractionAmount, "glass refraction amount");
        compare(profile.glass.innerRefractionHeight, sample.innerRefractionHeight, "glass refraction height");
        compare(profile.glass.shadowAmount, sample.shadowAmount, "glass shadow amount");
        compare(profile.glass.highlight.keySpread, sample.highlightSpread, "key highlight spread");
        compare(profile.glass.highlight.fillSpread, sample.highlightSpread, "fill highlight spread");
        finiteProfile(profile);
    }
    expect(nativeLensExpectedSamples.size() == 72, "all native lifted cases present");
}

void variantState() {
    for (bool selection : {false, true}) for (bool dark : {false, true}) {
        const auto p = nativeLensProfile(selection, dark, {116, 56});
        expect(near(p.backdrop.amount, 9) && near(p.backdrop.height, 36), "native below-glass warp settings");
        expect(near(p.contentWrapper.amount, -17.5f) && near(p.contentWrapper.height, 11.2f),
            "content wrapper retains fixed native warp settings");
        expect(p.backdrop.hasGradientOvalization && p.contentWrapper.hasGradientOvalization
            && near(p.backdrop.gradientOvalization, .5f) && near(p.contentWrapper.gradientOvalization, .5f),
            "ovalized direct SDF gradients retained");
        expect(!p.source.hasGradientOvalization, "portal gradient availability remains explicit");
        expect(near(p.backdropBlurRadius, 0), "lifted lens keeps zero backdrop blur");
        expect(near(p.foreground.captureMargin,100), "native foreground capture margin retained");
        expect(!p.hasGlassClamp && !p.hasGlassClampPreserveHue,
            "absent native lens clamp settings are not asserted as captured");
        expect(near(p.glass.clamp, 1) && !p.glass.clampPreserveHue, "missing clamps have neutral portable fallbacks");
        expect(near(p.bleedColorMatrixFillColor.a, 0), "additional native bleed-fill input retained");
        expect(near(p.glass.blurRadius, 0) && near(p.glass.faceOpacity, 1), "sharp native glass body retained");
        expect(near(p.glass.faceBlack, selection ? 0 : .1f), "small-variant body lift retained");
        expect(p.glass.bleedDarkenBlend == !dark, "native theme-specific bleed flag retained");
        expect(near(p.glass.ringShadowOpacity, selection ? .1f : (dark ? .4f : .15f)),
            "native style/theme contact shadow retained");
        expect(p.glass.captureName.find("lift1") != std::string_view::npos
            && p.glass.backdropArchiveSHA256.size() == 64 && p.glass.layerSourceSHA256.size() == 64,
            "native capture provenance retained");
        expect(near(p.foreground.refractionAmount, 0) && near(p.foreground.refractionHeight, 0)
            && near(p.foreground.aberrationHeight, 0), "zero-height native foreground inputs preserved as data");
        expect(near(p.foreground.edgeOpacityStart, 1) && near(p.foreground.edgeOpacityEnd, 0),
            "native edge visibility endpoints retained");
        expect(near(p.spec.lensHangTime, .22f), "native lens hang time");
        expect(near(p.spec.liftedDisplacement, 9) && near(p.spec.contentWrapperDisplacement, -17.5f),
            "native variant warp settings");
        expect(near(p.spec.unliftedDisplacement, selection ? 0 : 50)
            && near(p.spec.unliftedBlurRadius, selection ? 0 : 6), "native unlifted variant settings");
        expect(near(p.spec.innerShadowOffsetY, 7) && near(p.spec.innerShadowOpacity, .06f)
            && near(p.spec.innerShadowRadius, 3), "native inner shadow spec");
        expect(near(p.spec.liftSpring.response, selection ? .25f : .27f)
            && near(p.spec.liftSpring.dampingRatio, selection ? 1 : .625f)
            && near(p.spec.liftSpring.retargetImpulse, selection ? 0 : .032f), "native lift spring");
        expect(near(p.spec.unliftSpring.response, selection ? .25f : .5f)
            && near(p.spec.unliftSpring.dampingRatio, selection ? 1 : .7f)
            && near(p.spec.unliftSpring.retargetImpulse, selection ? 0 : .016f), "native unlift spring");
        expect(p.spec.liftSpring.behaviorType == 2 && p.spec.unliftSpring.behaviorType == 2
            && !p.spec.liftSpring.smoothingAndProjectionEnabled
            && !p.spec.unliftSpring.smoothingAndProjectionEnabled, "native spring mode flags");
        expect(near(p.spec.liftSpring.trackingResponse, .15f)
            && near(p.spec.liftSpring.trackingDampingRatio, .86f), "native tracking spring settings");
    }
    auto altered = nativeLensProfile(true, false, {116, 56});
    altered.glass.faceBlack = 99;
    altered.foreground.aberrationAmount = 99;
    const auto fresh = nativeLensProfile(true, false, {116, 56});
    expect(near(fresh.glass.faceBlack, 0) && fresh.foreground.aberrationAmount != 99,
        "returned profiles are independent values");
}

void geometryBoundaries() {
    for (bool selection : {false, true}) for (bool dark : {false, true}) {
        for (const auto size : {NXSize{}, NXSize{0, 56}, NXSize{116, 0}, NXSize{.001f, .002f},
                NXSize{56, 116}, NXSize{90, 90}, NXSize{160, 160},
                NXSize{std::numeric_limits<NXFloat>::max(), std::numeric_limits<NXFloat>::max()}})
            finiteProfile(nativeLensProfile(selection, dark, size));
        const auto zero = nativeLensProfile(selection, dark, {});
        expect(near(zero.foreground.edgeStart, 0) && near(zero.source.height, 0)
            && near(zero.glass.innerRefractionHeight, 0), "empty geometry has finite zero optical distances");
        const auto wide = nativeLensProfile(selection, dark, {116, 56});
        const auto tall = nativeLensProfile(selection, dark, {56, 116});
        expect(near(wide.source.amount, tall.source.amount)
            && near(wide.foreground.edgeStart, tall.foreground.edgeStart)
            && near(wide.glass.innerRefractionHeight, tall.glass.innerRefractionHeight),
            "shortest-side rules work for tall and wide lenses");
    }
    const auto nan = std::numeric_limits<NXFloat>::quiet_NaN();
    const auto inf = std::numeric_limits<NXFloat>::infinity();
    for (const auto invalid : {NXSize{-1, 56}, NXSize{116, -1}, NXSize{nan, 56},
            NXSize{116, nan}, NXSize{inf, 56}, NXSize{116, inf}}) {
        bool rejected = false;
        try { (void)nativeLensProfile(true, false, invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected, "invalid native lens geometry rejects");
    }
}
} // namespace

int main() {
    capturedSamples();
    variantState();
    geometryBoundaries();
    if (failures) return 1;
    std::cout << "Native lens profiles:72 native samples, variant specs and geometry boundaries passed\n";
    return 0;
}
