#pragma once

#include <UITraitCollection.h>

namespace NXKit {

class UITraitEnvironment {
public:
    std::shared_ptr<UITraitCollection> traitCollection() const { return _traitCollection; }

    virtual void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection);

protected:
    friend bool applicationRunLoop();
    friend class UIWindow;
    friend class UIViewController;
    friend class UIView;

    std::shared_ptr<UITraitCollection> _traitCollection;
    
    virtual void invalidateTraitCollection() {}
};

}
