> [!WARNING]
> Deeply WIP project!

Main goal is to create a UI framework that tries to mimic [Apple's UIKit](https://developer.apple.com/documentation/uikit/) as much as possible, so any iOS developer will be able to start using it without learning things from scratch and be as much portable as possible supporting various of platforms.

## Current state:
- [Skia](https://github.com/google/skia) powered renderer
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

<img width="936" alt="image" src="https://github.com/user-attachments/assets/13877bc2-c8f4-4449-ac66-fee2c3377a57" />

## TODO:
- UIViewController presentation
- Interface Builder
- A lot of things tbh ...

# Build

First Skia need to be compiled:

### Switch:
```shell
bin/gn gen out/horizon --args='is_official_build=false skia_use_gl=true is_trivial_abi=true target_cpu="arm64" target_os="horizon" is_debug=false'
ninja -C out/horizon skia skparagraph
```

### iOS:
```shell
bin/gn gen out/ios-arm64-angle --args='is_official_build=false target_cpu="arm64" skia_use_gl=true skia_use_metal=true is_trivial_abi=true skia_use_angle=true target_os="ios"'
ninja -C out/ios-arm64-angle skia skparagraph
```

To build project:

### Switch:
```shell
cmake -B build/switch -DPLATFORM_SWITCH=ON
make -C build/switch ThorVGApp.nro -j$(nproc)
nxlink build/switch/ThorVGApp.nro
```

### iOS:
```shell
cmake -B build/ios -GXcode -DPLATFORM_IOS=ON
open xCode project
```
