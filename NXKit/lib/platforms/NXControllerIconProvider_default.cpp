#include "NXControllerIconProvider.h"

namespace NXKit {

namespace {

class DefaultControllerIconProvider final : public NXControllerIconProvider {
public:
    std::shared_ptr<UIImage> iconForButton(
        NXActionButton,
        NXControllerType,
        NXFloat,
        NXFloat
    ) const override {
        return nullptr;
    }
};

} // namespace

std::unique_ptr<NXControllerIconProvider>
NXMakePlatformControllerIconProvider() {
    return std::make_unique<DefaultControllerIconProvider>();
}

} // namespace NXKit
