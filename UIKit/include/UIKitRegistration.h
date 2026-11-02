#pragma once

namespace NXKit {
/** Register UIKit's XML components before inflating a layout that uses them.
    Safe to call repeatedly; Core and NXKit do not register optional UIKit types. */
void registerUIKitViews();
}
