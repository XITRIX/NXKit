# Native iOS 26 scroll-edge reference

These captures run Apple's UIKit on **iOS 26.0 release, build 23A343**, on a dedicated iPhone 16 Pro simulator. They are not iOS 27 captures. The fixture refuses other major versions. The 49-case matrix and paired profiles use `UIGraphicsImageRenderer` plus `drawViewHierarchy(afterScreenUpdates:NO)` at scale 2, with a logical window of 402 × 874 points. Three representative held states were also captured from the actual simulator framebuffer at scale 3; the methods and cross-checks are recorded below. The app uses the installed SDK with deployment target 26.0; runtime behavior is supplied by the recorded 26.0 installation.

[Light contact sheet](light-contact.png), [dark contact sheet](dark-contact.png), [variable-blur mask probe](variable-blur-mask-contact.png), [runtime metadata](runtime-api.json), [state/layer traces](states.json.gz).

## Public API observations

The installed Apple headers define the API below; native runtime behavior is identified separately where it differs. Primary references are Apple’s [UIScrollEdgeEffect](https://developer.apple.com/documentation/uikit/uiscrolledgeeffect), [UIScrollEdgeEffectStyle](https://developer.apple.com/documentation/uikit/uiscrolledgeeffectstyle), and [UIScrollEdgeElementContainerInteraction](https://developer.apple.com/documentation/uikit/uiscrolledgeelementcontainerinteraction). The headers inspected are the installed UIKit `UIScrollView.h`, `UIViewController.h` and `UIScrollEdgeElementContainerInteraction.h`.

- `UIScrollEdgeEffectStyle.automaticStyle`, `.softStyle`, `.hardStyle`. The hard-style header describes a hard cutoff and dividing line.
- `UIScrollEdgeEffect.style` and `.hidden`; all four newly created effects are `.automatic`, with `hidden == false`.
- `UIScrollView.topEdgeEffect`, `.leftEdgeEffect`, `.bottomEdgeEffect`, `.rightEdgeEffect` are readonly effect objects.
- `UIViewController.setContentScrollView(_:forEdge:)` accepts directional edge flags, including aggregates. Passing nil restores the framework's automatic search for that edge. `contentScrollViewForEdge(_:)` returns the explicit value; the installed header documents one edge, while the iOS 26.0 runtime probe accepts only top/bottom and throws for leading, none, aggregates or unknown values; it does **not** return the result of automatic discovery. In the captured automatic cases the getter returns nil; in the manually bound case it returns the configured scroll view.
- The public `UIScrollEdgeElementContainerInteraction` associates custom overlay chrome with a scroll view and an edge. Its `scrollView` is weak and nullable (default nil); `edge` uses `UIRectEdge` and defaults to none. The native setter roundtrips none, individual edges, aggregates and even unknown bits without eager validation; see [API probes](public-api-probes.json). Its header states that labels, images, glass and controls in the container affect the edge effect's shape. Merely adding 32-point lateral safe-area insets did not produce side carriers in the initial reference; separate fixtures add explicit left/right chrome interactions.

The fixture's first-child obstruction case is recorded, but the appearance of a scroll view's own edge effect does not prove which scroll view the containing navigation controller's heuristic chose. The precise first-child heuristic and a subclass override returning nil are therefore not established by these screenshots alone.

## Native private variable blur

Read-only Objective-C runtime metadata establishes the owning class and signatures on iOS 26.0:

```objc
@interface UIBlurEffect (PrivateObserved)
+ (instancetype)effectWithVariableBlurRadius:(double)radius imageMask:(UIImage *)mask;
+ (instancetype)_effectWithVariableBlurRadius:(double)radius imageMask:(UIImage *)mask scale:(double)scale;
+ (instancetype)_effectWithVariableBlurRadius:(double)radius imageMask:(UIImage *)mask scale:(double)scale allowingAXAdaptation:(BOOL)allow;
@end
```

The private implementation class is `_UIBlurEffectVariableImpl`, with `-initWithBaseRadius:imageMask:scale:allowAXAdaptation:`. The `_UIProgressiveBlur…` classes found in this runtime are context/presentation controllers and animators, rather than a `UIProgressiveBlurEffect` class. These signatures are private implementation evidence; they are not supported public Apple API.

The fixture invokes only the first factory, in an isolated comparison app, with radius 24 and a 2 × 256 alpha image stretched across a 354 × 256 effect view. Transparent mask pixels preserve crisp content, opaque pixels blur, and a transparent-to-opaque ramp produces spatially increasing blur. A nil mask raises the native exception `UIImage ((null)) returned NULL from -CGImage.` See [mask comparison](variable-blur-mask-contact.png) and [exception](nil-exception.json). This validates image-alpha semantics and normalized mask placement, but does not establish Apple's exact Gaussian kernel, radius mapping, or accessibility adaptations.

A bounded [radius calibration](private-radius-calibration.json) compares the native radius 24 opaque-mask output to Gaussian filtering of the native transparent-mask source. In the selected inner ROI, a Pillow Gaussian sigma around 44–46 points has RGB error about 3.0–3.1, while sigma 24 has error about 20.2. This is evidence that the private radius is not interchangeable with a portable Gaussian sigma. It does not establish a universal 1.8 multiplier across radii, scales, content or accessibility settings.

## Geometry and optical observations

Automatic and soft styles have the same qualitative appearance in the reference navigation/tab configuration. At the middle offset, the scroll view has safe-area insets `(top:116,left:32,bottom:83,right:32)`. The native soft top carrier occupies window `(0,0,402,170.8)`; the bottom occupies `(0,726.2,402,147.8)`. Thus the soft carrier extends inward past the safe area. Hard carriers use a sharp top/bottom boundary and a dividing edge: top height 106 and bottom 83, including at-rest and 1/8-point offsets. Explicit side chrome produces soft left/right carrier widths 85.47/85.8 and hard widths 20.67/21. That difference reflects the descendant footprint plus a 64.8-point soft tail; the 32-point lateral safe area alone is insufficient to predict the native boundary.

The final captures configure edge styles **before** mounting a screen. An initial prototype configured styles after presentation and retained transitional material states in some first snapshots; those images and derived coefficients were replaced. [Measurements](measurements.json) distinguish samples at true tile centers (`x=48+48k` with content offset 120) from samples near tile boundaries (`x=25+48k`). The latter combine fade with substantially more blur mixing and must not be mistaken for direct opacity estimates. Initial analysis used the boundary samples as generic color-transfer measurements; their placement is now explicitly labelled.

Native soft edges darken content; a white alpha fade does not reproduce that behavior. At true tile centers near y=20, the native light slope is about 0.735 with intercept near 2, and the dark slope about 0.395 with intercept near 0. At y=116, light is about 0.92 and dark about 0.824; at y=126, about 0.97/0.93; by y=156 the transformation reaches zero. Full RGB coefficients and near-boundary profiles are preserved. They remain screenshot regressions rather than recovered native mask constants.

Near rest, source colors reconstructed from the same-trait hidden image give first-row contrast slopes 0.9939 at rest, 0.9939 after 1 point, 0.9910 after 8 points, and 1.0 during 24-point overscroll (light soft style, first content row +4 points). The content begins at window y=116/115/108 in those cases, still outside the hard top band ending at 106. This supports measuring penetration into the actual chrome band rather than treating the extra 10-point inset gap as already covered; that threshold interpretation is an inference.

Read-only layer inspection observes `CABackdropLayer` with a `variableBlur` filter (`inputNormalizeEdges = 1`, reported `inputRadius = 1`) plus portal color-matrix stages. The exact native 4×5 portal matrix is RGB identity with alpha scale 1.25 and bias −0.25, in both light and dark ([decoded values](profile/color-matrices.json)). It does not directly explain the trait-dependent darkening; other backdrop/mask/compositing stages remain unresolved. The reported filter radius alone is insufficient to recover the true optical sigma. The [paired light/dark profile](profile/measurements.json) records both regression slope and intercept; near bottom y=860 native output ≈0.75×source in light and ≈0.42×source in dark. Navigation-bar style and controller status-bar style remain default; title textColor remains dynamic labelColor in both traits.

## Transparent content and framebuffer controls

The [sparse profile](sparse-profile/sparse-comparison.png) skips both fill and text for alternate 48-point tiles, makes the drawing view nonopaque with a clear background, and keeps the controller’s system background behind it. Native transparent gaps remain white in light mode (soft-versus-hidden center difference 0–3 RGB) and black in dark mode (difference 0). Colored sparse cells fade toward that background: near the outer top, approximately 0.153×source+213 in light and 0.153×source in dark. Full center measurements are [recorded here](sparse-profile/sparse-measurements.json).

A separate [dense nonopaque control](nonopaque-control/light-soft.png) sets both the Grid view and its layer to nonopaque while filling all tiles. It still matches the dense opaque control’s darkening. Therefore the `opaque` promise alone does not explain sparse-versus-dense behavior. Fresh native defaults are in [API probes](public-api-probes.json): UIView, UIImageView, UIScrollView and Grid have view/layer opaque=true; UILabel has view opaque=true but layer opaque=false. The nonopaque control records both as false. Background/coverage-dependent legibility and material adaptation are plausible explanations, but the selection algorithm remains unresolved. A portable rectangular black underlay must not turn transparent gaps black, and preserving gaps alone does not establish native adaptive-material parity.

The [actual framebuffer contact sheet](framebuffer/contact.png) shows dense, sparse, and dense-nonopaque light-soft states. Each raw framebuffer is 1206 × 2622 pixels (3×), including the OS shell. For a bounded comparison only, these were explicitly downsampled to the 804 × 1748 snapshot grid with Lanczos; the app ROI excluded the top 62 and bottom 34 points. Mean absolute RGB differences against the corresponding UIKit snapshots are 0.553, 0.363 and 0.553 respectively ([cross-check metadata](framebuffer/cross-checks.json)). Thus the dense darkening and sparse whitening are observable on screen, rather than being inferred only from the snapshot API. This cross-check covers three representative light-soft states; it does not certify every matrix case or physical-device rendering.

## Artifact identity and reverse-engineering limits

The runtime UIKitCore artifact is:

```
/Library/Developer/CoreSimulator/Volumes/iOS_23A343/Library/Developer/CoreSimulator/Profiles/Runtimes/iOS 26.0.simruntime/Contents/Resources/RuntimeRoot/System/Library/PrivateFrameworks/UIKitCore.framework/UIKitCore
SHA-256 5158d125875a9a81bc1c9160a70553403281562c0b7faac802c03816cfee9c79
```

REA successfully identified/opened this artifact with the Hopper provider. Deep procedure search hit a provider startup timeout, and its native Mach-O inspection timed out; no successful disassembly/decompilation evidence IDs are claimed. Runtime selectors, defaults, layer geometry and screenshots above come from the shipped framework executing inside the dedicated simulator and from the fixture's recorded read-only runtime metadata. Strings corroborated the variable-blur selector names before the runtime probe completed.

This reference does not cover physical-device rendering, iOS 26 patch-version differences, accessibility adaptations, interactive navigation transitions with effects, or every custom chrome arrangement. It is a behavior/appearance oracle, not a claim of pixel-identical portable rendering.

## Reproduction

```sh
python3 Demo/ScrollEdgeComparison/build-native.py /private/tmp/nxkit-scroll-edge-native
xcrun simctl create 'NXKit Scroll Edges iOS 26' \
  com.apple.CoreSimulator.SimDeviceType.iPhone-16-Pro \
  '/Library/Developer/CoreSimulator/Volumes/iOS_23A343/Library/Developer/CoreSimulator/Profiles/Runtimes/iOS 26.0.simruntime'
# Use only the UUID returned above; never use the ambiguous "booted" target.
xcrun simctl boot DEVICE_UUID
xcrun simctl bootstatus DEVICE_UUID -b
xcodebuild -project /private/tmp/nxkit-scroll-edge-native/Interaction.xcodeproj \
  -scheme Interaction -destination 'platform=iOS Simulator,id=DEVICE_UUID' \
  -derivedDataPath /private/tmp/nxkit-scroll-edge-native/build CODE_SIGNING_ALLOWED=NO build
xcrun simctl install DEVICE_UUID /private/tmp/nxkit-scroll-edge-native/build/Build/Products/Debug-iphonesimulator/InteractionReference.app
xcrun simctl launch DEVICE_UUID org.nxkit.ScrollEdgeReference
# Wait for Documents/scroll-edge-reference/done.json in this app's data container.
python3 Demo/ScrollEdgeComparison/collect-native.py DEVICE_UUID docs/scroll-edge-effects/native
xcrun simctl shutdown DEVICE_UUID
xcrun simctl delete DEVICE_UUID
```

The app self-runs 49 cases (36 style/theme/offset cases, 3 discovery/manual/hidden, 4 mask probes, 6 side-chrome cases). `--api-only` records public defaults/argument behavior without presenting a screen; `--profile-only` records a separate four-case light/dark soft/hidden optical probe in `Documents/scroll-edge-profile`. The app saves full-window PNGs, defaults, explicit-binding results, view frames and layer/filter metadata. The source manifest is generated beside the temporary Xcode project and copied here. The 49-case run predates additional default/matrix instrumentation and opt-in sparse/nonopaque/held-state switches; its default full-grid setup is unchanged. The later coverage and framebuffer controls record their own view/layer state. `--sparse-profile` selects transparent alternate tiles; `--nonopaque-profile` keeps a filled grid but clears its opaque promise; `--case=light-soft` holds a single profile for an external framebuffer screenshot. Pillow is optional for contact-sheet generation.
