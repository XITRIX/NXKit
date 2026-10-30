#include "GlassScreenView.hpp"
#include "GlassScreenTypography.hpp"
#include "DemoSystemSymbol.hpp"
#include <algorithm>
#include <cmath>
#include <array>
#include <tuple>
using namespace NXKit;
namespace {
using namespace GlassScreenTypography;
constexpr const char* titles[]={"Home","Favorites","Library","Settings"};
auto label(const std::string& text, Style style, UIColor color=UIColor::label) {
    auto value=new_shared<UILabel>(); value->setText(text); GlassScreenTypography::apply(value,style);
    value->setTextColor(color); value->setUserInteractionEnabled(false); return value;
}
class Row final : public UIControl {
    void willChangeHighlight(bool active) override { setBackgroundColor(active?std::optional<UIColor>(UIColor::systemFill):std::nullopt); }
};
}
GlassScreenView::GlassScreenView() {
    _scroll=new_shared<UIScrollView>();
    _scroll->setBounceVertically(true);
    _scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
    addSubview(_scroll);
    _content=new_shared<UIView>(); _scroll->addSubview(_content);
    _navigation=new_shared<UIView>(); addSubview(_navigation);
    _edgeBlur=new_shared<UIBlurView>(); _edgeBlur->setUserInteractionEnabled(false); _edgeBlur->setBlurRadius(16); _navigation->addSubview(_edgeBlur);
    _title=label("Home",headline); _title->setTextAlignment(NSTextAlignment::center); _navigation->addSubview(_title);
    UIGlassContainerEffect group; group.setSpacing(12);
    _navigationGlass=new_shared<GlassContainerView>(group); _navigation->addSubview(_navigationGlass);
    auto weak=weak_from_base<GlassScreenView>();
    for(int i=0;i<3;i++) {
        UIGlassEffect effect; effect.setInteractive(true);
        auto glass=new_shared<BackdropEffectView>(effect); glass->layer()->setCornerRadius(22); _navigationGlass->contentView()->addSubview(glass);
        auto button=new_shared<UIControl>(); glass->addSubview(button);
        std::shared_ptr<UIView> fallback;
        if(i==0) {
            auto icon=new_shared<UIView>(); icon->setUserInteractionEnabled(false); fallback=icon;
            for(int part=0;part<2;++part) {
                auto stroke=new_shared<UIView>(NXRect(16,part?25:16,13,2.4f)); stroke->setBackgroundColor(accentColor());
                stroke->layer()->setCornerRadius(1.2f); stroke->setTransform(NXAffineTransform::rotationBy(part?45:-45)); icon->addSubview(stroke);
            }
        } else {
            auto text=label(i==1?"⋯":"+",Style{NXFloat(i==1?26:32),400},i==2?UIColor::white:accentColor());
            text->setTextAlignment(NSTextAlignment::center); fallback=text;
        }
        button->addSubview(new_shared<DemoSystemSymbol>(i==0?"chevron.left":(i==1?"ellipsis":"plus"),
            i==1?17:20,i==2?400:600,i==2?UIColor::white:accentColor(),fallback));
        button->primaryAction=UIAction(i==0?"Back":(i==1?"More":"Add"),[weak,i] {
            if(auto self=weak.lock()) { if(i==0) { if(self->onBack) self->onBack(); } else if(i==1) self->toggleDetails(); else self->addItem(); }
        });
        _actions[i==0?"back":(i==1?"more":"add")]=button;
        if(i==2) _plusGlass=glass;
    }
    _tab=new_shared<LensTabBarView>(false,4); addSubview(_tab);
    _tab->onSelectionChanged=[weak](size_t) { if(auto self=weak.lock()) { self->setScrollPosition(0); self->rebuild(); } };
    rebuild();
}
UIColor GlassScreenView::accentColor() const { return _accent==0?UIColor::systemBlue:(_accent==1?UIColor(255,45,85):UIColor(0,163,140)); }
std::shared_ptr<UIControl> GlassScreenView::action(const std::string& id) const { auto it=_actions.find(id); return it==_actions.end()?nullptr:it->second; }
void GlassScreenView::selectTab(size_t index) { _tab->setSelectedIndex(index); }
void GlassScreenView::addItem() { if(_added<30) ++_added; rebuild(); }
void GlassScreenView::cycleAccent() { _accent=(_accent+1)%3; rebuild(); }
void GlassScreenView::cycleBackdrop() { _backdrop=(_backdrop+1)%3; rebuild(); }
void GlassScreenView::toggleDetails() { _details=!_details; rebuild(); }
void GlassScreenView::toggleOrder() { _reversed=!_reversed; rebuild(); }
void GlassScreenView::reset() { _added=_accent=_backdrop=0; _details=_reversed=false; _completed.clear(); rebuild(); }
void GlassScreenView::setScrollPosition(NXFloat value) {
    if(!std::isfinite(value)||_topInset<0) return;
    const auto maximum=std::max(0.0f,_scroll->contentSize().height-_scroll->bounds().height()+_scroll->contentInset().bottom+_topInset);
    _scroll->setContentOffset({0,-_topInset+std::clamp(value,0.0f,maximum)},false);
}
void GlassScreenView::rebuild() {
    const auto children=_content->subviews(); for(const auto& child:children) child->removeFromSuperview();
    std::erase_if(_actions,[](const auto& entry) { return entry.first!="back"&&entry.first!="more"&&entry.first!="add"; });
    setBackgroundColor(_backdrop==0?UIColorThemed(UIColor(242,242,247),UIColor::black,"glassScreenGrouped"):(_backdrop==1?UIColor(31,94,184):UIColor(209,71,61)));
    _title->setText(titles[selectedTab()]);
    auto effect=*_plusGlass->glassEffect(); effect.setTintColor(accentColor()); _plusGlass->setEffect(effect);
    for(const auto& child:_navigationGlass->contentView()->subviews()) {
        if(auto glass=std::dynamic_pointer_cast<BackdropEffectView>(child);glass&&glass!=_plusGlass) {
            auto material=*glass->glassEffect();material.setContentVibrancyEnabled(_accent==0);glass->setEffect(material);
        }
    }
    _tab->setAccentColor(accentColor());
    const auto recolor=[&](const auto& self,const std::shared_ptr<UIView>& view)->void {
        view->setTintColor(accentColor());
        if(auto text=std::dynamic_pointer_cast<UILabel>(view)) text->setTextColor(accentColor());
        if(view->backgroundColor()) view->setBackgroundColor(accentColor());
        for(const auto& child:view->subviews()) self(self,child);
    };
    recolor(recolor,action("back")->subviews().front()); recolor(recolor,action("more")->subviews().front());
    const auto weak=weak_from_base<GlassScreenView>();
    const auto width=std::max(0.0f,bounds().width()-safeAreaInsets().left-safeAreaInsets().right);
    const auto column=std::min(680.0f,std::max(0.0f,width-32));
    const auto x=(bounds().width()-column)*.5f;
    NXFloat y=0;
    auto section=[&](const std::string& title,const std::vector<std::tuple<std::string,std::string,std::string,std::function<void()>>>& rows,const std::string& footer="") {
        if(!title.empty()) { auto heading=label(title,headline,UIColor::secondaryLabel); heading->setFrame({x+16,y+8,column-32,24}); _content->addSubview(heading); y+=40.333333f; }
        auto card=new_shared<UIView>(); card->setBackgroundColor(UIColor::secondarySystemGroupedBackground); card->layer()->setCornerRadius(24); card->setClipsToBounds(true); _content->addSubview(card);
        NXFloat rowY=0;
        for(const auto& [id,title,value,callback]:rows) {
            const bool detail=_details&&id.starts_with("item-"); const NXFloat height=detail?70:52;
            auto control=new_shared<Row>(); control->setFrame({0,rowY,column,height}); card->addSubview(control); _actions[id]=control;
            control->primaryAction=UIAction(title,callback);
            auto text=label(title,body); text->setFrame({16,0,std::max(0.0f,column-140),52}); control->addSubview(text);
            if(id=="add-row"||id=="reset") text->setTextColor(accentColor());
            auto detailValue=label(value,body,UIColor::secondaryLabel); detailValue->setTextAlignment(NSTextAlignment::right); detailValue->setFrame({std::max(16.0f,column-136),0,std::max(0.0f,std::min(120.0f,column-32)),52}); control->addSubview(detailValue);
            if(detail) { auto subtitle=label("Tap to change this item's status.",footnote,UIColor::secondaryLabel); subtitle->setFrame({16,42,column-32,22}); control->addSubview(subtitle); }
            rowY+=height;
            if(rowY>0 && id!=std::get<0>(rows.back())) { auto line=new_shared<UIView>(NXRect(16,rowY-.5f,std::max(0.0f,column-16),.5f)); line->setBackgroundColor(UIColor::separator); card->addSubview(line); }
        }
        card->setFrame({x,y,column,rowY}); y+=rowY;
        if(!footer.empty()) { auto note=label(footer,footnote,UIColor::secondaryLabel); note->setFrame({x+16,y+4,column-32,24}); _content->addSubview(note); y+=29.666667f; }
        else y+=17.666667f;
    };
    section("Appearance",{{"background","Background",std::array{"System","Blue","Coral"}[_backdrop],[weak]{if(auto self=weak.lock())self->cycleBackdrop();}},
        {"accent","Accent color",std::array{"Blue","Pink","Mint"}[_accent],[weak]{if(auto self=weak.lock())self->cycleAccent();}}});
    section("Collection",{{"add-row","Add item","",[weak]{if(auto self=weak.lock())self->addItem();}},
        {"sort","Sort order",_reversed?"Descending":"Ascending",[weak]{if(auto self=weak.lock())self->toggleOrder();}},
        {"reset","Reset demo","",[weak]{if(auto self=weak.lock())self->reset();}}},"Use + to add an item. More shows additional details.");
    std::vector<std::tuple<std::string,std::string,std::string,std::function<void()>>> items;
    for(int i=1;i<=8+_added;++i) { const auto item=_reversed?9+_added-i:i; items.push_back({"item-"+std::to_string(item),"Item "+std::to_string(item),_completed.contains(item)?"Done":"Open",[weak,item] { if(auto self=weak.lock()) { if(self->_completed.contains(item))self->_completed.erase(item);else self->_completed.insert(item);self->rebuild(); } }}); }
    section(std::string(titles[selectedTab()])+" collection",items);
    auto summary=label(std::to_string(_completed.size())+" completed · "+std::to_string(8+_added)+" items",body,UIColor::secondaryLabel); summary->setFrame({x+16,y,column-32,44}); _content->addSubview(summary); y+=74;
    _content->setFrame({0,0,bounds().width(),y}); _scroll->setContentSize({bounds().width(),y}); arrange();
}
void GlassScreenView::arrange() {
    const auto size=bounds().size; const auto safe=safeAreaInsets();
    _scroll->setFrame(bounds());
    const NXFloat top=safe.top+54, barHeight=62;
    const NXFloat barWidth=std::max(0.0f,std::min(520.0f,size.width-safe.left-safe.right-42));
    const NXFloat tabY=std::max(top,size.height-barHeight-std::max(8.0f,safe.bottom-13));
    _tab->setFrame({(size.width-barWidth)*.5f,tabY,barWidth,barHeight});
    _navigation->setFrame({0,0,size.width,top}); _edgeBlur->setFrame({0,0,size.width,top});
    _title->setFrame({safe.left+72,safe.top,std::max(0.0f,size.width-safe.left-safe.right-144),44});
    _navigationGlass->setFrame({0,safe.top,size.width,44});
    for(int i=0;i<3;++i) {
        const auto button=action(i==0?"back":(i==1?"more":"add")); const auto glass=button->superview().lock();
        glass->setFrame({i==0?safe.left+16:size.width-safe.right-(i==1?112.0f:60.0f),0,44,44});
        button->setFrame({0,0,44,44}); button->subviews().front()->setFrame({0,0,44,44});
    }
    const NXFloat oldPosition=_topInset<0?0:_scroll->contentOffset().y+_topInset;
    _scroll->setContentInset({top,0,std::max(0.0f,size.height-tabY+12),0});
    _scroll->setVerticalScrollIndicatorInsets({top,0,std::max(0.0f,size.height-tabY),safe.right});
    if(_topInset!=top) { _topInset=top; setScrollPosition(oldPosition); }
    _tab->layoutIfNeeded();
}
void GlassScreenView::layoutSubviews() { UIView::layoutSubviews(); if(_content->frame().width()!=bounds().width()) rebuild(); else arrange(); }
void GlassScreenView::safeAreaInsetsDidChange() { UIView::safeAreaInsetsDidChange(); if(_scroll) rebuild(); }
