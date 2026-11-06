# Calibrated material verification

Source ABI: `4b541699bc48606e83a9699c093df049dfc09c5a03d25c847d0c50b6fc34ba91`.

`summary.json` enumerates 24 strict profiles / 43,416 frames. Every row's library
and pipeline compilation counters is zero. The writable Metal cache is excluded
by `NXKIT_METAL_BUNDLED_ONLY=1`; `NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1` forbids runtime
compilation. Edge profiles also expose a zero `failures` counter. Native descriptor
and raw-source validation checks the cold catalog before rebuilding GPU binaries.

Coverage: six 975-frame edge profiles, two 17,497-frame navigation catalogs,
ten 64-frame fixed-preference profiles, six 322-frame continuous-preference
profiles. Navigation covers all eight arrangements, push/pop and interactive
completion/cancellation. Continuous sweeps exercise 161 values in both themes
at 1×/2×/3×. Strict archives come from the final normal macOS app bundle.

All `.csv.gz` files contain exact per-frame results. `.log.gz` files retain
benchmark/build output. Device console logs are separate smoke evidence: final
signed app, iPhone 15 Pro Max/iOS 27.2, prebuilt archive only, normal and strict
stress replays, no runtime pipeline compilation/shader miss reported. The existing
SDL root-controller appearance-transition warning is unrelated to this material
comparison and remains visible in the unedited logs. No device frame-count,
full physical native reference matrix or temperature claim is inferred from
these console logs. The app was returned to normal launch afterward.
