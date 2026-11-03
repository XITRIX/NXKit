#include <UITabBarController.h>
#include <UIWindow.h>
#include "UITabBarViewInternal.hpp"
#include "ViewControllerContainerAccess.hpp"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

using namespace NXKit;

void UITabBarController::setMode(Mode mode) {
    if (mode != Mode::automatic && mode != Mode::tabBar)
        throw std::invalid_argument("UITabBarController currently supports automatic and tabBar modes");
    _mode = mode;
}
void UITabBarController::validate(const std::vector<std::shared_ptr<UIViewController>>& controllers) {
    std::unordered_set<const UIViewController*> unique;
    std::unordered_set<const UITabBarItem*> items;
    for (const auto& controller : controllers) {
        if (!controller || !unique.insert(controller.get()).second)
            throw std::invalid_argument("Tab view controllers must be non-null and distinct");
        for (auto ancestor=shared_from_this();ancestor;ancestor=ancestor->parent().lock())
            if (ancestor==controller) throw std::invalid_argument("Tab containment cannot form a cycle");
        if (const auto parent=controller->parent().lock();parent && parent.get()!=this)
            throw std::invalid_argument("A tab view controller already belongs to another container");
        if (!items.insert(controller->tabBarItem().get()).second)
            throw std::invalid_argument("Each tab view controller requires a distinct tab bar item");
    }
}
void UITabBarController::runUpdate(std::function<void()> update) {
    if (_updating || _appearanceTransition) { _pending.push_back(std::move(update)); return; }
    const auto keepAlive = shared_from_this();
    _updating = true;
    try {
        update();
        while (!_pending.empty() && !_appearanceTransition) {
            auto next=std::move(_pending.front()); _pending.pop_front(); next();
        }
    } catch (...) { _updating=false; _pending.clear(); throw; }
    _updating=false;
}
void UITabBarController::drainUpdates() {
    if (!_pending.empty() && !_appearanceTransition && !_updating) {
        auto next=std::move(_pending.front()); _pending.pop_front(); runUpdate(std::move(next));
    }
}
void UITabBarController::setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers) {
    setViewControllers(std::move(controllers),false);
}
void UITabBarController::setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool animated) {
    validate(controllers);
    runUpdate([this,controllers=std::move(controllers),animated]() mutable {
        applyControllers(std::move(controllers),animated);
    });
}
void UITabBarController::applyControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool animated) {
    validate(controllers);
    const auto previous=_controllers;
    const auto selected=selectedViewController();
    const auto previousIndex=_selectedIndex;
    _controllers=std::move(controllers);
    const auto found=std::find(_controllers.begin(),_controllers.end(),selected);
    if (_controllers.empty()) _selectedIndex.reset();
    else if (found!=_controllers.end()) _selectedIndex=size_t(found-_controllers.begin());
    else _selectedIndex=previousIndex && *previousIndex<_controllers.size()?*previousIndex:0;
    ++_revision;
    const auto self=shared_from_this();
    const auto weak=weak_from_base<UITabBarController>();
    for (const auto& controller:_controllers) {
        if (controller->parent().expired()) { addChild(controller); controller->didMoveToParent(self); }
        Internal::ViewControllerContainerAccess::observeTabBarItem(*controller,self,
            [weak,child=std::weak_ptr<UIViewController>(controller)] {
                if (const auto owner=weak.lock()) {
                    if (const auto value=child.lock(); value && value->parent().lock()==owner)
                        owner->runUpdate([weak] { if (const auto current=weak.lock()) { ++current->_revision; current->synchronizeBar(); } });
                }
            });
    }
    synchronizeBar(animated);
    for (const auto& controller:previous)
        if (std::find(_controllers.begin(),_controllers.end(),controller)==_controllers.end()
            && controller->parent().lock().get()==this) controller->willMoveToParent(nullptr);
    updateDisplayedController();
    for (const auto& controller:previous) {
        if (std::find(_controllers.begin(),_controllers.end(),controller)!=_controllers.end()) continue;
        if (controller->parent().lock().get()==this) {
            if (controller->viewIsLoaded()) controller->view()->removeFromSuperview();
            controller->removeFromParent();
        }
    }
    if (viewIsLoaded()) updateLayout();
}
std::shared_ptr<UIViewController> UITabBarController::selectedViewController() const {
    return _selectedIndex && *_selectedIndex<_controllers.size()?_controllers[*_selectedIndex]:nullptr;
}
bool UITabBarController::setSelectedIndex(size_t index) {
    if (index>=_controllers.size()) return false;
    return setSelectedViewController(_controllers[index]);
}
bool UITabBarController::setSelectedViewController(const std::shared_ptr<UIViewController>& controller) {
    if (!controller) return _controllers.empty();
    if (std::find(_controllers.begin(),_controllers.end(),controller)==_controllers.end()) return false;
    runUpdate([this,controller] {
        if (std::find(_controllers.begin(),_controllers.end(),controller)!=_controllers.end())
            applySelection(controller,false);
    });
    return true;
}
void UITabBarController::applySelection(const std::shared_ptr<UIViewController>& controller, bool animateBar) {
    const auto found=std::find(_controllers.begin(),_controllers.end(),controller);
    if (found==_controllers.end()) return;
    const auto previous=selectedViewController();
    _selectedIndex=size_t(found-_controllers.begin());
    if (previous!=controller) ++_revision;
    if (_tabBar) Internal::TabBarViewAccess::select(*_tabBar,controller->tabBarItem(),animateBar);
    updateDisplayedController();
}
std::shared_ptr<UITabBar> UITabBarController::tabBar() { loadViewIfNeeded(); return _tabBar; }
void UITabBarController::loadView() {
    auto root=new_shared<UIView>();
    _contentView=new_shared<UIView>(); root->addSubview(_contentView);
    _tabBar=new_shared<UITabBarView>(); root->addSubview(_tabBar);
    auto& bar=Internal::TabBarViewAccess::state(*_tabBar);
    const auto self=shared_from_base<UITabBarController>();
    bar.owner=self; bar.delegate=self;
    bar.managedSelection=[weak=weak_from_base<UITabBarController>()](const auto& item,bool user) {
        if (const auto owner=weak.lock()) return owner->handleSelection(item,user);
        return false;
    };
    setView(root);
}
void UITabBarController::viewDidLoad() {
    runUpdate([this] { synchronizeBar(); updateDisplayedController(); updateLayout(); });
}
void UITabBarController::synchronizeBar(bool animated) {
    if (!_tabBar) return;
    std::vector<std::shared_ptr<UITabBarItem>> items;
    for (const auto& controller:_controllers) items.push_back(controller->tabBarItem());
    if (_tabBar->items()!=items) Internal::TabBarViewAccess::setItems(*_tabBar,std::move(items),animated);
    const auto selected=selectedViewController();
    Internal::TabBarViewAccess::select(*_tabBar,selected?selected->tabBarItem():nullptr);
    _tabBar->setHidden(_controllers.empty());
}
bool UITabBarController::handleSelection(const std::shared_ptr<UITabBarItem>& item, bool user) {
    const auto found=std::find_if(_controllers.begin(),_controllers.end(),[&](const auto& value) {
        return value->tabBarItem()==item;
    });
    if (found==_controllers.end() || (user && (!item->isEnabled() || !allowsActionDispatch()))) return false;
    const auto candidate=*found;
    const auto target=_delegate.lock();
    const auto revision=_revision;
    const auto self=shared_from_base<UITabBarController>();
    if (user && target && !target->tabBarControllerShouldSelect(self,candidate)) return false;
    if (_revision!=revision || std::find(_controllers.begin(),_controllers.end(),candidate)==_controllers.end()
        || candidate->tabBarItem()!=item || (user && !item->isEnabled())) return false;
    runUpdate([this,candidate,item,user,target] {
        if (std::find(_controllers.begin(),_controllers.end(),candidate)==_controllers.end()
            || candidate->tabBarItem()!=item) return;
        applySelection(candidate,user);
        if (user && target) target->tabBarControllerDidSelect(shared_from_base<UITabBarController>(),candidate);
    });
    return true;
}
void UITabBarController::tabBarDidSelect(const std::shared_ptr<UITabBar>& bar,
                                        const std::shared_ptr<UITabBarItem>& item) {
    if (bar==_tabBar) handleSelection(item,true);
}
void UITabBarController::updateDisplayedController() {
    if (!_contentView) return;
    const auto next=selectedViewController();
    if (_displayed==next) return;
    const auto previous=std::exchange(_displayed,next);
    const auto window=view()->window();
    auto focused=window?std::dynamic_pointer_cast<UIView>(window->focusSystem()->focusedItem().lock()):nullptr;
    const bool focusWasInContent=focused && previous && previous->viewIsLoaded()
        && focused->isDescendantOf(previous->view());
    const auto nextView=next?next->view():nullptr;
    if (_visible && previous) previous->viewWillDisappear(false);
    if (previous && previous->viewIsLoaded()) previous->view()->removeFromSuperview();
    if (next) {
        _contentView->addSubview(nextView); nextView->setFrame(_contentView->bounds());
        Internal::ViewControllerContainerAccess::setChildSafeAreaInsets(shared_from_this(),next,_reservedInsets);
    }
    if (_visible && next) next->viewWillAppear(false);
    if (_visible && previous) previous->viewDidDisappear(false);
    if (_visible && next) next->viewDidAppear(false);
    if (window && _visible) {
        if (focusWasInContent && next) window->focusSystem()->requestFocusUpdate(next);
        else window->updateFocus();
    }
}
void UITabBarController::updateLayout() {
    if (!_tabBar || !_contentView) return;
    const auto bounds=view()->bounds();
    const auto safe=view()->safeAreaInsets();
    const auto width=std::max(0.0f,bounds.width()-safe.left-safe.right-32);
    const auto height=_controllers.empty()?0.0f:std::min(62.0f,std::max(0.0f,bounds.height()-safe.top-safe.bottom-24));
    const auto barWidth=std::min(1000.0f,width);
    _contentView->setFrame(bounds);
    _tabBar->setFrame({bounds.origin.x+safe.left+(bounds.width()-safe.left-safe.right-barWidth)*.5f,
        bounds.maxY()-safe.bottom-12-height,barWidth,height});
    _reservedInsets={0,0,_controllers.empty()?0.0f:std::min(std::max(0.0f,bounds.height()-safe.bottom),height+24),0};
    for (const auto& child:_controllers)
        Internal::ViewControllerContainerAccess::setChildSafeAreaInsets(shared_from_this(),child,_reservedInsets);
    if (_displayed) _displayed->view()->setFrame(_contentView->bounds());
}
void UITabBarController::viewDidLayoutSubviews() { runUpdate([this] { updateLayout(); }); }
void UITabBarController::viewSafeAreaInsetsDidChange() { if (viewIsLoaded()) view()->setNeedsLayout(); }
void UITabBarController::viewWillAppear(bool animated) {
    _appearanceTransition=true;
    if (_displayed) _displayed->viewWillAppear(animated);
}
void UITabBarController::viewDidAppear(bool animated) {
    if (_displayed) _displayed->viewDidAppear(animated);
    _visible=true; _appearanceTransition=false; drainUpdates();
}
void UITabBarController::viewWillDisappear(bool animated) {
    _appearanceTransition=true;
    if (_displayed) _displayed->viewWillDisappear(animated);
}
void UITabBarController::viewDidDisappear(bool animated) {
    if (_displayed) _displayed->viewDidDisappear(animated);
    _visible=false; _appearanceTransition=false; drainUpdates();
}
std::vector<std::shared_ptr<UIFocusEnvironment>> UITabBarController::preferredFocusEnvironments() {
    std::vector<std::shared_ptr<UIFocusEnvironment>> result;
    if (const auto selected=selectedViewController()) result.push_back(selected);
    if (_tabBar) result.push_back(_tabBar);
    return result;
}
