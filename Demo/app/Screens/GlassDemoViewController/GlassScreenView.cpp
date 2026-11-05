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
    _scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes);
    addSubview(_scroll);
    _content=new_shared<UIView>(); _scroll->addSubview(_content);
    rebuild();
}
UIColor GlassScreenView::accentColor() const { return _accent==0?UIColor::systemBlue:(_accent==1?UIColor(255,45,85):UIColor(0,163,140)); }
std::shared_ptr<UIControl> GlassScreenView::action(const std::string& id) const { auto it=_actions.find(id); return it==_actions.end()?nullptr:it->second; }
void GlassScreenView::selectTab(size_t index) { if(index>=4)throw std::out_of_range("Demo tab index");_selectedTab=index;rebuild(); }
void GlassScreenView::addItem() { if(_added<30) ++_added; rebuild(); }
void GlassScreenView::cycleAccent() { _accent=(_accent+1)%3; rebuild(); }
void GlassScreenView::cycleBackdrop() { _backdrop=(_backdrop+1)%3; rebuild(); }
void GlassScreenView::cycleScrollEdgeStyle() {
    _scrollEdgeStyle=(_scrollEdgeStyle+1)%4;
    const auto style=std::array{UIScrollEdgeEffectStyle::automatic,UIScrollEdgeEffectStyle::soft,UIScrollEdgeEffectStyle::hard,UIScrollEdgeEffectStyle::none}[_scrollEdgeStyle];
    _scroll->topEdgeEffect()->setStyle(style);_scroll->bottomEdgeEffect()->setStyle(style);
    rebuild();
}
void GlassScreenView::toggleDetails() { _details=!_details; rebuild(); }
void GlassScreenView::toggleOrder() { _reversed=!_reversed; rebuild(); }
void GlassScreenView::reset() { _added=_accent=_backdrop=_scrollEdgeStyle=0; _scroll->topEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::automatic);_scroll->bottomEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::automatic);_details=_reversed=false; _completed.clear(); rebuild(); }
void GlassScreenView::setScrollPosition(NXFloat value) {
    if(!std::isfinite(value)||_topInset<0) return;
    const auto maximum=std::max(0.0f,_scroll->contentSize().height-_scroll->bounds().height()+_scroll->adjustedContentInset().bottom+_topInset);
    _scroll->setContentOffset({0,-_topInset+std::clamp(value,0.0f,maximum)},false);
}
void GlassScreenView::rebuild() {
    const auto children=_content->subviews(); for(const auto& child:children) child->removeFromSuperview();
    _actions.clear();
    setBackgroundColor(_backdrop==0?UIColorThemed(UIColor(242,242,247),UIColor::black,"glassScreenGrouped"):(_backdrop==1?UIColor(31,94,184):UIColor(209,71,61)));
    if(onAccentChanged)onAccentChanged(accentColor());
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
        {"accent","Accent color",std::array{"Blue","Pink","Mint"}[_accent],[weak]{if(auto self=weak.lock())self->cycleAccent();}},
        {"scroll-edge-style","Scroll edge",std::array{"Automatic","Soft","Hard","None"}[_scrollEdgeStyle],[weak]{if(auto self=weak.lock())self->cycleScrollEdgeStyle();}}});
    section("Collection",{{"add-row","Add item","",[weak]{if(auto self=weak.lock())self->addItem();}},
        {"sort","Sort order",_reversed?"Descending":"Ascending",[weak]{if(auto self=weak.lock())self->toggleOrder();}},
        {"reset","Reset demo","",[weak]{if(auto self=weak.lock())self->reset();}}},"Use + to add an item. More shows additional details.");
    section("Navigation",{{"navigation-demo","Navigation morphing","Push ›",[weak]{if(auto self=weak.lock();self&&self->onNavigationDemo)self->onNavigationDemo();}},
        {"large-title-demo","Large titles","Push ›",[weak]{if(auto self=weak.lock();self&&self->onLargeTitleDemo)self->onLargeTitleDemo();}}});
    std::vector<std::tuple<std::string,std::string,std::string,std::function<void()>>> items;
    for(int i=1;i<=8+_added;++i) { const auto item=_reversed?9+_added-i:i; items.push_back({"item-"+std::to_string(item),"Item "+std::to_string(item),_completed.contains(item)?"Done":"Open",[weak,item] { if(auto self=weak.lock()) { if(self->_completed.contains(item))self->_completed.erase(item);else self->_completed.insert(item);self->rebuild(); } }}); }
    section(std::string(titles[selectedTab()])+" collection",items);
    auto summary=label(std::to_string(_completed.size())+" completed · "+std::to_string(8+_added)+" items",body,UIColor::secondaryLabel); summary->setFrame({x+16,y,column-32,44}); _content->addSubview(summary); y+=74;
    _content->setFrame({0,0,bounds().width(),y}); _scroll->setContentSize({bounds().width(),y}); arrange();
}
void GlassScreenView::arrange() {
    const auto size=bounds().size; const auto safe=safeAreaInsets();
    _scroll->setFrame(bounds());
    const NXFloat top=safe.top;
    // Let UIScrollView coordinate changing bar safe areas. Reapplying a
    // manual top inset/offset here would cancel every collapse/deceleration tick.
    _topInset=top;
    _scroll->setVerticalScrollIndicatorInsets({top,0,safe.bottom,safe.right});
}
void GlassScreenView::layoutSubviews() { UIView::layoutSubviews(); if(_content->frame().width()!=bounds().width()) rebuild(); else arrange(); }
void GlassScreenView::safeAreaInsetsDidChange() { UIView::safeAreaInsetsDidChange(); if(_scroll) rebuild(); }
