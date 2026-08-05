#pragma once

#include <NXResponderAction.h>
#include <Geometry.h>

#include <memory>

namespace NXKit {

class UIImage;

enum class NXControllerType {
    automatic,
    generic,
    xbox,
    playStation,
    nintendoSwitch,
};

// Resolves controller hints into template images. Automatic resolution follows
// the controller that most recently produced SDL gamepad input. On Switch the
// Nintendo controller family is always used because it is the only supported
// controller family.
class NXControllerIconResolver {
public:
    static NXControllerIconResolver& shared();

    [[nodiscard]] NXControllerType currentControllerType() const;

    [[nodiscard]] std::shared_ptr<UIImage> iconForButton(
        NXActionButton button,
        NXControllerType controllerType = NXControllerType::automatic,
        NXFloat pointSize = 24
    ) const;

    ~NXControllerIconResolver();

    NXControllerIconResolver(const NXControllerIconResolver&) = delete;
    NXControllerIconResolver& operator=(const NXControllerIconResolver&) = delete;

private:
    NXControllerIconResolver();

    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace NXKit
