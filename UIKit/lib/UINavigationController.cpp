#include <UINavigationController.h>
#include <UIWindow.h>
#include <UIPress.h>
#include "UINavigationBarInternal.hpp"
#include "NavigationBarMotion.hpp"
#include "ViewControllerContainerAccess.hpp"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
using namespace NXKit;
namespace {bool contains(const auto& controllers,const auto& controller){return std::find(controllers.begin(),controllers.end(),controller)!=controllers.end();}}
UINavigationController::UINavigationController() {
    const auto weak=weak_from_base<UINavigationController>();
    registerAction(UIResponderAction {
        .identifier="UIKit.navigationController.back",
        .inputIdentifier=UIResponderActionInputMenu,
        .action=UIAction("Back",[weak]{if(const auto self=weak.lock())self->popViewController(true);}),
        .matches=[](const auto& press){return press&&press->type()==UIPressType::menu;},
        .canPerform=[weak]{const auto self=weak.lock();return self&&self->_controllers.size()>1;}
    });
}
UINavigationController::UINavigationController(const std::shared_ptr<UIViewController>& root):UINavigationController() {
    if(!root)throw std::invalid_argument("A root controller cannot be null");setViewControllers({root},false);
}
const std::vector<std::shared_ptr<UIViewController>>& UINavigationController::requested() const {return _pending.empty()?_controllers:_pending.back().controllers;}
void UINavigationController::validate(const std::vector<std::shared_ptr<UIViewController>>& controllers) {
    std::unordered_set<const UIViewController*> unique;
    for(const auto& controller:controllers) {
        if(!controller||!unique.insert(controller.get()).second)throw std::invalid_argument("Navigation controllers must be non-null and distinct");
        for(auto ancestor=shared_from_this();ancestor;ancestor=ancestor->parent().lock())if(ancestor==controller)throw std::invalid_argument("Navigation containment cannot form a cycle");
        if(const auto parent=controller->parent().lock();parent&&parent.get()!=this)throw std::invalid_argument("Navigation content already has a parent");
        if(std::dynamic_pointer_cast<UINavigationControllerInterface>(controller))throw std::invalid_argument("A navigation stack cannot contain another navigation controller");
    }
}
void UINavigationController::setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers,bool animated) {
    validate(controllers);if(controllers==requested())return;
    if(_updating||_appearanceTransition){_pending.push_back({std::move(controllers),animated});return;}
    apply(std::move(controllers),animated);
}
void UINavigationController::pushViewController(const std::shared_ptr<UIViewController>& controller,bool animated) {auto controllers=requested();controllers.push_back(controller);setViewControllers(std::move(controllers),animated);}
std::shared_ptr<UIViewController> UINavigationController::popViewController(bool animated) {
    auto controllers=requested();if(controllers.size()<2)return nullptr;const auto top=controllers.back();controllers.pop_back();setViewControllers(std::move(controllers),animated);return top;
}
std::vector<std::shared_ptr<UIViewController>> UINavigationController::popToViewController(const std::shared_ptr<UIViewController>& controller,bool animated) {
    const auto controllers=requested();const auto found=std::find(controllers.begin(),controllers.end(),controller);
    if(found==controllers.end()||std::next(found)==controllers.end())return {};
    std::vector<std::shared_ptr<UIViewController>> removed(std::next(found),controllers.end());setViewControllers({controllers.begin(),std::next(found)},animated);return removed;
}
std::vector<std::shared_ptr<UIViewController>> UINavigationController::popToRootViewController(bool animated) {return requested().empty()?std::vector<std::shared_ptr<UIViewController>>{}:popToViewController(requested().front(),animated);}
std::shared_ptr<UIViewController> UINavigationController::topViewController() const {return _controllers.empty()?nullptr:_controllers.back();}
std::shared_ptr<UIViewController> UINavigationController::visibleViewController() const {
    if(const auto presented=const_cast<UINavigationController*>(this)->presentedViewController())return presented;return _displayed?_displayed:topViewController();
}
std::shared_ptr<UINavigationBar> UINavigationController::navigationBar(){loadViewIfNeeded();return _bar;}
void UINavigationController::loadView() {
    auto root=new_shared<UIView>();root->setBackgroundColor(UIColor::systemBackground);
    _content=new_shared<UIView>();_content->setClipsToBounds(true);root->addSubview(_content);
    _bar=new_shared<UINavigationBar>();root->addSubview(_bar);
    auto& state=Internal::NavigationBarAccess::state(*_bar);state.owner=shared_from_this();state.delegate=shared_from_base<UINavigationController>();
    state.backAction=[weak=weak_from_base<UINavigationController>()]{if(const auto self=weak.lock();self&&!self->_updating)self->popViewController(true);};
    setView(root);
}
void UINavigationController::viewDidLoad(){_updating=true;layout();mount(0,false,{});}
void UINavigationController::apply(std::vector<std::shared_ptr<UIViewController>> controllers,bool animated) {
    const auto keepAlive=shared_from_this();validate(controllers);_updating=true;
    try {
        const auto previous=_controllers;const auto oldTop=previous.empty()?std::shared_ptr<UIViewController>{}:previous.back();
        const auto next=controllers.empty()?nullptr:controllers.back();
        std::vector<std::shared_ptr<UIViewController>> removed;for(const auto& child:previous)if(!contains(controllers,child))removed.push_back(child);
        _controllers=std::move(controllers);
        for(const auto& child:_controllers)if(child->parent().expired()){addChild(child);child->didMoveToParent(shared_from_this());}
        for(const auto& child:removed)child->willMoveToParent(nullptr);
        const auto direction=next==oldTop?0:(contains(previous,next)?-1:1);
        if(viewIsLoaded()){layout();mount(direction,animated,std::move(removed));}
        else {for(const auto& child:removed)child->removeFromParent();finish();}
    }catch(...){_updating=false;_pending.clear();throw;}
}
void UINavigationController::mount(int direction,bool animated,std::vector<std::shared_ptr<UIViewController>> removed) {
    const auto next=topViewController(),previous=_displayed;
    std::vector<std::shared_ptr<UINavigationItem>> items;for(const auto& child:_controllers)items.push_back(child->navigationItem());
    const bool animate=animated&&_visible&&previous&&next&&previous!=next&&_content->bounds().width()>0;
    Internal::NavigationBarAccess::setItems(*_bar,std::move(items),animated&&_visible,direction);
    if(next==previous) {for(const auto& child:removed)child->removeFromParent();finish();return;}
    _displayed=next;
    const auto oldView=_displayedHost;
    const auto nextView=next?new_shared<UIView>():nullptr;
    _outgoingHost=oldView;_displayedHost=nextView;
    const auto bounds=_content->bounds();
    if(nextView){nextView->setFrame(bounds);if(direction<0&&oldView)_content->insertSubviewBelow(nextView,oldView);else _content->addSubview(nextView);nextView->addSubview(next->view());next->view()->setFrame(bounds);}
    if(_visible) {
        if(previous)previous->viewWillDisappear(animated);
        if(next)next->viewWillAppear(animated);
        if(next)if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),next,animated);
    }
    if(_visible&&next)if(const auto window=view()->window())window->focusSystem()->requestFocusUpdate(next);
    std::shared_ptr<UIView> dimming;
    if(animate) {dimming=new_shared<UIView>();dimming->setUserInteractionEnabled(false);dimming->setBackgroundColor(UIColor::black);dimming->setFrame(bounds);(direction<0?nextView:oldView)->addSubview(dimming);dimming->setAlpha(direction<0?0:.1f);}
    const auto weak=weak_from_base<UINavigationController>();const auto visible=_visible;
    auto completion=[weak,previous,next,oldView,nextView,dimming,removed=std::move(removed),visible,animated](bool) {
        const auto self=weak.lock();if(!self)return;
        if(oldView&&oldView!=nextView){oldView->removeFromSuperview();if(previous&&previous->viewIsLoaded())previous->view()->removeFromSuperview();}
        if(dimming)dimming->removeFromSuperview();
        self->_outgoingHost.reset();
        if(nextView)nextView->setTransform(NXAffineTransform::identity);
        for(const auto& child:removed)if(!contains(self->_controllers,child))child->removeFromParent();
        if(visible){if(previous)previous->viewDidDisappear(animated);if(next)next->viewDidAppear(animated);if(next)if(const auto target=self->delegate())target->navigationControllerDidShow(self,next,animated);}
        self->finish();
    };
    if(!animate){completion(true);return;}
    const auto width=bounds.width();
    const auto from=NXAffineTransform::translationBy(direction<0?-width*.3f:width,0);
    const auto to=NXAffineTransform::translationBy(direction<0?width:-width*.3f,0);
    UIView::performWithoutAnimation([&]{nextView->setTransform(NXAffineTransform::identity);oldView->setTransform(to);});
    auto group=new_shared<UIViewAnimationGroup>(UIViewAnimationOptions(preferredFramesPerSecond120),std::move(completion));
    auto dim=Internal::navigationAnimation("opacity");dim->duration=Internal::navigationContentDuration;dim->animationGroup=group;dim->fromValue=direction<0?.1f:0.f;dim->toValue=direction<0?0.f:.1f;dim->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){return fraction>=1?1.:Internal::navigationContentProgress(fraction*Internal::navigationContentDuration);});dimming->layer()->add(dim,"opacity");dimming->layer()->animateAt(Timer());
    for(const auto& host:{nextView,oldView}) {
        auto slide=Internal::navigationAnimation("transform");slide->duration=Internal::navigationContentDuration;slide->animationGroup=group;
        slide->fromValue=NXTransform3DMakeAffineTransform(host==nextView?from:NXAffineTransform::identity);
        slide->toValue=NXTransform3DMakeAffineTransform(host==nextView?NXAffineTransform::identity:to);
        slide->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){return fraction>=1?1.:Internal::navigationContentProgress(fraction*Internal::navigationContentDuration);});
        host->layer()->add(slide,"transform");host->layer()->animateAt(Timer());
    }
}
void UINavigationController::finish() {
    _updating=false;if(_appearanceTransition||_pending.empty())return;
    auto update=std::move(_pending.front());_pending.pop_front();apply(std::move(update.controllers),update.animated);
}
void UINavigationController::layout() {
    if(!_bar||!_content)return;const auto bounds=view()->bounds();const auto safe=view()->safeAreaInsets();
    _content->setFrame(bounds);if(_displayedHost)_displayedHost->setFrame(_content->bounds());if(_outgoingHost)_outgoingHost->setFrame(_content->bounds());const auto height=_bar->sizeThatFits(bounds.size).height;_bar->setFrame({bounds.origin.x,_barHidden?-height:bounds.origin.y+safe.top,bounds.width(),height});_bar->setHidden(_barHidden);
    for(const auto& child:_controllers){Internal::ViewControllerContainerAccess::setChildSafeAreaInsets(shared_from_this(),child,{_barHidden?0.0f:height,0,0,0});if(child->viewIsLoaded())child->view()->setFrame(_content->bounds());}
}
void UINavigationController::setNavigationBarHidden(bool hidden,bool animated) {
    if(_barHidden==hidden)return;_barHidden=hidden;if(!viewIsLoaded())return;
    if(!animated){layout();_bar->setAlpha(1);return;}
    _bar->setHidden(false);_bar->setAlpha(hidden?1:0);
    UIView::animate(.25,0,UIViewAnimationOptions(beginFromCurrentState|allowUserInteraction),[&]{layout();_bar->setHidden(false);_bar->setAlpha(hidden?0:1);},[weak=weak_from_base<UINavigationController>()](bool){if(const auto self=weak.lock()){self->layout();self->_bar->setAlpha(1);}});
}
void UINavigationController::show(const std::shared_ptr<UIViewController>& controller,const std::shared_ptr<UIResponder>&){pushViewController(controller,true);}
void UINavigationController::viewWillAppear(bool animated){_appearanceTransition=true;if(_displayed){_displayed->viewWillAppear(animated);if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),_displayed,animated);}}
void UINavigationController::viewDidAppear(bool animated){if(_displayed){if(const auto target=delegate())target->navigationControllerDidShow(shared_from_base<UINavigationController>(),_displayed,animated);_displayed->viewDidAppear(animated);}_visible=true;_appearanceTransition=false;if(!_updating)finish();}
void UINavigationController::viewWillDisappear(bool animated){_appearanceTransition=true;if(_displayed)_displayed->viewWillDisappear(animated);}
void UINavigationController::viewDidDisappear(bool animated){if(_displayed)_displayed->viewDidDisappear(animated);_visible=false;_appearanceTransition=false;if(!_updating)finish();}
void UINavigationController::viewDidLayoutSubviews(){layout();}
void UINavigationController::viewSafeAreaInsetsDidChange(){if(viewIsLoaded())view()->setNeedsLayout();}
std::vector<std::shared_ptr<UIFocusEnvironment>> UINavigationController::preferredFocusEnvironments(){std::vector<std::shared_ptr<UIFocusEnvironment>> result;if(_displayed)result.push_back(_displayed);if(_bar)result.push_back(_bar);return result;}
