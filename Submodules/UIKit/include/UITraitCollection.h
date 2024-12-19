#pragma once

#include <memory>
#include <Geometry.h>

namespace NXKit {

enum class UIUserInterfaceStyle {
    unspecified,
    light,
    dark
};

class UITraitCollection {
public:
    std::shared_ptr<UITraitCollection> copy();

    UIUserInterfaceStyle userInterfaceStyle() const { return _userInterfaceStyle; }
    NXFloat displayScale() const { return _displayScale; }

    static std::shared_ptr<UITraitCollection> current() { return _current; }
    static void setCurrent(std::shared_ptr<UITraitCollection> current) { _current = current; }

private:
    friend bool applicationRunLoop();

    static std::shared_ptr<UITraitCollection> _current;

    UIUserInterfaceStyle _userInterfaceStyle = UIUserInterfaceStyle::unspecified;
    NXFloat _displayScale = 1;
};

}
