#pragma once

#include <Geometry.h>
#include <memory>

namespace NXKit::Internal {

struct IOSWindowFrame {
    NXFloat width = 0, height = 0;
    NXFloat top = 0, left = 0, bottom = 0, right = 0;
    NXFloat scale = 1;
    bool transitioning = false;
};

// Private UIKit bridge. Native controller/layer ownership stays in the .mm.
class IOSWindowGeometry {
public:
    explicit IOSWindowGeometry(void* nativeWindow);
    ~IOSWindowGeometry();
    IOSWindowFrame sample();
private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}
