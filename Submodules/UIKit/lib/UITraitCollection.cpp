#include <UITraitCollection.h>
#include <tools/Tools.hpp>

using namespace NXKit;

std::shared_ptr<UITraitCollection> UITraitCollection::_current = new_shared<UITraitCollection>();

std::shared_ptr<UITraitCollection> UITraitCollection::copy() {
    auto newCollection = new_shared<UITraitCollection>();
    newCollection->_userInterfaceStyle = _userInterfaceStyle;
    newCollection->_displayScale = _displayScale;
    return newCollection;
}
