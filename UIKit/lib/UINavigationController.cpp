#include <UINavigationController.h>
#include <UIWindow.h>
#include <UIPress.h>
#include <UIPanGestureRecognizer.h>
#include <UITouch.h>
#include "LayerAnimationTimeline.hpp"
#include "ViewCornerAccess.hpp"
#include "UINavigationBarInternal.hpp"
#include "NavigationBarMotion.hpp"
#include "ViewControllerContainerAccess.hpp"
#include "UIScrollPocket.hpp"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
using namespace NXKit;
namespace {bool contains(const auto& controllers,const auto& controller){return std::find(controllers.begin(),controllers.end(),controller)!=controllers.end();}}
namespace {
// The public capability returns UIGestureRecognizer, as UIKit does. Keeping
// edge admission here avoids exporting a partial UIScreenEdgePan API in Core.
class NavigationPopGesture final : public UIPanGestureRecognizer {
    std::function<bool()> allowed;
    std::function<void(std::shared_ptr<UIGestureRecognizer>)> action;
    NXPoint start;
    NXPoint lastLocation;
    Timer lastMovement;
    NXFloat release=0;
    bool admitted=false;
public:
    NavigationPopGesture(std::function<bool()> eligible,std::function<void(std::shared_ptr<UIGestureRecognizer>)> handler):allowed(std::move(eligible)),action(std::move(handler)){setMaximumNumberOfTouches(1);}
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) override {
        const bool active=state()==UIGestureRecognizerState::began||state()==UIGestureRecognizerState::changed;
        UIPanGestureRecognizer::touchesBegan(touches,event);
        if(active){setState(UIGestureRecognizerState::cancelled);return;}
        const auto host=view().lock();
        if(touches.size()!=1||!host||!allowed()){setState(UIGestureRecognizerState::failed);return;}
        start=touches.front()->locationIn(host);lastLocation=touches.front()->absoluteLocation();lastMovement=touches.front()->timestamp();admitted=start.x>=host->bounds().minX()&&start.x<=host->bounds().minX()+24;
        if(!admitted)setState(UIGestureRecognizerState::failed);
    }
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) override {
        if(!touches.empty()&&touches.front()->absoluteLocation()!=lastLocation){lastLocation=touches.front()->absoluteLocation();lastMovement=touches.front()->timestamp();}
        if(state()==UIGestureRecognizerState::possible&&!touches.empty()) {
            const auto delta=touches.front()->locationIn(view().lock())-start;
            if(std::max(std::abs(delta.x),std::abs(delta.y))>=10&&(delta.x<=0||std::abs(delta.y)>=delta.x||!allowed())){setState(UIGestureRecognizerState::failed);return;}
        }
        UIPanGestureRecognizer::touchesMoved(touches,event);
    }
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) override {
        release=touches.empty()||touches.front()->timestamp()-lastMovement>100?0:velocityIn(view().lock()).x;
        UIPanGestureRecognizer::touchesEnded(touches,event);
    }
    NXFloat releaseVelocity() const{return release;}
    bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>& other) const override {
        // Content is visited first by UIWindow. An eligible edge gesture must
        // retain a chance to recognize before a horizontal scroll cancels it.
        return admitted&&_state==UIGestureRecognizerState::possible?false:UIPanGestureRecognizer::canBePreventedBy(other);
    }
protected:
    void didChangeState() override {action(shared_from_this());}
    void reset() override {UIPanGestureRecognizer::reset();admitted=false;}
};
// Resolve the stationary host before translating it. The entire controller
// travels as a card with the window's corners, including during a reversal.
void beginNavigationCard(const std::shared_ptr<UIView>& host) {
    UIView::performWithoutAnimation([&]{
        host->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::containerConcentric()));
        const auto fixed=[&](UIRectCorner corner){return UICornerRadius::fixed(host->effectiveRadius(corner));};
        host->setCornerConfiguration(UICornerConfiguration::corners(fixed(UIRectCorner::topLeft),fixed(UIRectCorner::topRight),
            fixed(UIRectCorner::bottomLeft),fixed(UIRectCorner::bottomRight)));
        host->setClipsToBounds(true);
    });
}
void endNavigationCard(const std::shared_ptr<UIView>& host) {
    if (!host) return;
    Internal::ViewCornerAccess::clearConfiguration(*host);
    host->setClipsToBounds(false);
}
constexpr double interactiveSpan=.97;
}
struct UINavigationController::InteractivePop {
    std::vector<std::shared_ptr<UIViewController>> original;
    std::shared_ptr<UIViewController> source,target;
    std::shared_ptr<UIView> sourceHost,targetHost,dimming;
    std::unique_ptr<Internal::LayerAnimationTimeline> timeline;
    NXFloat width=0;
    struct End {bool commit;NXFloat velocity;bool animated;};
    std::optional<End> pendingEnd;
    bool starting=true,ending=false,finishImmediately=false,settling=false,commit=false;
};
UINavigationController::~UINavigationController()=default;
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
    auto gesture=new_shared<NavigationPopGesture>([weak=weak_from_base<UINavigationController>()]{const auto self=weak.lock();return self&&self->canBeginInteractivePop();},[weak=weak_from_base<UINavigationController>()](const auto& recognizer){if(const auto self=weak.lock())self->handlePopGesture(recognizer);});
    _popGesture=gesture;root->addGestureRecognizer(gesture);
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
    const auto nextView=next?Internal::makeScrollPocketHost(next):nullptr;
    _outgoingHost=oldView;_displayedHost=nextView;
    const auto bounds=_content->bounds();
    if(nextView){nextView->setFrame(bounds);if(direction<0&&oldView)_content->insertSubviewBelow(nextView,oldView);else _content->addSubview(nextView);nextView->addSubview(next->view());next->view()->setFrame(bounds);Internal::updateScrollPocketHost(nextView);}
    if(_visible) {
        if(previous)previous->viewWillDisappear(animated);
        if(next)next->viewWillAppear(animated);
        if(next)if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),next,animated);
    }
    if(_visible&&next)if(const auto window=view()->window())window->focusSystem()->requestFocusUpdate(next);
    std::shared_ptr<UIView> dimming;
    if(animate) {beginNavigationCard(oldView);beginNavigationCard(nextView);dimming=new_shared<UIView>();dimming->setUserInteractionEnabled(false);dimming->setBackgroundColor(UIColor::black);dimming->setFrame(bounds);(direction<0?nextView:oldView)->addSubview(dimming);dimming->setAlpha(direction<0?0:.1f);}
    const auto weak=weak_from_base<UINavigationController>();const auto visible=_visible;
    auto completion=[weak,previous,next,oldView,nextView,dimming,removed=std::move(removed),visible,animated](bool) {
        const auto self=weak.lock();if(!self)return;
        if(oldView&&oldView!=nextView){oldView->removeFromSuperview();if(previous&&previous->viewIsLoaded())previous->view()->removeFromSuperview();}
        if(dimming)dimming->removeFromSuperview();
        endNavigationCard(oldView);endNavigationCard(nextView);
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
    if(_interactive&&_interactive->width!=bounds.width())endInteractivePop(false,0,false);
    _content->setFrame(bounds);if(_displayedHost)_displayedHost->setFrame(_content->bounds());if(_outgoingHost)_outgoingHost->setFrame(_content->bounds());const auto height=_bar->sizeThatFits(bounds.size).height;_bar->setFrame({bounds.origin.x,_barHidden?-height:bounds.origin.y+safe.top,bounds.width(),height});_bar->setHidden(_barHidden);
    for(const auto& child:_controllers){Internal::ViewControllerContainerAccess::setChildSafeAreaInsets(shared_from_this(),child,{_barHidden?0.0f:height,0,0,0});if(child->viewIsLoaded())child->view()->setFrame(_content->bounds());}
    Internal::updateScrollPocketHost(_displayedHost);Internal::updateScrollPocketHost(_outgoingHost);
}
void UINavigationController::setNavigationBarHidden(bool hidden,bool animated) {
    if(_barHidden==hidden)return;if(_interactive)endInteractivePop(false,0,false);_barHidden=hidden;if(!viewIsLoaded())return;
    if(!animated){layout();_bar->setAlpha(1);return;}
    _bar->setHidden(false);_bar->setAlpha(hidden?1:0);
    UIView::animate(.25,0,UIViewAnimationOptions(beginFromCurrentState|allowUserInteraction),[&]{layout();_bar->setHidden(false);_bar->setAlpha(hidden?0:1);},[weak=weak_from_base<UINavigationController>()](bool){if(const auto self=weak.lock()){self->layout();self->_bar->setAlpha(1);}});
}
void UINavigationController::show(const std::shared_ptr<UIViewController>& controller,const std::shared_ptr<UIResponder>&){pushViewController(controller,true);}
void UINavigationController::viewWillAppear(bool animated){_appearanceTransition=true;if(_displayed){_displayed->viewWillAppear(animated);if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),_displayed,animated);}}
void UINavigationController::viewDidAppear(bool animated){if(_displayed){if(const auto target=delegate())target->navigationControllerDidShow(shared_from_base<UINavigationController>(),_displayed,animated);_displayed->viewDidAppear(animated);}_visible=true;_appearanceTransition=false;if(!_updating)finish();}
void UINavigationController::viewWillDisappear(bool animated){_appearanceTransition=true;if(_interactive)endInteractivePop(false,0,false);if(_displayed)_displayed->viewWillDisappear(animated);}
void UINavigationController::viewDidDisappear(bool animated){if(_displayed)_displayed->viewDidDisappear(animated);_visible=false;_appearanceTransition=false;if(!_updating)finish();}
void UINavigationController::viewDidLayoutSubviews(){layout();}
void UINavigationController::viewSafeAreaInsetsDidChange(){if(viewIsLoaded())view()->setNeedsLayout();}
std::vector<std::shared_ptr<UIFocusEnvironment>> UINavigationController::preferredFocusEnvironments(){std::vector<std::shared_ptr<UIFocusEnvironment>> result;if(_displayed)result.push_back(_displayed);if(_bar)result.push_back(_bar);return result;}

std::shared_ptr<UIGestureRecognizer> UINavigationController::interactivePopGestureRecognizer() {loadViewIfNeeded();return _popGesture;}
bool UINavigationController::canBeginInteractivePop() {
    if(!_visible||_updating||_appearanceTransition||_controllers.size()<2||!_content||!std::isfinite(_content->bounds().width())||_content->bounds().width()<=0||presentedViewController())return false;
    const auto item=_controllers.back()->navigationItem();
    return !item->hidesBackButton()&&(item->leftBarButtonItems().empty()||item->leftItemsSupplementBackButton());
}
void UINavigationController::handlePopGesture(const std::shared_ptr<UIGestureRecognizer>& gesture) {
    const auto pan=std::static_pointer_cast<UIPanGestureRecognizer>(gesture);
    switch(gesture->state()) {
    case UIGestureRecognizerState::began:if(canBeginInteractivePop())beginInteractivePop();break;
    case UIGestureRecognizerState::changed:
        if(_interactive&&!_interactive->settling)_interactive->timeline->seek(pan->translationInView(view()).x/_interactive->width);
        break;
    case UIGestureRecognizerState::ended:
        if(_interactive&&!_interactive->settling){const auto velocity=std::static_pointer_cast<NavigationPopGesture>(gesture)->releaseVelocity();const auto p=_interactive->timeline->progress();endInteractivePop(velocity>=-100&&(p>.5||p+velocity/_interactive->width*.2>.5),velocity);}
        break;
    case UIGestureRecognizerState::cancelled:if(_interactive&&!_interactive->settling)endInteractivePop(false,0);break;
    default:break;
    }
}
void UINavigationController::beginInteractivePop() {
    _updating=true;_interactive=std::make_shared<InteractivePop>();const auto transition=_interactive;auto& state=*transition;
    state.original=_controllers;state.source=_controllers.back();state.target=_controllers[_controllers.size()-2];state.sourceHost=_displayedHost;state.width=_content->bounds().width();
    _controllers.pop_back();_displayed=state.target;_outgoingHost=state.sourceHost;
    state.targetHost=Internal::makeScrollPocketHost(state.target);state.targetHost->setFrame(_content->bounds());_displayedHost=state.targetHost;
    _content->insertSubviewBelow(state.targetHost,state.sourceHost);state.targetHost->addSubview(state.target->view());state.target->view()->setFrame(_content->bounds());
    Internal::updateScrollPocketHost(state.targetHost);
    state.dimming=new_shared<UIView>(_content->bounds());state.dimming->setUserInteractionEnabled(false);state.dimming->setBackgroundColor(UIColor::black);state.targetHost->addSubview(state.dimming);
    beginNavigationCard(state.sourceHost);beginNavigationCard(state.targetHost);
    // Loading/mounting application content can start its own animations. Take
    // the baseline afterwards so those never inherit our reversible clock.
    state.timeline=std::make_unique<Internal::LayerAnimationTimeline>(view()->layer(),interactiveSpan);
    std::vector<std::shared_ptr<UINavigationItem>> items;for(const auto& child:_controllers)items.push_back(child->navigationItem());
    Internal::NavigationBarAccess::beginInteractivePop(*_bar,std::move(items),interactiveSpan);
    UIView::performWithoutAnimation([&]{state.sourceHost->setTransform(NXAffineTransform::translationBy(state.width,0));state.targetHost->setTransform(NXAffineTransform::identity);state.dimming->setAlpha(0);});
    const auto linear=new_shared<CAMediaTimingFunction>([](double f){return f;});
    for(const auto& host:{state.sourceHost,state.targetHost}) {
        auto slide=Internal::navigationAnimation("transform");slide->duration=interactiveSpan;slide->timingFunction=linear;
        slide->fromValue=NXTransform3DMakeAffineTransform(host==state.sourceHost?NXAffineTransform::identity:NXAffineTransform::translationBy(-state.width*.3f,0));
        slide->toValue=NXTransform3DMakeAffineTransform(host==state.sourceHost?NXAffineTransform::translationBy(state.width,0):NXAffineTransform::identity);host->layer()->add(slide,"transform");
    }
    auto dim=Internal::navigationAnimation("opacity");dim->duration=interactiveSpan;dim->timingFunction=linear;dim->fromValue=.1f;dim->toValue=0.f;state.dimming->layer()->add(dim,"opacity");
    state.timeline->capture(view()->layer());state.timeline->seek(0);
    state.source->willMoveToParent(nullptr);if(_interactive!=transition)return;
    state.source->viewWillDisappear(true);if(_interactive!=transition)return;
    state.target->viewWillAppear(true);if(_interactive!=transition)return;
    if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),state.target,true);
    if(_interactive!=transition)return;
    state.starting=false;
    if(const auto end=state.pendingEnd)endInteractivePop(end->commit,end->velocity,end->animated);
}
void UINavigationController::endInteractivePop(bool commit,NXFloat velocity,bool animated) {
    if(!_interactive)return;
    const auto transition=_interactive;auto& state=*transition;
    if(state.starting){state.pendingEnd=InteractivePop::End{commit,velocity,animated};return;}
    if(state.ending){state.finishImmediately|=!animated;return;}
    if(state.settling){if(!animated)finishInteractivePop(state.commit);return;}
    state.settling=true;state.commit=commit;state.ending=true;
    if(_interactive.get()!=&state)return;
    if(commit)Internal::NavigationBarAccess::endInteractivePop(*_bar,true,state.timeline->progress());
    state.ending=false;
    if(!animated||state.finishImmediately){finishInteractivePop(commit);return;}
    const auto progress=state.timeline->progress(),distance=std::abs((commit?1.:0.)-progress);
    const auto duration=.5;
    // Bound the release tangent so the return cannot move farther away before
    // it follows the finger's final direction. Geometry and bar share this clock.
    const auto direction=commit?1.:-1.;const auto speed=std::clamp(double(velocity/state.width)*direction,0.,3*distance/duration)*direction;
    state.timeline->settle(commit?1:0,speed,duration,[duration,speed,delta=(commit?1.:0.)-progress](double f){
        if(f>=1)return 1.;const auto t=f*duration,decay=.86*25,frequency=25*std::sqrt(1-.86*.86);
        const auto tangent=std::abs(delta)>.00001?speed/delta:0.;
        return 1-std::exp(-decay*t)*(std::cos(frequency*t)+(decay-tangent)/frequency*std::sin(frequency*t));
    });
    auto gate=Internal::navigationAnimation("opacity");gate->duration=duration;gate->fromValue=1.f;gate->toValue=1.f;
    gate->animationGroup=new_shared<UIViewAnimationGroup>(UIViewAnimationOptions(allowUserInteraction|preferredFramesPerSecond120),[weak=weak_from_base<UINavigationController>(),commit](bool){if(const auto self=weak.lock())self->finishInteractivePop(commit);});
    _content->layer()->add(gate,"interactivePopCompletion");
}
void UINavigationController::finishInteractivePop(bool commit) {
    if(!_interactive)return;
    auto state=std::move(_interactive);_content->layer()->removeAnimation("interactivePopCompletion");
    state->timeline->discard();
    state->dimming->removeFromSuperview();
    if(commit) {
        Internal::NavigationBarAccess::finishInteractivePop(*_bar);
        state->source->view()->removeFromSuperview();state->sourceHost->removeFromSuperview();state->source->removeFromParent();
        state->targetHost->setTransform(NXAffineTransform::identity);_displayed=state->target;
    }else {
        _controllers=state->original;_displayed=state->source;_displayedHost=state->sourceHost;
        state->target->view()->removeFromSuperview();state->targetHost->removeFromSuperview();state->sourceHost->setTransform(NXAffineTransform::identity);
        Internal::NavigationBarAccess::endInteractivePop(*_bar,false,0);
        state->source->didMoveToParent(shared_from_this());
    }
    endNavigationCard(state->sourceHost);endNavigationCard(state->targetHost);
    _outgoingHost.reset();
    const auto shown=commit?state->target:state->source;
    if(commit){state->source->viewDidDisappear(true);state->target->viewDidAppear(true);}
    else {
        // UIKit 27 reconciles a cancelled appearance transaction after the
        // visual return, with the original logical stack already restored.
        state->target->viewWillDisappear(true);state->target->viewDidDisappear(true);
        state->source->viewWillAppear(true);state->source->viewDidAppear(true);
        if(const auto target=delegate())target->navigationControllerWillShow(shared_from_base<UINavigationController>(),shown,true);
    }
    if(const auto window=view()->window())window->focusSystem()->requestFocusUpdate(shown);
    if(const auto target=delegate())target->navigationControllerDidShow(shared_from_base<UINavigationController>(),shown,true);
    finish();
}
