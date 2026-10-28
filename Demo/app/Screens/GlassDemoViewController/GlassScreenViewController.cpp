#include "GlassScreenViewController.hpp"
#include <NXNavigationController.h>
using namespace NXKit;
void GlassScreenViewController::willMoveToParent(const std::shared_ptr<UIViewController>& parent) {
    UIViewController::willMoveToParent(parent);
    if(auto navigation=std::dynamic_pointer_cast<NXNavigationController>(parent)) {
        auto item=navigation->navigationItemFor(shared_from_this());
        item->setHidesNavigationBar(true); item->setHidesToolbar(true);
    }
}
void GlassScreenViewController::loadView() {
    auto screen=new_shared<GlassScreenView>();
    screen->onBack=[weak=weak_from_base<GlassScreenViewController>()] {
        if(auto self=weak.lock()) {
            for(auto current=self->parent().lock();current;current=current->parent().lock())
                if(auto navigation=std::dynamic_pointer_cast<NXNavigationController>(current)) {
                    if(navigation->topViewController()==self) navigation->popViewController(true);
                    return;
                }
            if(self->presentingViewController()) self->dismiss(true);
        }
    };
    setView(screen);
}
