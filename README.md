> [!WARNING]
> Deeply WIP project!

Main goal is to create a UI framework that tries to mimic [Apple's UIKit](https://developer.apple.com/documentation/uikit/) as much as possible, so any iOS developer will be able to start using it without learning things from scratch and be as much portable as possible supporting various of platforms.

## Current state:
- [Skia Graphite](https://skia.org/docs/user/api/graphite/) powered renderer
- SDL 3.4 platform integration, including the UIScene lifecycle on iOS and the
  devkitPro libnx backend on Nintendo Switch
- Platforms supported:
  - iOS
  - macOS
  - Nintendo Switch ([LibNX](https://github.com/switchbrew/libnx))
- Core implementation:
  - CALayer (rendering hierarchy)
  - [Yoga](https://github.com/facebook/yoga) flex auto layout
  - Animation system
  - Responder chain (touch propagation)
  - Focus system (basic gamepad input support)
  - UITraitCollection (dark mode)
 
<img width="936" alt="image" src="https://github.com/user-attachments/assets/0c32631c-6d6d-4785-836c-430e8345a0fb" />
<details>
<summary>More screenshots</summary>
<img width="936" alt="image" src="https://github.com/user-attachments/assets/13877bc2-c8f4-4449-ac66-fee2c3377a57" />
</details>
  
## TODO:
- UIViewController presentation
- Interface Builder
- A lot of things tbh ...

## Modules

`UIKitCore` provides style-independent views, controllers, controls, text/images,
scrolling, input/focus, layout, animation, generic rendering, and platform backends.
`UIKit` adds Apple-styled components and glass materials; `NXKit` adds
Nintendo-styled controls, navigation, tabs, and controller hints. Both component
kits depend directly on Core and can be used together on every supported platform.

Include `<UIKitCore.h>`, `<UIKit.h>`, or `<NXKit.h>` and link the corresponding
`Xitrix::UIKitCore`, `Xitrix::UIKit`, or `Xitrix::NXKit` target. `XITRIXUIKit`
remains the underlying UIKit target to avoid an Xcode name collision with the
native UIKit framework. Optional kits can be disabled with
`UIKIT_BUILD_UIKIT=OFF` and `UIKIT_BUILD_NXKIT=OFF` when embedding the repository.
The Demo requires both kits. UIKit XML components require `registerUIKitViews()`
before inflation; Core registers only its own views.

See [module boundaries and migration](docs/module-boundaries.md) for the component
inventory, shared controller contracts, API differences, and next steps.

# Build

Initialize the dependencies first:

```shell
git submodule update --init --recursive
```

Skia must first be compiled with Graphite. Apple platforms use Graphite's native
Metal backend; Nintendo Switch uses Graphite's native Vulkan backend and the
statically linked Horizon NVK driver. Dawn is not part of the Switch renderer.

### Switch:
```shell
# Run from the NXKit repository root:
python3 scripts/switch/fetch_nvk.py
sh scripts/switch/build_skia.sh
```

The wrapper uses Skia's checked-in `tools/build-horizon.sh`. Set `DEVKITPRO` to
a complete SDK with Switch portlibs (verification uses `/opt/devkitpro`). See
[the Skia upgrade and recovery notes](docs/skia-upgrade/README.md) for the upstream
base, the single Switch-support commit, and preserved local changes. Rebuild
platform-specific Skia archives whenever updating its headers.

Apple builds also require NXKit's maintained Graphite cache/sampling patch.
From the repository root, run `python3 scripts/apple/apply_skia_patches.py`
before rebuilding the macOS/iOS Skia archives below. The Demo now compiles its
standard Metal shader catalog and GPU pipelines during the app build; install
Apple's compiler with `xcodebuild -downloadComponent MetalToolchain` if needed.
See [Metal shader baking and verification](docs/navigation/pipeline-cache/README.md).

### iOS:
```shell
bin/gn gen out/ios-arm64 --args='is_official_build=false target_cpu="arm64" target_os="ios" ios_min_target="15.0" skia_enable_graphite=true skia_enable_ganesh=false skia_use_gl=false skia_use_metal=true is_trivial_abi=true is_debug=false skia_use_partition_alloc=false'
ninja -C out/ios-arm64 skia skparagraph
```

### macOS:
```shell
bin/gn gen out/mac-arm64 --args='is_official_build=false target_cpu="arm64" skia_enable_graphite=true skia_enable_ganesh=false skia_use_gl=false skia_use_metal=true is_trivial_abi=true is_debug=false skia_use_partition_alloc=false'
ninja -C out/mac-arm64 skia skparagraph
```

Keep `skia_use_partition_alloc=false` for these embedded Apple builds. NXKit's
CMake targets use Skia's ordinary pointer implementation, without Chromium's
allocator headers or `SK_USE_PARTITION_ALLOC` definition. Build Skia and its
consumers with matching release and trivial-ABI settings.

To build project:

### Switch:
```shell
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON
cmake --build Demo/build/switch --target ThorVGApp.nro -j
nxlink Demo/build/switch/ThorVGApp.nro
```

The NVK SDK is pinned and checksum-verified inside `Submodules/nvk-switch`;
it does not replace the system devkitPro installation. Switch Skia archives
live in `Submodules/skia/out/horizon-vulkan`. Both paths can be overridden with
`NXKIT_SWITCH_NVK_ROOT` and `NXKIT_SWITCH_SKIA_OUT`.

To compile the Demo's GPU pipelines ahead of interactive use, leave the Switch
in an nxlink-capable homebrew launcher with FTP enabled, then run:

```sh
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON \
    -DNXKIT_SWITCH_ADDRESS=192.168.1.148
export NXKIT_SWITCH_FTP_USER=your-ftp-user
export NXKIT_SWITCH_FTP_PASSWORD='your-ftp-password'
cmake --build Demo/build/switch --target ThorVGApp.switch-shaders -j8
```

This target compiles pipelines on the Switch GPU, retrieves the native Vulkan
cache, and rebuilds the NRO with that cache bundled. Its distributable output
is `Demo/build/switch/switch-package/`. Later ordinary NRO builds embed an
available matching cache automatically. Shader/Skia/driver changes invalidate
the cache and require another bake. Runtime cache growth is saved on orderly
exit, without SD writes during frame submission.

Skia generates pipeline combinations dynamically; standalone shader-source
compilation cannot cover arbitrary application effects or scene graphs. The
bake fixture covers the Demo gallery, grouped glass, lifted/resting lens
states, stress screen, tabs, text and XML screen. New effects or combinations
must be exercised by the fixture. See [Switch Vulkan and shader compilation](docs/switch-vulkan/README.md)
for architecture, cache compatibility, validation and remaining limitations.

### iOS:
```shell
cmake -S Demo -B Demo/build/ios -G Xcode -DPLATFORM_IOS=ON
open Demo/build/ios/ThorVGApp.xcodeproj
```

### macOS:
```shell
cmake -S Demo -B Demo/build/macos -DPLATFORM_DESKTOP=ON
cmake --build Demo/build/macos --target ThorVGApp -j
```
