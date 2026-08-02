# NXKit development guide

This file is the standing guide for agents and contributors working in this repository. Read it before making changes.

## Project goal

NXKit is a C++20, cross-platform reimplementation of the portable core of UIKit. It should feel familiar to UIKit developers while remaining practical on platforms where Objective-C, Swift, and Apple's frameworks are unavailable.

The goal is behavioral compatibility for common application UI, not binary or source compatibility with Apple's UIKit. Prefer UIKit names and semantics when they translate cleanly to C++; document intentional C++ or platform-neutral differences.

Current supported platforms are:

- macOS
- iOS
- Nintendo Switch through devkitPro/libnx

Android, Windows, and Linux are desired future targets but are not implemented.

## Repository structure

- `UIKitCore/` contains the portable UIKit-style API, rendering, layout, events, controllers, and platform backends. New UIKit-compatible functionality belongs here.
- `NXKit/` contains opinionated or product-specific components such as the current console-oriented navigation and tab controllers. Do not treat these as UIKit-compatible implementations unless their behavior and public API are brought into parity.
- `Demo/` is the executable smoke-test and visual playground.
- `Submodules/` contains SDL, Skia, Yoga, TinyXML2, fmt, vcpkg, and platform support dependencies. Avoid unrelated submodule changes.
- `UIKIT_API_PROGRESS.md` is the required compatibility and roadmap tracker.

The core CMake target is currently `XITRIXUIKit`, with alias `Xitrix::UIKit`; the extension target is `NXKit`. The namespace is `NXKit`. This naming is existing technical debt: do not introduce another project name.

## Architecture

- SDL 3 owns window creation, lifecycle events, touch/mouse/gamepad input, and the platform surface.
- Skia Graphite owns GPU rendering. Apple platforms use Graphite's native Metal backend; Switch uses Graphite through Dawn's OpenGL ES compatibility backend.
- SkParagraph plus ICU-backed SkUnicode handles text shaping and layout.
- `CALayer` is the retained rendering and animation tree.
- `UIView` owns a backing layer and participates in layout, responder, gesture, trait, and focus systems.
- Yoga is NXKit's cross-platform layout engine. It is an intentional alternative to UIKit Auto Layout, not an implementation of `NSLayoutConstraint`.
- TinyXML2 powers NXKit's custom `UINib` XML dialect. It is not compatible with Apple nib or storyboard files.
- Platform-specific renderer code belongs under `UIKitCore/lib/platforms/`; portable behavior must stay outside platform folders.

## Authorities and references

Use sources in this order:

1. [Apple UIKit documentation](https://developer.apple.com/documentation/uikit) and observable UIKit behavior define the compatibility target.
2. [flowkey/UIKit-cross-platform](https://github.com/flowkey/UIKit-cross-platform) is an implementation and test reference, especially for view hierarchy, geometry, animation, controls, and Android integration. It is not authoritative and contains deliberate deviations and incomplete behavior.
3. Official Skia, SDL, Yoga, and platform documentation define backend usage.
4. Existing NXKit behavior is a compatibility constraint only when changing it would unnecessarily break current users. Correctness and a documented migration path take priority over preserving accidental behavior.

When borrowing an algorithm or porting code, check its license, preserve required attribution, and adapt it to NXKit's architecture rather than copying assumptions tied to Swift ARC or SDL_gpu.

## Compatibility rules

- Reproduce semantics, state transitions, lifecycle ordering, defaults, and edge cases—not just UIKit class names.
- Keep application-specific assets, strings, dimensions, and navigation behavior out of `UIKitCore`.
- Do not expose a public API until its behavior is usable. If a necessary API must land incrementally, mark it clearly as incomplete and update `UIKIT_API_PROGRESS.md`.
- A declaration with an empty implementation is a stub, not implemented functionality.
- Prefer portable UIKit concepts. Do not add iOS-only shell APIs unless a supported platform genuinely needs them.
- Maintain familiar hierarchy and ownership behavior: views own subviews and layers; parents, delegates, and back-references should normally be weak.
- UI state is main-thread/main-queue confined. New asynchronous work must marshal UI mutations back to `DispatchQueue::main()`.
- New public headers should not include private Skia source headers or expose backend types unless the API is explicitly a low-level rendering extension.
- Keep `UIKitCore` independently usable; optional NXKit components may depend on it, never the reverse.

## Current C++ conventions

- Use C++20.
- Use `NXFloat`, `NXPoint`, `NXSize`, `NXRect`, `NXAffineTransform`, and `NXTransform3D` until the public naming strategy is changed deliberately.
- Follow the existing getter plus `set...` style.
- Use `std::shared_ptr` for ownership and `std::weak_ptr` for delegates, parents, superviews, and other non-owning back-references. Avoid owning raw pointers.
- Existing UI classes use `new_shared<T>()`, including during construction. Do not allocate a class derived from NXKit's custom `enable_shared_from_this` on the stack or with `std::make_shared`; that creates unsafe competing ownership. Long term, replace `tools/SharedBase.hpp` with standard `std::enable_shared_from_this` and two-phase/factory initialization.
- Avoid calling user callbacks while invariants are temporarily broken or collections are being mutated.
- Validate indices, nullable values, finite geometry, and hierarchy cycles at public API boundaries.
- Keep headers self-contained and add new public headers to the relevant umbrella header and CMake target.
- Register XML-instantiable views and attributes when adding them; unsupported attributes should fail visibly rather than silently doing nothing.

## Definition of done

For a behavioral or API change:

1. Compare the intended behavior with Apple UIKit documentation and, when useful, UIKit-cross-platform tests.
2. Implement the full vertical path: public API, state, rendering/layout/event behavior, invalidation, and platform integration.
3. Add focused automated tests when a test target exists. Until then, add the smallest useful regression harness or Demo case and document manual verification.
4. Check animation and presentation-layer behavior for visual properties.
5. Check traits, scale factors, safe areas, input modes, empty content, and removal/reparenting where relevant.
6. Build the touched platform when its SDK is available. Do not claim a successful build when required prebuilt Skia archives are absent.
7. Update `UIKIT_API_PROGRESS.md` in the same change.

Do not mark an API as usable solely because it compiles. Usable means its core application behavior works without a known correctness blocker.

## Maintaining the API progress tracker

`UIKIT_API_PROGRESS.md` must stay synchronized with the public API and behavior.

Every change that adds, removes, renames, fixes, or materially changes a UIKit-facing API must update the tracker in the same patch. This includes changes that turn a stub into working behavior or reveal a previously unknown limitation.

When updating it:

- Update the audit date and NXKit revision or write `working tree after <revision>` when the change is not committed yet.
- Change a status only after checking the implementation, not from the filename or declaration alone.
- Add or update the evidence/remaining-work note.
- Add newly discovered portable UIKit APIs in priority order.
- Keep genuinely iOS-specific APIs in the out-of-scope section instead of expanding the core roadmap.
- If behavior deliberately differs from UIKit, record it under intentional divergences.
- If an API is implemented but untested, keep it `Partial` unless its behavior has been verified thoroughly through the Demo or another reproducible harness.

Before finishing any UIKit-facing task, run:

```sh
git diff --check
git diff -- AGENTS.md UIKIT_API_PROGRESS.md UIKitCore NXKit Demo
```

## Build and verification

Initialize dependencies with:

```sh
git submodule update --init --recursive
```

Skia must currently be built manually into the architecture-specific directories documented in `README.md`. The macOS smoke-build flow is:

```sh
cmake -S Demo -B Demo/build/macos -DPLATFORM_DESKTOP=ON
cmake --build Demo/build/macos --target ThorVGApp -j
```

Equivalent iOS and Switch commands are in `README.md`. CMake configuration intentionally fails if the expected Skia archive is missing; report that as an unmet prerequisite, not a successful source build.

There is currently no first-party automated test suite or CI. Adding a small, fast core test target—beginning with geometry, ownership, hierarchy, event delivery, animation, controls, and scroll behavior—is a higher priority than broad API expansion.

## Known high-priority hazards

Confirm these areas before building on them:

- `tools/SharedBase.hpp` has a dangerous custom ownership scheme that requires `new_shared` and should be replaced.
- `CALayer::mask` is stored but not applied by `skiaRender`.
- Some visual setters do not dirty the layer tree, so updates may be deferred until another mutation.
- `UIScrollView::contentSize()` is inferred from only the first subview; indicators and parts of delegate delivery are incomplete.
- `UIControlEvent` lacks a real target/action registry, and control states do not model UIKit option-set semantics.
- `NXNavigationController` and `NXTabBarController` are console/demo components, not UIKit-compatible container implementations.
- Public headers leak Skia implementation details.
- Debug `printf` calls remain in frame and text paths.

Fix correctness and truthful API behavior before adding breadth on top of these areas.
