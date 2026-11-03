#pragma once
#include <UIView.h>
namespace NXKit::Internal {
class ViewCornerAccess {
public:
    static void clearConfiguration(UIView& view) { view.clearCornerConfiguration(); }
};
}
