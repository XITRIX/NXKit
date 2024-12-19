#include <UITraitEnvironment.h>

using namespace NXKit;

void UITraitEnvironment::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    invalidateTraitCollection();
}
