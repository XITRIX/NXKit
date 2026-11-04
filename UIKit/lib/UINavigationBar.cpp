#include <UINavigationBar.h>
#include "UINavigationBarInternal.hpp"
#include "NavigationItemAccess.hpp"
#include "BarItemAccess.hpp"
#include "NavigationBarMotion.hpp"
#include <UIGlassEffect.h>
#include <CABasicAnimation.h>
#include <UITouch.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <map>
#include <limits>
using namespace NXKit;
namespace {
// A capsule is the actual parent of its controls. The material's visual-only
// contact affine therefore carries its ink as well as its glass surface.
class NavigationGlassView final : public BackdropEffectView {
public:
    explicit NavigationGlassView(const UIGlassEffect& effect):BackdropEffectView(effect) {}
    bool point(NXPoint p,UIEvent* event) override {
        if(!BackdropEffectView::point(p,event)||!isActionDispatchAllowed())return false;
        for(const auto& child:subviews())if(const auto control=std::dynamic_pointer_cast<UIControl>(child))
            if(control->isEnabled()&&!control->isHidden()&&control->point(p-control->frame().origin,event))return true;
        return false;
    }
};
class BarControl final : public UIControl {
public:
    bool point(NXPoint p,UIEvent*) override {return bounds().insetBy({4,4,4,4}).contains(p);}
    bool canBecomeFocused() override {return isEnabled()&&UIControl::canBecomeFocused();}
    void willGainFocus() override {layer()->setBorderWidth(1);layer()->setBorderColor(UIColor::tint);layer()->setCornerRadius(22);}
    void willLoseFocus() override {layer()->setBorderWidth(0);}
    void willChangeHighlight(bool active) override {setAlpha(active?.5f:1);}
};
const char* systemSymbol(UIBarButtonSystemItem item) {
    switch(item) {
    case UIBarButtonSystemItem::done:return "checkmark";case UIBarButtonSystemItem::add:return "plus";case UIBarButtonSystemItem::compose:return "square.and.pencil";
    case UIBarButtonSystemItem::reply:return "arrowshape.turn.up.left";case UIBarButtonSystemItem::action:return "square.and.arrow.up";
    case UIBarButtonSystemItem::organize:return "folder";case UIBarButtonSystemItem::bookmarks:return "book";
    case UIBarButtonSystemItem::search:return "magnifyingglass";case UIBarButtonSystemItem::refresh:return "arrow.clockwise";
    case UIBarButtonSystemItem::stop:return "stop.fill";case UIBarButtonSystemItem::camera:return "camera";
    case UIBarButtonSystemItem::trash:return "trash";case UIBarButtonSystemItem::play:return "play.fill";
    case UIBarButtonSystemItem::pause:return "pause.fill";case UIBarButtonSystemItem::rewind:return "backward.fill";
    case UIBarButtonSystemItem::fastForward:return "forward.fill";case UIBarButtonSystemItem::undo:return "arrow.uturn.backward";
    case UIBarButtonSystemItem::redo:return "arrow.uturn.forward";default:return "";
    }
}
const char* systemTitle(UIBarButtonSystemItem item) {
    const char* names[]={"Done","Cancel","Edit","Save","+","","","Compose","Reply","Share","Organize","Bookmarks","Search","Refresh","Stop","Camera","Trash","Play","Pause","Rewind","Fast forward","Undo","Redo"};
    return names[int(item)];
}
void validate(const auto& items) {
    std::unordered_set<const UINavigationItem*> unique;
    for(const auto& item:items)if(!item||!unique.insert(item.get()).second)throw std::invalid_argument("Navigation items must be non-null and distinct");
}
constexpr auto animationOptions=UIViewAnimationOptions(curveEaseInOut|beginFromCurrentState|allowUserInteraction|preferredFramesPerSecond120);
// Keep transform decomposition invertible while the absorbed surface is
// smaller than a raster pixel at every supported display scale.
constexpr NXFloat collapsedLeadingScale=.0001f;
}
UINavigationBar::UINavigationBar(NXRect frame):UIView(frame),_state(std::make_unique<Internal::NavigationBarState>()) {
    // Keep the blend reach within the resting inter-capsule gap. A larger
    // kernel pulls even disconnected facing edges out of their circular shape.
    UIGlassContainerEffect effect;effect.setSpacing(12);_state->glass=new_shared<GlassContainerView>(effect);addSubview(_state->glass);
    _state->title=new_shared<UILabel>();_state->title->setFontSize(17);_state->title->setFontWeight(600);
    _state->title->setTextAlignment(NSTextAlignment::center);_state->title->setUserInteractionEnabled(false);
    _state->titleInk=new_shared<Internal::NavigationInkView>();_state->titleInk->setUserInteractionEnabled(false);insertSubviewBelow(_state->titleInk,_state->glass);_state->titleInk->addSubview(_state->title);
    _state->backButton=std::make_shared<UIBarButtonItem>();
    _state->backButton->setIdentifier("UIKit.navigationBar.back");
}
UINavigationBar::~UINavigationBar()=default;
auto UINavigationBar::items() const -> std::vector<std::shared_ptr<UINavigationItem>> {return _state->items;}
auto UINavigationBar::topItem() const -> std::shared_ptr<UINavigationItem> {return _state->items.empty()?nullptr:_state->items.back();}
auto UINavigationBar::backItem() const -> std::shared_ptr<UINavigationItem> {return _state->items.size()<2?nullptr:_state->items[_state->items.size()-2];}
auto UINavigationBar::delegate() const -> std::shared_ptr<UINavigationBarDelegate> {return _state->delegate.lock();}
void UINavigationBar::setDelegate(std::shared_ptr<UINavigationBarDelegate> value) {if(!_state->owner.expired())throw std::logic_error("The navigation controller owns its bar delegate");_state->delegate=value;}
void UINavigationBar::setItems(std::vector<std::shared_ptr<UINavigationItem>> values,bool animated) {
    if(!_state->owner.expired())throw std::logic_error("Configure the navigation controller's stack");validate(values);
    const auto top=topItem();const bool pop=top&&!values.empty()&&std::find(_state->items.begin(),_state->items.end(),values.back())!=_state->items.end();
    install(std::move(values),animated,pop?-1:1);
}
void UINavigationBar::pushNavigationItem(const std::shared_ptr<UINavigationItem>& item,bool animated) {
    if(!_state->owner.expired())throw std::logic_error("Push through the navigation controller");
    auto values=items();values.push_back(item);validate(values);const auto revision=_state->revision;
    const auto target=delegate();if(target&&!target->navigationBarShouldPushItem(shared_from_base<UINavigationBar>(),item))return;
    if(_state->revision!=revision)return;install(std::move(values),animated,1);
    if(target)target->navigationBarDidPushItem(shared_from_base<UINavigationBar>(),item);
}
std::shared_ptr<UINavigationItem> UINavigationBar::popNavigationItem(bool animated) {
    if(!_state->owner.expired())throw std::logic_error("Pop through the navigation controller");
    if(items().empty())return nullptr;const auto item=topItem(),targetItem=item;const auto revision=_state->revision;
    const auto target=delegate();if(target&&!target->navigationBarShouldPopItem(shared_from_base<UINavigationBar>(),item))return item;
    if(_state->revision!=revision)return nullptr;auto values=items();values.pop_back();install(std::move(values),animated,-1);
    if(target)target->navigationBarDidPopItem(shared_from_base<UINavigationBar>(),targetItem);return item;
}
void UINavigationBar::install(std::vector<std::shared_ptr<UINavigationItem>> values,bool animated,int direction) {
    validate(values);_state->items=std::move(values);++_state->revision;
    const auto weak=weak_from_base<UINavigationBar>();
    for(const auto& item:_state->items)Internal::NavigationItemAccess::observe(*item,shared_from_this(),[weak](bool animated){if(const auto bar=weak.lock())bar->refresh(animated);});
    refresh(animated,direction);
}
void UINavigationBar::refresh(bool animated,int direction) {
    if(!_state||_state->refreshing||_state->interactiveSource)return;
    _state->refreshing=true;
    struct RefreshGuard {bool& flag;~RefreshGuard(){flag=false;}} guard{_state->refreshing};
    const auto handoff=_state->interactiveHandoff;_state->interactiveHandoff=-1;
    const auto weak=weak_from_base<UINavigationBar>();const auto top=topItem();
    const auto generation=++_state->generation;const bool animate=animated&&window();
    const bool stackTransition=direction!=0&&top!=_state->renderedItem.lock();_state->renderedItem=top;
    const auto previousLeadingCount=_state->left.groups.size();
    const bool retainedBack=!_state->left.buttons.empty()&&_state->left.buttons.front()->back;
    _state->transitioning=animate;
    _state->glass->layer()->removeAnimation("navigationRetirement");
    // A new navigation operation supersedes the optical tail of the last one.
    // Retired material must not keep participating in a later SDF union.
    for(const auto& weak:_state->retiring)if(const auto view=weak.lock())view->removeFromSuperview();
    _state->retiring.clear();
    _state->interactiveRetiringTitle.reset();
    struct Pose {NXRect frame;NXFloat alpha;};
    std::map<std::shared_ptr<UIView>,Pose> before;
    auto presentedFrame=[&](const std::shared_ptr<UIView>& view) {
        const auto bounds=view->layer()->presentationOrSelf()->bounds();
        NXPoint corners[]={{bounds.minX(),bounds.minY()},{bounds.maxX(),bounds.minY()},{bounds.minX(),bounds.maxY()},{bounds.maxX(),bounds.maxY()}};
        for(auto current=view;current&&current.get()!=this;current=current->superview().lock()) {
            const auto p=current->layer()->presentationOrSelf();const auto size=p->bounds().size;
            for(auto& point:corners)point=(point-p->bounds().origin-NXPoint{size.width*p->anchorPoint().x,size.height*p->anchorPoint().y}).applying(NXTransform3DGetAffineTransform(p->transform()))+p->position();
        }
        NXFloat x=corners[0].x,y=corners[0].y,right=x,bottom=y;
        for(auto point:corners){x=std::min(x,point.x);y=std::min(y,point.y);right=std::max(right,point.x);bottom=std::max(bottom,point.y);}
        return NXRect{x,y,right-x,bottom-y};
    };
    auto remember=[&](const std::shared_ptr<UIView>& view){before[view]={presentedFrame(view),view->layer()->presentationOrSelf()->opacity()};};
    for(const auto* side:{&_state->left,&_state->right}) {
        for(const auto& group:side->groups)remember(group.view);
        for(const auto& button:side->buttons)remember(button->control);
    }
    remember(_state->title);remember(_state->titleInk);
    std::vector<std::shared_ptr<UIView>> retiring;
    std::vector<std::shared_ptr<UIView>> outgoingInk,revealingInk,retiringCarriers;
    std::unordered_set<const UIView*> bornCarriers,replacedCarriers,unwrappedCarriers;
    std::map<const UIView*,std::shared_ptr<UIView>> splitReceivers;
    auto left=top?top->leftBarButtonItems():std::vector<std::shared_ptr<UIBarButtonItem>>{};
    auto right=top?top->rightBarButtonItems():std::vector<std::shared_ptr<UIBarButtonItem>>{};
    if(top&&backItem()&&!top->hidesBackButton()&&(left.empty()||top->leftItemsSupplementBackButton()))left.insert(left.begin(),_state->backButton);
    auto configureSide=[&](Internal::NavigationSide& side,const auto& values,bool rightSide) {
        auto old=side.buttons;side.buttons.clear();auto previous=side.groups;side.groups.clear();side.backgrounds.clear();std::shared_ptr<UIView> fallbackReceiver;
        for(const auto& item:values) {
            // Automatic Back is one persistent semantic item. An application
            // identifier must never let a custom action adopt its presentation.
            const bool back=item==_state->backButton;
            auto found=std::find_if(old.begin(),old.end(),[&](const auto& entry){return entry->back==back&&(entry->item==item||(item->identifier()&&entry->item->identifier()==item->identifier()));});
            std::shared_ptr<Internal::NavigationButton> entry;
            if(found!=old.end()){entry=*found;old.erase(found);}else {
                entry=std::make_shared<Internal::NavigationButton>();entry->control=new_shared<BarControl>();
                entry->ink=new_shared<Internal::NavigationInkView>();entry->control->addSubview(entry->ink);
                entry->label=new_shared<UILabel>();entry->label->setFontSize(17);entry->label->setTextAlignment(NSTextAlignment::center);entry->label->setUserInteractionEnabled(false);
                entry->image=new_shared<UIImageView>();entry->image->setContentMode(UIViewContentMode::scaleAspectFit);entry->image->setUserInteractionEnabled(false);
                entry->ink->addSubview(entry->image);entry->ink->addSubview(entry->label);revealingInk.push_back(entry->ink);
            }
            entry->item=item;entry->back=item==_state->backButton;
            const auto system=Internal::BarItemAccess::systemItem(*item);
            entry->spacer=system&&(system==UIBarButtonSystemItem::fixedSpace||system==UIBarButtonSystemItem::flexibleSpace);
            // Modern UIKit uses label ink for ordinary navigation items,
            // even when the bar inherits an application accent color.
            const auto color=item->tintColor().value_or(UIColor::label);
            entry->control->setEnabled(item->isEnabled()&&!entry->spacer);entry->control->setUserInteractionEnabled(true);
            entry->control->setAlpha(item->isEnabled()?1:.4f);entry->control->setHidden(entry->spacer);
            auto image=item->image();std::string title=item->title();
            if(entry->back){image=UIImage::systemImageNamed("chevron.left",20,600,traitCollection()?traitCollection()->displayScale():1);title="‹";}
            else if(system){title=systemTitle(*system);image=UIImage::systemImageNamed(systemSymbol(*system),20,400,traitCollection()?traitCollection()->displayScale():1);}
            // UIKit updates matched identifiers in place, even when their
            // symbol changes. New/reparented items receive the blurred reveal.
            entry->image->setImage(image);entry->image->setTintColor(item->style()==UIBarButtonItemStyle::prominent?UIColor::white:color);
            entry->label->setText(image?"":title);entry->label->setTextColor(item->style()==UIBarButtonItemStyle::prominent?UIColor::white:color);
            entry->label->setFontWeight(item->style()==UIBarButtonItemStyle::prominent?600:500);
            if(entry->custom!=item->customView()) {if(entry->custom)entry->custom->removeFromSuperview();entry->custom=item->customView();if(entry->custom){if(entry->custom.get()==this||isDescendantOf(entry->custom))throw std::invalid_argument("Bar custom view forms a cycle");entry->ink->addSubview(entry->custom);}}
            entry->label->setHidden(bool(entry->custom));entry->image->setHidden(bool(entry->custom));
            entry->control->primaryAction=UIAction(title,[weak,item,back=entry->back]{if(const auto bar=weak.lock())bar->activate(item,back);});
            Internal::BarItemAccess::observe(*item,shared_from_this(),[weak]{if(const auto bar=weak.lock())bar->refresh();});
            side.buttons.push_back(entry);
            if(entry->spacer)continue;
            const bool separate=entry->back||item->style()==UIBarButtonItemStyle::prominent||item->hidesSharedBackground()||!item->sharesBackground()||(!image&&!entry->custom);
            const bool afterSpace=side.buttons.size()>1&&side.buttons[side.buttons.size()-2]->spacer;
            const bool previousSeparate=!side.groups.empty()&&(side.groups.back().hiddenBackground||side.groups.back().tint.has_value()||!side.groups.back().buttons.back()->item->sharesBackground()||side.groups.back().buttons.back()->back||(!side.groups.back().buttons.back()->image->image()&&!side.groups.back().buttons.back()->custom));
            if(side.groups.empty()||separate||previousSeparate||afterSpace)side.groups.push_back({});
            auto& group=side.groups.back();group.buttons.push_back(entry);group.hiddenBackground=item->hidesSharedBackground();
            if(item->style()==UIBarButtonItemStyle::prominent)group.tint=item->tintColor().value_or(tintColor());
        }
        for(const auto& entry:old){entry->control->setUserInteractionEnabled(false);retiring.push_back(entry->control);outgoingInk.push_back(entry->ink);}
        // Match groups by shared items first. Ties are resolved in visual order,
        // as observed when one shared capsule divides into multiple capsules.
        std::vector<size_t> order;for(size_t i=0;i<side.groups.size();++i)order.push_back(i);if(rightSide)std::reverse(order.begin(),order.end());
        for(auto index:order) {
            auto& group=side.groups[index];int best=-1,score=-1,priority=-1;
            for(size_t i=0;i<previous.size();++i)if(previous[i].view&&previous[i].hiddenBackground==group.hiddenBackground) {
                // A surviving visual-leading member keeps its capsule. If
                // that member was removed, UIKit falls back to the first item
                // in array order. This matters when three items split into two.
                const auto visual=rightSide?previous[i].buttons.back():previous[i].buttons.front();
                auto preferred=visual;
                const auto survives=[&](const auto& entry){return std::find(side.buttons.begin(),side.buttons.end(),entry)!=side.buttons.end();};
                if(!survives(preferred))for(const auto& entry:previous[i].buttons)if(survives(entry)){preferred=entry;break;}
                if(survives(preferred)&&std::find(group.buttons.begin(),group.buttons.end(),preferred)==group.buttons.end())continue;
                int shared=0;for(const auto& a:group.buttons)for(const auto& b:previous[i].buttons)shared+=a==b;
                const int expanded=handoff>=0&&before.at(previous[i].view).frame.width()>previous[i].view->frame().width()+.5f;
                if(shared>score||(shared==score&&(expanded>priority||(expanded==priority&&rightSide)))){score=shared;priority=expanded;best=int(i);}
            }
            if(best>=0){group.view=previous[best].view;
                const auto& prior=previous[best];const auto visual=rightSide?prior.buttons.back():prior.buttons.front();
                const bool noRemoval=std::all_of(prior.buttons.begin(),prior.buttons.end(),[&](const auto& entry){return std::find(side.buttons.begin(),side.buttons.end(),entry)!=side.buttons.end();});
                if(animate&&prior.buttons.size()>group.buttons.size()&&noRemoval&&std::find(group.buttons.begin(),group.buttons.end(),visual)!=group.buttons.end())unwrappedCarriers.insert(group.view.get());
                if(animate&&!noRemoval&&prior.buttons.size()>group.buttons.size()&&side.groups.size()>1)fallbackReceiver=group.view;
                previous[best].view.reset();if(score==0)replacedCarriers.insert(group.view.get());}
            else {
                if(group.hiddenBackground)group.view=new_shared<UIView>();
                else {UIGlassEffect effect;effect.setInteractive(true);group.view=new_shared<NavigationGlassView>(effect);group.view->layer()->setCornerRadius(22);}
                _state->glass->contentView()->addSubview(group.view);
                bornCarriers.insert(group.view.get());
            }
            if(const auto glass=std::dynamic_pointer_cast<BackdropEffectView>(group.view)) {
                auto effect=*glass->glassEffect();if(effect.tintColor()!=group.tint){effect.setTintColor(group.tint);glass->setEffect(effect);}
            }
            for(const auto& entry:group.buttons)if(entry->control->superview().lock()!=group.view){group.view->addSubview(entry->control);revealingInk.push_back(entry->ink);}
        }
        if(fallbackReceiver)for(const auto& group:side.groups)if(bornCarriers.contains(group.view.get()))splitReceivers[group.view.get()]=fallbackReceiver;
        for(const auto& group:previous)if(group.view){group.view->setUserInteractionEnabled(false);retiring.push_back(group.view);retiringCarriers.push_back(group.view);}
        for(const auto& group:side.groups)if(const auto glass=std::dynamic_pointer_cast<BackdropEffectView>(group.view))side.backgrounds.push_back(glass);
    };
    configureSide(_state->left,left,false);configureSide(_state->right,right,true);
    if(_state->customTitle!=(top?top->titleView():nullptr)) {
        if(_state->customTitle&&handoff<0)_state->customTitle->removeFromSuperview();_state->customTitle=top?top->titleView():nullptr;
        if(_state->customTitle){if(_state->customTitle.get()==this||isDescendantOf(_state->customTitle))throw std::invalid_argument("Title view forms a cycle");if(handoff<0)insertSubviewBelow(_state->customTitle,_state->glass);}
    }
    const auto oldTitle=_state->title;const auto oldTitleInk=_state->titleInk;
    if(handoff>=0&&_state->interactiveTitle) {
        // The gesture already owns both title carriers and their reveal mask.
        // Retain those exact animations on the content's reversible clock;
        // only the glass/ink grouping changes to its committed appearance.
        _state->titleInk=_state->interactiveTitle;_state->title=_state->interactiveTitleLabel;
        _state->interactiveRetiringTitle=oldTitleInk;
        retiring.push_back(oldTitleInk);
    }else if(oldTitle->text()!=(top?top->title():"")||(animate&&stackTransition)) {
        auto title=new_shared<UILabel>();title->setFontSize(17);title->setFontWeight(600);title->setTextAlignment(NSTextAlignment::center);title->setUserInteractionEnabled(false);
        title->setText(top?top->title():"");_state->titleInk=new_shared<Internal::NavigationInkView>();_state->titleInk->setUserInteractionEnabled(false);insertSubviewBelow(_state->titleInk,_state->glass);_state->titleInk->addSubview(title);_state->title=title;retiring.push_back(oldTitleInk);
    }
    _state->title->setHidden(bool(_state->customTitle));
    const bool nextBack=!_state->left.buttons.empty()&&_state->left.buttons.front()->back;
    const bool leadingChange=(retainedBack||nextBack)&&previousLeadingCount>0&&!_state->left.groups.empty()&&previousLeadingCount!=_state->left.groups.size();
    const bool rootLeadingChange=leadingChange&&(!retainedBack||!nextBack);
    const bool leadingAppears=leadingChange&&_state->left.groups.size()>previousLeadingCount;
    UIView::performWithoutAnimation([&]{arrange();});
    std::map<const UIView*,NXFloat> splitCenters;for(const auto& [born,receiver]:splitReceivers)splitCenters[born]=receiver->frame().midX();
    if(!animate){
        for(const auto& view:retiring)view->removeFromSuperview();
        for(const auto* side:{&_state->left,&_state->right}) {
            for(const auto& group:side->groups){group.view->layer()->removeAllAnimations();group.view->setAlpha(1);group.view->setTransform(NXAffineTransform::identity);}
            for(const auto& button:side->buttons){button->control->layer()->removeAllAnimations();button->ink->layer()->removeAllAnimations();button->ink->setAlpha(1);}
        }
        _state->title->layer()->removeAllAnimations();_state->title->setAlpha(1);_state->title->setTransform(NXAffineTransform::identity);
        _state->titleInk->layer()->removeAllAnimations();_state->titleInk->setAlpha(1);_state->titleInk->setTransform(NXAffineTransform::identity);
        setNeedsLayout();return;
    }
    for(const auto& view:retiring)_state->retiring.push_back(view);
    // Each carrier and its descendants start in one coordinate system. A split
    // births a small capsule at the new item's center; a join keeps a carrier
    // and absorbs the retiring one, rather than cross-fading disjoint layers.
    std::map<std::shared_ptr<UIView>,Pose> targets,starts;
    for(const auto* side:{&_state->left,&_state->right})for(const auto& group:side->groups) {
        const auto view=group.view;targets[view]={view->frame(),1};view->setUserInteractionEnabled(true);
        const bool born=!before.contains(view);
        if(born){auto initial=view->frame();if(splitCenters.contains(view.get()))initial.origin.x=splitCenters.at(view.get())-initial.width()*.5f;else if(!group.buttons.front()->back&&initial.midX()>bounds().width()*.5f)initial.origin.x+=4;before[view]={initial,1};}
        if(unwrappedCarriers.contains(view.get()))before.at(view).frame.size.width=view->frame().width();
        const auto pose=before.at(view);UIView::performWithoutAnimation([&]{view->layer()->removeAllAnimations();view->setFrame(pose.frame);view->setAlpha(pose.alpha);});
        if(born)view->setTransform(NXAffineTransform::scale(group.buttons.front()->back?1.2f:.2f));
        for(const auto& button:group.buttons) {
            const auto control=button->control;targets[control]={control->frame(),button->item->isEnabled()?1.f:.4f};
            auto local=control->frame();
            if(before.contains(control)){local=before.at(control).frame;local.origin-=pose.frame.origin;}
            else local.origin.x+=direction*12;
            // A joining item used to live in another capsule. Bring its ink
            // into the receiving capsule before revealing it; otherwise the
            // glyph floats outside that material while the glass expands.
            const auto oldCenter=local.midX();
            const auto center=std::clamp(oldCenter,std::min(local.width()*.5f+4,pose.frame.width()*.5f),std::max(pose.frame.width()*.5f,pose.frame.width()-local.width()*.5f-4));
            local.origin.x+=center-oldCenter;
            const auto alpha=button->item->isEnabled()?1.f:.4f;
            UIView::performWithoutAnimation([&]{control->layer()->removeAllAnimations();control->setFrame(local);control->setAlpha(alpha);});
            starts[control]={local,control->alpha()};
        }
    }
    const auto titleTarget=_state->title->frame();
    if(before.contains(_state->title))_state->title->setFrame(before.at(_state->title).frame);
    else {_state->title->setAlpha(0);_state->title->setTransform(NXAffineTransform::translationBy(direction*24,0));}
    for(const auto& ink:revealingInk)UIView::performWithoutAnimation([&]{ink->layer()->removeAllAnimations();ink->setAlpha(0);});
    UIView::performWithoutAnimation([&]{for(const auto& [view,pose]:targets){view->setTransform(NXAffineTransform::identity);view->setFrame(pose.frame);view->setAlpha(pose.alpha);}});
    // A dedicated clock owns retirement. Replacing implicit geometry animations
    // with the fitted springs must not fire their cleanup completion early.
    auto retirement=Internal::navigationAnimation("opacity");retirement->duration=.97;retirement->fromValue=1.f;retirement->toValue=1.f;
    retirement->animationGroup=new_shared<UIViewAnimationGroup>(animationOptions,[weak,generation,retiring](bool){for(const auto& view:retiring)view->removeFromSuperview();if(const auto bar=weak.lock();bar&&bar->_state->generation==generation)bar->_state->transitioning=false;});
    _state->glass->layer()->add(retirement,"navigationRetirement");
    if(stackTransition) {
        _state->title->setAlpha(1);_state->title->setTransform(NXAffineTransform::identity);_state->title->setFrame(titleTarget);
        if(handoff<0) {
            const auto width=bounds().width();
            const auto incoming=NXAffineTransform::translationBy(direction<0?-width*.3f:width,0);
            const auto outgoing=NXAffineTransform::translationBy(direction<0?width:-width*.3f,0);
            UIView::performWithoutAnimation([&]{_state->titleInk->setTransform(NXAffineTransform::identity);oldTitleInk->setTransform(outgoing);});
            for(const auto& host:{_state->titleInk,oldTitleInk}) {
                auto slide=Internal::navigationAnimation("transform");slide->duration=Internal::navigationContentDuration;
                slide->fromValue=NXTransform3DMakeAffineTransform(host==oldTitleInk?NXAffineTransform::identity:incoming);
                slide->toValue=NXTransform3DMakeAffineTransform(host==oldTitleInk?outgoing:NXAffineTransform::identity);
                slide->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){return fraction>=1?1.:Internal::navigationContentProgress(fraction*Internal::navigationContentDuration);});
                host->layer()->add(slide,"transform");
            }
            // Titles follow the same moving visibility boundary as their content:
            // pushing wipes away the outgoing title, popping reveals the incoming
            // one. A center cross-fade leaves unrelated titles superimposed.
            const auto clipped=direction<0?_state->titleInk:oldTitleInk;
            const auto height=bounds().height();clipped->setClipsToBounds(true);
            UIView::performWithoutAnimation([&]{clipped->layer()->setAnchorPoint({0,0});clipped->layer()->setPosition({0,0});clipped->setBounds({0,0,direction<0?width:width*.3f,height});});
            auto wipe=Internal::navigationAnimation("bounds");wipe->duration=Internal::navigationContentDuration;wipe->fromValue=NXRect{0,0,direction<0?width*.3f:width,height};wipe->toValue=NXRect{0,0,direction<0?width:width*.3f,height};wipe->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){return fraction>=1?1.:Internal::navigationContentProgress(fraction*Internal::navigationContentDuration);});clipped->layer()->add(wipe,"bounds");
        }
        auto gate=Internal::navigationAnimation("opacity");gate->fromValue=1.f;gate->toValue=1.f;gate->duration=Internal::navigationContentDuration;
        gate->animationGroup=new_shared<UIViewAnimationGroup>(animationOptions,[weak,generation](bool){if(const auto bar=weak.lock();bar&&bar->_state->generation==generation)bar->_state->transitioning=false;});_state->titleInk->layer()->add(gate,"navigationInputGate");
    } else {
        UIView::animate(.35,0,animationOptions,[&]{_state->title->setFrame(titleTarget);_state->title->setTransform(NXAffineTransform::identity);_state->title->setAlpha(1);if(oldTitle!=_state->title)oldTitleInk->setAlpha(0);},[weak,generation](bool){if(const auto bar=weak.lock();bar&&bar->_state->generation==generation)bar->_state->transitioning=false;});
    }
    for(const auto& ink:outgoingInk){
        const auto control=ink->superview().lock();const bool supplemental=leadingChange&&control&&before.contains(control)&&before.at(control).frame.midX()>60&&before.at(control).frame.midX()<bounds().width()*.5f;
        const auto delay=supplemental?(rootLeadingChange?.2:.1):.1,duration=supplemental?.22:.1;
        Internal::navigationInkBlur(ink,0,supplemental?8:4,duration,delay);auto fade=Internal::navigationAnimation("opacity");fade->fromValue=ink->layer()->presentationOrSelf()->opacity();fade->toValue=0.f;fade->duration=duration;fade->delay=delay;ink->setAlpha(0);ink->layer()->add(fade,"opacity");}
    for(const auto& ink:revealingInk){Internal::navigationInkBlur(ink,8,0,.24,.10);auto fade=Internal::navigationAnimation("opacity");fade->fromValue=0.f;fade->toValue=1.f;fade->duration=.18;fade->delay=.12;ink->setAlpha(1);ink->layer()->add(fade,"opacity");}
    // A redundant capsule remains a real SDF participant while moving into
    // the receiving capsule. Its material disappears by overlap, not opacity.
    for(const auto& view:retiringCarriers) {
        const auto from=before.at(view).frame;
        if(leadingChange&&from.midX()<bounds().width()*.5f) {
            auto motion=Internal::navigationAnimation("position");motion->duration=rootLeadingChange?.5:.4;motion->fromValue=NXPoint{from.midX(),from.midY()};motion->toValue=NXPoint{before.at(_state->left.groups.front().view).frame.midX(),from.midY()};motion->timingFunction=new_shared<CAMediaTimingFunction>([rootLeadingChange](double f){return Internal::navigationLeadingAbsorption(f*(rootLeadingChange?.5:.4)-(rootLeadingChange?.1:0));});view->layer()->add(motion,"position");
            // Keep overlapping material opaque: fading the foremost surface
            // reveals the black backdrop through Back instead of absorbing it.
            UIView::performWithoutAnimation([&]{view->setTransform(NXAffineTransform::scale(collapsedLeadingScale));view->setAlpha(1);});
            auto shape=Internal::navigationAnimation("transform");shape->duration=motion->duration;shape->fromValue=NXTransform3D::identity;shape->toValue=NXTransform3DMakeAffineTransform(NXAffineTransform::scale(collapsedLeadingScale));shape->timingFunction=motion->timingFunction;view->layer()->add(shape,"transform");
            continue;
        }
        std::shared_ptr<UIView> receiver;
        NXFloat distance=std::numeric_limits<NXFloat>::max();
        for(const auto* side:{&_state->left,&_state->right})for(const auto& group:side->groups){const auto target=targets.at(group.view).frame;if((target.midX()<bounds().width()*.5f)!=(from.midX()<bounds().width()*.5f))continue;const auto d=std::abs(target.midX()-from.midX());if(d<distance){distance=d;receiver=group.view;}}
        if(!receiver){UIView::animate(.15,0,animationOptions,[&]{view->setAlpha(0);});continue;}const auto to=targets.at(receiver).frame,source=before.at(receiver).frame;const NXPoint start{from.midX(),from.midY()},end{to.midX(),to.midY()};
        UIView::performWithoutAnimation([&]{view->setAlpha(1);view->setFrame({end.x-from.width()*.5f,end.y-from.height()*.5f,from.width(),from.height()});});
        auto motion=Internal::navigationAnimation("position");motion->duration=.85;motion->fromValue=start;motion->toValue=end;
        const auto first=source.midX();
        motion->timingFunction=new_shared<CAMediaTimingFunction>([start,end,first](double fraction){if(fraction<=0)return 0.;if(fraction>=1)return 1.;const auto t=fraction*.85,u=std::clamp((t-.01)/.025,0.,1.);const auto merge=u*u*(3-2*u);const auto x=start.x+(first-start.x)*merge+(end.x-first)*Internal::navigationSpring(t-.105);return std::abs(end.x-start.x)>.001?(x-start.x)/(end.x-start.x):1.;});view->layer()->add(motion,"position");
        const auto glass=std::dynamic_pointer_cast<BackdropEffectView>(view);const bool tinted=glass&&glass->glassEffect()->tintColor().has_value();
        auto absorption=Internal::navigationAnimation("opacity");absorption->duration=.035;absorption->fromValue=1.f;absorption->toValue=tinted?0.f:1.f;
        // Once fully covered, remove this participant before the receiver's
        // undershoot. A lingering 44-point circle would flatten that rebound.
        absorption->animationGroup=new_shared<UIViewAnimationGroup>(animationOptions,[weakView=std::weak_ptr<UIView>(view)](bool){if(const auto view=weakView.lock()){view->removeFromSuperview();view->layer()->removeAllAnimations();}});
        view->layer()->add(absorption,"navigationAbsorption");
    }
    for(const auto* side:{&_state->left,&_state->right})for(const auto& group:side->groups) {
        const auto view=group.view;const auto from=before.at(view).frame,to=targets.at(view).frame;
        for(const auto* key:{"bounds","position"}) {
            const bool size=std::string(key)=="bounds";auto animation=Internal::navigationAnimation(key);animation->duration=.85;
            const auto delta=to.width()-from.width();const bool expanding=size&&delta>0;
            animation->delay=.105;
            const bool born=bornCarriers.contains(view.get());
            const bool displacedBirth=!size&&splitCenters.contains(view.get());if(displacedBirth)animation->delay=.14;
            animation->timingFunction=new_shared<CAMediaTimingFunction>([size,expanding,born,displacedBirth](double fraction) {
                if(fraction<=0)return 0.0;if(fraction>=1)return 1.0;
                const auto t=fraction*.85;
                if(displacedBirth)return Internal::navigationSpring(t,.81,23);
                const auto base=Internal::navigationSpring(t,size?(expanding?.72:.81):.72,16);
                return base+(!size&&born?2*Internal::navigationSurfacePulse(t):0);
            });
            if(size){animation->fromValue=NXRect{{},from.size};animation->toValue=NXRect{{},to.size};}
            else {animation->fromValue=NXPoint{from.midX(),from.midY()};animation->toValue=NXPoint{to.midX(),to.midY()};}
            view->layer()->add(animation,key);
            if(size)for(const auto& button:group.buttons) {
                const auto control=button->control;const auto start=starts.at(control).frame,end=targets.at(control).frame;
                // Content uses the capsule's size clock, including its delay
                // and overshoot. A trailing glyph remains inside a shrinking
                // trailing edge instead of following an unrelated fade clock.
                for(const auto* property:{"position","bounds"}) {
                    auto ink=Internal::navigationAnimation(property);ink->duration=animation->duration;ink->delay=animation->delay;ink->timingFunction=animation->timingFunction;
                    if(std::string(property)=="position"){ink->fromValue=NXPoint{start.midX(),start.midY()};ink->toValue=NXPoint{end.midX(),end.midY()};}
                    else {ink->fromValue=NXRect{{},start.size};ink->toValue=NXRect{{},end.size};}
                    control->layer()->add(ink,property);
                }
            }
        }
        if(from!=to||bornCarriers.contains(view.get())||replacedCarriers.contains(view.get())||unwrappedCarriers.contains(view.get())) {
            auto shape=Internal::navigationAnimation("transform");shape->duration=.85;shape->delay=.10;
            if(bornCarriers.contains(view.get())) {
                const bool back=group.buttons.size()==1&&group.buttons.front()->back;
                shape->fromValue=NXTransform3DMakeAffineTransform(NXAffineTransform::scale(back?1.2f:.2f));shape->toValue=NXTransform3D::identity;
                shape->delay=.11;shape->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){if(fraction<=0)return 0.;if(fraction>=1)return 1.;return Internal::navigationSpring(fraction*.85,.665,21.4);});
                auto reveal=Internal::navigationAnimation("opacity");reveal->duration=.01;reveal->delay=.08;reveal->fromValue=0.f;reveal->toValue=1.f;view->layer()->add(reveal,"opacity");
            } else {
                shape->fromValue=NXTransform3D::identity;auto inflation=NXAffineTransform::scale(1.285f);
                if(unwrappedCarriers.contains(view.get())){inflation.tX=-to.width()*.5f*.285f;inflation.tY=to.height()*.5f*.285f;}
                shape->toValue=NXTransform3DMakeAffineTransform(inflation);
                shape->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction){if(fraction<=0||fraction>=1)return 0.;return Internal::navigationSurfacePulse(fraction*.85);});
            }
            view->layer()->add(shape,"transform");
        }
    }
    if(leadingChange&&nextBack) {
        const auto back=_state->left.groups.front().view;const auto rest=targets.at(back).frame;
        if(handoff<0)back->layer()->removeAllAnimations();
        else {back->layer()->removeAnimation("transform");back->layer()->removeAnimation("opacity");}
        // Inflating about the leading/top edge preserves UIKit's back anchor.
        auto pulse=Internal::navigationAnimation("transform");pulse->duration=.97;pulse->fromValue=NXTransform3D::identity;
        auto inflated=NXAffineTransform::scale(2);inflated.tX=rest.width()*.5f;inflated.tY=rest.height()*.5f;
        pulse->toValue=NXTransform3DMakeAffineTransform(inflated);pulse->timingFunction=new_shared<CAMediaTimingFunction>([](double f){return Internal::navigationLeadingInflation(f*.97);});back->layer()->add(pulse,"transform");
        if(leadingAppears&&handoff<0) {
            auto drift=Internal::navigationAnimation("position");drift->duration=.97;drift->fromValue=NXPoint{rest.midX(),rest.midY()};drift->toValue=NXPoint{rest.midX()+7,rest.midY()};drift->timingFunction=new_shared<CAMediaTimingFunction>([](double f){return Internal::navigationLeadingDisplacement(f*.97)/7;});back->layer()->add(drift,"position");
        }
        const auto backInk=_state->left.buttons.front()->ink;
        auto blur=Internal::navigationAnimation("navigationInkBlur");blur->duration=.55;blur->fromValue=0.f;blur->toValue=8.f;blur->timingFunction=new_shared<CAMediaTimingFunction>([](double f){return Internal::navigationCurve(f*.55,{{0,0},{.1,0},{.15,.25},{.2,1},{.3,1},{.4,.4},{.475,0},{.55,0}});});backInk->layer()->add(blur,"navigationInkBlur");
        if(leadingAppears)for(size_t i=1;i<_state->left.groups.size();++i) {
            const auto& group=_state->left.groups[i];const auto glass=group.view;const auto target=targets.at(glass).frame;
            const auto revealOffset=(rootLeadingChange||handoff>=0)?.2:0.;
            glass->layer()->removeAllAnimations();
            auto travel=Internal::navigationAnimation("position");travel->duration=.97;travel->fromValue=NXPoint{handoff<0?rest.midX():before.at(back).frame.midX(),target.midY()};travel->toValue=NXPoint{target.midX(),target.midY()};travel->timingFunction=new_shared<CAMediaTimingFunction>([revealOffset](double f){return Internal::navigationLeadingTravel(f*.97+revealOffset);});glass->layer()->add(travel,"position");
            glass->setAlpha(1);
            auto shape=Internal::navigationAnimation("transform");shape->duration=.97;shape->fromValue=NXTransform3DMakeAffineTransform(NXAffineTransform::scale(collapsedLeadingScale));shape->toValue=NXTransform3D::identity;shape->timingFunction=new_shared<CAMediaTimingFunction>([revealOffset](double f){return std::max(0.,Internal::navigationLeadingReveal(f*.97+revealOffset));});glass->layer()->add(shape,"transform");
            for(const auto& button:group.buttons) {
                button->ink->layer()->removeAllAnimations();button->ink->setAlpha(1);
                Internal::navigationInkBlur(button->ink,8,0,.28,revealOffset>0?.07:.27);
                // Ink must not accumulate on Back before its own surface
                // emerges. Its reveal is independent of material opacity.
                auto reveal=Internal::navigationAnimation("opacity");reveal->duration=.55;reveal->fromValue=0.f;reveal->toValue=1.f;
                reveal->timingFunction=new_shared<CAMediaTimingFunction>([revealOffset](double f){return std::clamp(Internal::navigationLeadingReveal(f*.55+revealOffset),0.,1.);});
                button->ink->layer()->add(reveal,"opacity");
            }
        }
    }
    // Explicit animations must expose their source pose immediately, including
    // when another navigation operation arrives before the next display tick.
    const auto now=Timer();std::function<void(const std::shared_ptr<CALayer>&)> prime=[&](const auto& layer){const auto children=layer->sublayers();layer->animateAt(now);for(const auto& child:children)prime(child);};prime(layer());
    setNeedsLayout();
}
void UINavigationBar::arrange() {
    if(!_state)return;auto& s=*_state;s.titleInk->setFrame(bounds());const auto size=bounds().size;const auto safe=safeAreaInsets();s.glass->setFrame(bounds());
    const auto margin=16.f,available=std::max(0.f,size.width-safe.left-safe.right-2*margin);
    auto layoutSide=[&](Internal::NavigationSide& side,bool right,NXFloat budget) {
        NXFloat used=0;
        for(auto& group:side.groups) {
            const auto padding=4.f;NXFloat width=2*padding;
            for(const auto& entry:group.buttons) {
                const auto item=entry->item;const auto image=entry->image->image();
                entry->width=item->width()>0?item->width():(entry->custom?std::max(36.f,entry->custom->sizeThatFits({1000,44}).width):(entry->back?36.f:(image?(group.buttons.size()==1?36.f:std::max(36.f,image->size().width+12)):std::max(36.f,std::ceil(entry->label->sizeThatFits({1000,44}).width*3)/3+24))));
                width+=entry->width;
            }
            width+=std::max(0,int(group.buttons.size())-1)*16;
            // Independent sides may use all space until they meet the center.
            const bool visible=used+width<=budget;
            group.view->setHidden(!visible);if(!visible)continue;
            const auto x=right?size.width-safe.right-margin-used-width:safe.left+margin+used;
            group.view->setFrame({x,0,width,44});NXFloat offset=padding;
            for(const auto& entry:group.buttons) {
                const auto localX=right?width-offset-entry->width:offset;
                entry->control->setFrame({localX,0,entry->width,44});entry->control->setHidden(false);
                entry->ink->setFrame({0,0,entry->width,44});
                entry->label->setFrame({0,0,entry->width,44});if(entry->back)entry->label->setFontSize(30);
                if(const auto image=entry->image->image()){const auto intrinsic=image->size();entry->image->setFrame({(entry->width-intrinsic.width)*.5f,(44-intrinsic.height)*.5f,intrinsic.width,intrinsic.height});}
                if(entry->custom)entry->custom->setFrame({0,0,entry->width,44});offset+=entry->width+16;
            }
            used+=width+12;
        }
        return used>0?used-12:0.f;
    };
    const auto right=layoutSide(s.right,true,available*.7f),left=layoutSide(s.left,false,std::max(0.f,available-right-20));
    const auto titleLeft=safe.left+margin+left+(left>0?12:0),titleRight=size.width-safe.right-margin-right-(right>0?12:0);
    const auto room=std::max(0.f,titleRight-titleLeft),titleWidth=std::min(room,s.title->sizeThatFits({10000,44}).width);
    const auto trailingCount=std::count_if(s.right.buttons.begin(),s.right.buttons.end(),[](const auto& item){return !item->spacer&&!item->control->isHidden();});
    // UIKit adopts leading title alignment for a dense trailing button row;
    // otherwise it centers the title, shifting only to clear leading content.
    const auto centered=std::clamp((size.width-titleWidth)*.5f,titleLeft,std::max(titleLeft,titleRight-titleWidth));
    s.title->setFrame({trailingCount>=3?titleLeft:centered,0,titleWidth,44});
    if(s.customTitle){const auto w=std::min(room,s.customTitle->sizeThatFits({room,44}).width);s.customTitle->setFrame({std::clamp((size.width-w)*.5f,titleLeft,std::max(titleLeft,titleRight-w)),0,w,44});}
}
void UINavigationBar::activate(const std::shared_ptr<UIBarButtonItem>& item,bool back) {
    if(_state->transitioning||!isActionDispatchAllowed()||!item->isEnabled())return;
    if(back){if(_state->backAction){const auto action=_state->backAction;action();}else popNavigationItem(true);return;}
    const auto present=[&](const auto& side){return std::any_of(side.buttons.begin(),side.buttons.end(),[&](const auto& button){return button->item==item;});};
    if(!present(_state->left)&&!present(_state->right))return;
    if(const auto action=item->primaryAction())action->perform();
}
void UINavigationBar::layoutSubviews(){UIView::layoutSubviews();arrange();}
void UINavigationBar::tintColorDidChange(){UIView::tintColorDidChange();if(_state)refresh();}
void UINavigationBar::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous){UIView::traitCollectionDidChange(std::move(previous));if(_state)refresh();}
NXSize UINavigationBar::sizeThatFits(NXSize size){return {std::max(0.0f,size.width),traitCollection()&&traitCollection()->verticalSizeClass()==UIUserInterfaceSizeClass::compact?44.0f:54.0f};}
bool UINavigationBar::point(NXPoint p,UIEvent*) {return bounds().contains(p)&&bool(topItem());}
std::vector<std::shared_ptr<UIFocusEnvironment>> UINavigationBar::preferredFocusEnvironments(){std::vector<std::shared_ptr<UIFocusEnvironment>> views;for(const auto* side:{&_state->left,&_state->right})for(const auto& entry:side->buttons)if(entry->control->canBecomeFocused())views.push_back(entry->control);return views;}

namespace {
// Layout-only copies never reparent application-owned custom views into the
// measuring bar. They preserve UIKit grouping metadata and intrinsic sizes.
std::shared_ptr<UINavigationItem> previewItem(const std::shared_ptr<UINavigationItem>& source,std::map<const UIBarButtonItem*,std::shared_ptr<UIBarButtonItem>>& originals) {
    auto result=std::make_shared<UINavigationItem>(source->title());
    result->setHidesBackButton(source->hidesBackButton());result->setLeftItemsSupplementBackButton(source->leftItemsSupplementBackButton());
    auto copy=[&](const auto& items) {
        std::vector<std::shared_ptr<UIBarButtonItem>> result;
        for(const auto& item:items) {
            auto clone=Internal::BarItemAccess::systemItem(*item)?std::make_shared<UIBarButtonItem>(*Internal::BarItemAccess::systemItem(*item)):std::make_shared<UIBarButtonItem>();
            clone->setTitle(item->title());clone->setImage(item->image());clone->setIdentifier(item->identifier());clone->setStyle(item->style());clone->setWidth(item->width());clone->setSharesBackground(item->sharesBackground());clone->setHidesSharedBackground(item->hidesSharedBackground());
            if(const auto custom=item->customView())clone->setCustomView(new_shared<UIView>(NXRect{{},custom->sizeThatFits({1000,44})}));
            originals.emplace(clone.get(),item);
            result.push_back(clone);
        }
        return result;
    };
    result->setLeftBarButtonItems(copy(source->leftBarButtonItems()));result->setRightBarButtonItems(copy(source->rightBarButtonItems()));
    if(const auto custom=source->titleView())result->setTitleView(new_shared<UIView>(NXRect{{},custom->sizeThatFits({1000,44})}));
    return result;
}
bool sameInteractiveButton(const std::shared_ptr<Internal::NavigationButton>& a,const std::shared_ptr<Internal::NavigationButton>& b) {
    if(a->back||b->back)return a->back&&b->back;
    return a->item==b->item||(a->item->identifier()&&a->item->identifier()==b->item->identifier());
}
void interactiveProperty(const std::shared_ptr<UIView>& view,const char* key,AnimatableProperty from,AnimatableProperty to,double span) {
    auto animation=Internal::navigationAnimation(key);animation->duration=span;animation->fromValue=std::move(from);animation->toValue=std::move(to);animation->timingFunction=new_shared<CAMediaTimingFunction>([](double f){return f;});view->layer()->add(animation,key);
}
}
void Internal::NavigationBarAccess::beginInteractivePop(UINavigationBar& bar,std::vector<std::shared_ptr<UINavigationItem>> items,double span) {
    auto& s=*bar._state;
    // Finish the previous operation's optical tail before capturing this one.
    // Its source metadata is unchanged; no target styling is installed yet.
    bar.refresh(false);
    auto measure=new_shared<UINavigationBar>(bar.bounds());std::map<const UIBarButtonItem*,std::shared_ptr<UIBarButtonItem>> originals;
    std::vector<std::shared_ptr<UINavigationItem>> copies;for(const auto& item:items)copies.push_back(previewItem(item,originals));measure->setItems(std::move(copies));measure->layoutIfNeeded();
    for(auto* side:{&measure->_state->left,&measure->_state->right})for(const auto& button:side->buttons)if(const auto found=originals.find(button->item.get());found!=originals.end())button->item=found->second;
    const auto& destination=*measure->_state;
    for(const auto& pair:{std::pair{&s.left,&destination.left},std::pair{&s.right,&destination.right}}) {
        const auto matches=[&](const auto& a,const auto& b) {
            if(sameInteractiveButton(a,b))return true;
            if(a->back||b->back||a->spacer||b->spacer||a->item->identifier()||b->item->identifier())return false;
            const auto ordinal=[](const auto* side,const auto& button){size_t index=0;for(const auto& entry:side->buttons){if(entry==button)return index;if(!entry->back&&!entry->spacer)++index;}return index;};
            return ordinal(pair.first,a)==ordinal(pair.second,b);
        };
        std::unordered_set<const UIView*> claimed;
        std::vector<const NavigationSide::Group*> ordered;for(const auto& group:pair.first->groups)ordered.push_back(&group);
        if(pair.first==&s.right)std::reverse(ordered.begin(),ordered.end());
        std::stable_sort(ordered.begin(),ordered.end(),[](const auto* a,const auto* b){const auto priority=[](const auto* group){return group->tint.has_value()||!group->buttons.front()->item->sharesBackground();};return priority(a)>priority(b);});
        for(const auto* current:ordered) {
            const auto& group=*current;
            const auto before=group.view->frame();NXRect extent;bool found=false;
            for(const auto& target:pair.second->groups) {
                if(claimed.contains(target.view.get()))continue;
                bool shared=false;for(const auto& a:group.buttons)for(const auto& b:target.buttons)shared|=matches(a,b);
                // New supplemental leading material starts inside Back and
                // emerges only after commitment. During tracking Back widens.
                if(pair.first==&s.left&&group.buttons.front()->back) {
                    bool existing=false;for(const auto& old:pair.first->groups)for(const auto& a:old.buttons)for(const auto& b:target.buttons)existing|=matches(a,b);
                    shared|=!existing;
                }
                if(!shared)continue;claimed.insert(target.view.get());
                const auto frame=target.view->frame();if(!found){extent=frame;found=true;}else {const auto left=std::min(extent.minX(),frame.minX()),right=std::max(extent.maxX(),frame.maxX());extent={left,0,right-left,44};}
            }
            if(!found)continue;
            const NXRect preview{(before.minX()+extent.minX())*.5f,0,(before.width()+extent.width())*.5f,44};
            interactiveProperty(group.view,"position",NXPoint{before.midX(),before.midY()},NXPoint{preview.midX(),preview.midY()},span);
            interactiveProperty(group.view,"bounds",NXRect{{},before.size},NXRect{{},preview.size},span);
            for(const auto& button:group.buttons) {
                const auto old=button->control->frame();NXFloat targetX=before.minX()+old.midX();
                for(const auto& target:pair.second->groups)for(const auto& entry:target.buttons)if(matches(button,entry))targetX=target.view->frame().minX()+entry->control->frame().midX();
                const auto local=old.midX()+(targetX-before.minX()-old.midX())*.5f-(preview.minX()-before.minX());
                interactiveProperty(button->control,"position",NXPoint{old.midX(),old.midY()},NXPoint{local,old.midY()},span);
            }
        }
    }
    s.interactiveSource=s.items;s.items=std::move(items);++s.revision;s.transitioning=true;
    auto incoming=new_shared<Internal::NavigationInkView>();incoming->setUserInteractionEnabled(false);incoming->setFrame(bar.bounds());incoming->setClipsToBounds(true);
    auto title=new_shared<UILabel>();title->setText(bar.topItem()?bar.topItem()->title():"");title->setFontSize(17);title->setFontWeight(600);title->setTextAlignment(NSTextAlignment::center);title->setFrame(destination.title->frame());incoming->addSubview(title);
    const auto targetCustom=bar.topItem()?bar.topItem()->titleView():nullptr;
    if(targetCustom){title->setHidden(true);if(targetCustom!=s.customTitle){incoming->addSubview(targetCustom);targetCustom->setFrame(destination.customTitle->frame());}}
    bar.insertSubviewBelow(incoming,s.titleInk);s.interactiveTitle=incoming;s.interactiveTitleLabel=title;
    const auto width=bar.bounds().width(),height=bar.bounds().height();
    UIView::performWithoutAnimation([&]{incoming->layer()->setAnchorPoint({0,0});incoming->layer()->setPosition({0,0});});
    interactiveProperty(incoming,"transform",NXTransform3DMakeAffineTransform(NXAffineTransform::translationBy(-width*.3f,0)),NXTransform3D::identity,span);
    interactiveProperty(incoming,"bounds",NXRect{0,0,width*.3f,height},NXRect{0,0,width,height},span);
    interactiveProperty(s.titleInk,"transform",NXTransform3D::identity,NXTransform3DMakeAffineTransform(NXAffineTransform::translationBy(width,0)),span);
    // Move a distinct custom title with its carrier too. Its application-owned
    // transform remains untouched, and it cannot acquire a second slide clock.
    if(s.customTitle&&s.customTitle!=targetCustom)s.titleInk->addSubview(s.customTitle);
}
void Internal::NavigationBarAccess::endInteractivePop(UINavigationBar& bar,bool commit,double progress) {
    auto& s=*bar._state;if(!s.interactiveSource)return;
    const auto source=std::move(*s.interactiveSource);s.interactiveSource.reset();
    if(commit){s.interactiveHandoff=progress;bar.refresh(true,-1);}
    else {if(s.interactiveTitle)s.interactiveTitle->removeFromSuperview();s.items=source;++s.revision;bar.refresh(false);finishInteractivePop(bar);}
    s.interactiveTitle.reset();s.interactiveTitleLabel.reset();
}
void Internal::NavigationBarAccess::finishInteractivePop(UINavigationBar& bar) {
    auto& s=*bar._state;
    // The title slide ends with the interactive content clock, before the
    // independent glass retirement tail. Its model transform is the source
    // pose, so keeping it attached after discarding that slide flashes it back.
    if(const auto outgoing=s.interactiveRetiringTitle.lock())outgoing->removeFromSuperview();
    s.interactiveRetiringTitle.reset();
    // At rest, restore the ordinary titleView hierarchy without changing the
    // application's transform or frame. The visible carrier is at identity.
    if(s.customTitle&&s.customTitle->superview().lock()==s.titleInk)bar.insertSubviewBelow(s.customTitle,s.glass);
}
