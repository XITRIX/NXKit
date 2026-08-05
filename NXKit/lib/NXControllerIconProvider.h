#pragma once

#include <NXControllerIconResolver.h>

#include <functional>
#include <memory>

class SkCanvas;

namespace NXKit {

class NXControllerIconProvider {
public:
    virtual ~NXControllerIconProvider() = default;

    [[nodiscard]] virtual std::shared_ptr<UIImage> iconForButton(
        NXActionButton button,
        NXControllerType controllerType,
        NXFloat pointSize,
        NXFloat scale
    ) const = 0;
};

[[nodiscard]] std::unique_ptr<NXControllerIconProvider>
NXMakePlatformControllerIconProvider();

[[nodiscard]] std::shared_ptr<UIImage> NXMakeControllerIconImage(
    NXFloat pointSize,
    NXFloat scale,
    const std::function<void(SkCanvas&, NXFloat)>& drawing
);

} // namespace NXKit
