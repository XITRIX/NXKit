> [!WARNING]
> Deeply WIP project!

Main goal is to create a UI framework that tries to mimic [Apple's UIKit](https://developer.apple.com/documentation/uikit/) as much as possible, so any iOS developer will be able to start using it without learning things from scratch and be as much portable as possible supporting various of platforms.

## Current state:
- [Skia](https://github.com/google/skia) powered renderer
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

# Build

Initialize the dependencies first:

```shell
git submodule update --init --recursive
```

First Skia need to be compiled:

### Switch:
```shell
bin/gn gen out/horizon --args='is_official_build=false skia_use_gl=true is_trivial_abi=true target_cpu="arm64" target_os="horizon" is_debug=false'
ninja -C out/horizon skia skparagraph
```

### iOS:
```shell
bin/gn gen out/ios-arm64 --args='is_official_build=false target_cpu="arm64" skia_use_gl=true skia_use_metal=true is_trivial_abi=true target_os="ios" ios_min_target="15.0"'
ninja -C out/ios-arm64 skia skparagraph
```

### macOS:
```shell
bin/gn gen out/mac-arm64 --args='is_official_build=false target_cpu="arm64" skia_use_gl=true skia_use_metal=true is_trivial_abi=true'
ninja -C out/mac-arm64 skia skparagraph
```

To build project:

### Switch:
```shell
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON
cmake --build Demo/build/switch --target ThorVGApp.nro -j
nxlink Demo/build/switch/ThorVGApp.nro
```

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
