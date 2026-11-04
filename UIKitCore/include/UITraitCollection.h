#pragma once

#include <memory>
#include <Geometry.h>
#include <UIUserInterfaceSizeClass.h>

namespace NXKit {

enum class UIUserInterfaceStyle {
    unspecified,
    light,
    dark
};

class UITraitCollection {
public:
    std::shared_ptr<UITraitCollection> copy();

    /** Portable extension: Liquid Glass preference, from clear (0) to tinted
     (1). Platforms without a system preference use the regular midpoint (0.5).
     Collections inherit the application-resolved value. */
    static constexpr NXFloat defaultGlassTintAmount = 0.5f;
    NXFloat glassTintAmount() const { return _glassTintAmount; }
    static std::shared_ptr<UITraitCollection> traitCollectionWithGlassTintAmount(NXFloat amount);

    UIUserInterfaceStyle userInterfaceStyle() const { return _userInterfaceStyle; }
    NXFloat displayScale() const { return _displayScale; }
    UIUserInterfaceSizeClass horizontalSizeClass() const { return _horizontalSizeClass; }
    UIUserInterfaceSizeClass verticalSizeClass() const { return _verticalSizeClass; }

    static std::shared_ptr<UITraitCollection> current() { return _current; }
    static void setCurrent(std::shared_ptr<UITraitCollection> current) { _current = current; }

private:
    friend bool applicationRunLoop();
    friend class UIWindow;
    friend class GlassMaterialTestHarness;
    friend class GlassAppearanceTestHarness;
    friend class GlassContainerTestHarness;
    friend class BackdropEffectView;

    static std::shared_ptr<UITraitCollection> _current;

    UIUserInterfaceStyle _userInterfaceStyle = UIUserInterfaceStyle::unspecified;
    NXFloat _glassTintAmount = defaultGlassTintAmount;
    NXFloat _displayScale = 1;
    UIUserInterfaceSizeClass _horizontalSizeClass = UIUserInterfaceSizeClass::unspecified;
    UIUserInterfaceSizeClass _verticalSizeClass = UIUserInterfaceSizeClass::unspecified;
};

}
