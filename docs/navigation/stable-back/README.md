# Stable automatic Back identity — 2026-11-03

Each navigation bar owns one automatic Back item for its lifetime, with the
internal identifier `UIKit.navigationBar.back`. Push/pop and interactive preview
reuse that semantic identity. Ordinary application items cannot match automatic
Back by copying its identifier: automatic/custom roles must agree before an
identifier can retain a control. Interactive matching applies the same rule.

When the leading configuration is unchanged, Back retains its item, control,
ink and material objects. Its position, size, transform, opacity and sharp ink
remain unchanged while titles, content and trailing items transition. Introducing
or removing supplemental leading items still uses the existing UIKit-calibrated
glass split/join; this change does not suppress those actual layout transitions.

`stableBack` in `UIKit/tests/UINavigationTests.cpp` exercises 144 automatic
push/pop transitions: phone/compact layouts × 1×/2×/3× × light/dark × Back alone/
unchanged supplemental item × three pushes and pops. Each transition samples
133 presentation poses at 120 Hz, plus its immediate and settled states. Every
sample checks object/identifier continuity, exact stationary material/control
geometry, identity transforms, opacity and zero ink blur. Twenty-four additional
custom-item collision cases verify separate controls, correct actions and the
restored automatic Back identity.

`InteractiveNavigationTests` also checks unchanged Back identity and geometry
through tracking, reversal, commitment, cancellation and disabling mid-gesture.
Both navigation suites pass. The earlier [native cycle comparison](../cycle/README.md)
establishes stationary Back in 200 unchanged-leading clips; the new identifier
does not change those layouts.

Current build, catalog and strict GPU verification results are recorded in
[verification.json](verification.json). UIKit library compilation and both
UIKit-only macOS test executables pass. The final full Demo rebuild attempts
encountered concurrent corner-API edits: `UICornerConfiguration.h:9,13` uses
copy-list returns with an explicit constructor. Those edits were left untouched;
the full app builds must be repeated after that work compiles. GPU verification
uses the completed navigation snapshot and newly baked catalog, independently of
those subsequent edits. The preceding interactive-pop report's
hashes and captures remain historical evidence for that revision. Physical-device
execution of this follow-up is unverified.

The navigation snapshot's freshly baked macOS/iOS archives contain 68 shader
libraries and 54 native pipeline descriptions. A fresh current capture observes
66 libraries and 53 descriptions, all present in that retained raw-state superset.
Strict macOS runs cover 34,994 full-screen automatic/interactive frames in dark
3× and light 2×, with zero runtime library or pipeline compilations. Raw counters,
test logs, archive/source hashes and the full-build error summaries are retained
under [verification](verification/).
