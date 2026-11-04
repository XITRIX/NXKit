#include <UITraitCollection.h>
#include <tools/Tools.hpp>
#include <cmath>
#include <stdexcept>

using namespace NXKit;

std::shared_ptr<UITraitCollection> UITraitCollection::_current = new_shared<UITraitCollection>();

std::shared_ptr<UITraitCollection> UITraitCollection::copy() {
    auto newCollection = new_shared<UITraitCollection>();
    newCollection->_userInterfaceStyle = _userInterfaceStyle;
    newCollection->_glassTintAmount = _glassTintAmount;
    newCollection->_displayScale = _displayScale;
    newCollection->_horizontalSizeClass = _horizontalSizeClass;
    newCollection->_verticalSizeClass = _verticalSizeClass;
    return newCollection;
}

std::shared_ptr<UITraitCollection> UITraitCollection::traitCollectionWithGlassTintAmount(NXFloat amount) {
    if (!std::isfinite(amount) || amount < 0 || amount > 1)
        throw std::invalid_argument("Glass tint amount must be finite and between 0 and 1");
    auto traits = new_shared<UITraitCollection>();
    traits->_glassTintAmount = amount;
    return traits;
}
