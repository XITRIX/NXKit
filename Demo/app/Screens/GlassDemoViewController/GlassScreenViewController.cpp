#include "GlassScreenViewController.hpp"
#include "GlassScreenTypography.hpp"
#include <NXNavigationController.h>
#include <algorithm>
#include <array>
using namespace NXKit;
namespace {
constexpr const char* titles[]={"Home","Favorites","Library","Settings"};
constexpr const char* symbols[]={"house.fill","star.fill","books.vertical.fill","slider.horizontal.3"};
constexpr std::array arrangements={"Shared icons","Accent action","Fixed-space split","Three shared items","Text buttons","Leading items","Unshared item","Hidden glass"};
constexpr std::array explanations={
    "Two image actions share one glass capsule. Push the next screen to separate the accent action.",
    "A prominent Add action has its own tinted glass. The regular More action keeps a separate capsule.",
    "A zero-width fixed-space item splits two regular image actions into separate glass capsules.",
    "Add, More and Delete share one wider capsule. Watch it grow on push and shrink on Back.",
    "Edit and Save are text actions, each with its own glass. Their identifiers match the image actions on other screens.",
    "A leading favorite action supplements the automatic Back button. The trailing image pair still shares its glass.",
    "Add opts out of sharing its background. More remains a regular glass action, without a separator item.",
    "Add hides its glass background. More keeps its capsule; Back restores the previous arrangement."};
class LargeTitleDemoView final:public UIScrollView {
public:
    explicit LargeTitleDemoView(std::function<void(UINavigationItemLargeTitleDisplayMode)> push) {
        setBounceVertically(true);setBackgroundColor(UIColor::systemGroupedBackground);setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes);
        const std::array modes={UINavigationItemLargeTitleDisplayMode::automatic,UINavigationItemLargeTitleDisplayMode::always,UINavigationItemLargeTitleDisplayMode::never};
        const std::array names={"Push Automatic (inherits this screen)","Push Always","Push Never (inline title)"};
        for(size_t i=0;i<3;++i) {
            auto row=new_shared<UIButton>();row->setText(names[i]);row->primaryAction=UIAction(names[i],[push,mode=modes[i]]{push(mode);});addSubview(row);
        }
        for(size_t i=0;i<24;++i){auto label=new_shared<UILabel>();label->setText(i==0?"Scroll slowly and release halfway to test settling.":"Content row "+std::to_string(i));label->setTextColor(UIColor::secondaryLabel);addSubview(label);}
    }
    void layoutSubviews()override {
        NXFloat y=8;for(const auto& row:subviews()){row->setFrame({16,y,std::max(0.f,bounds().width()-32),48});y+=52;}
        setContentSize({bounds().width(),y});UIScrollView::layoutSubviews();
    }
};
class LargeTitleDemoController final:public UIViewController {
public:
    explicit LargeTitleDemoController(UINavigationItemLargeTitleDisplayMode mode=UINavigationItemLargeTitleDisplayMode::always) {
        setTitle(mode==UINavigationItemLargeTitleDisplayMode::never?"Inline title":mode==UINavigationItemLargeTitleDisplayMode::automatic?"Automatic title":"Large titles");navigationItem()->setLargeTitleDisplayMode(mode);
    }
    void loadView()override {
        setView(new_shared<LargeTitleDemoView>([weak=weak_from_base<LargeTitleDemoController>()](auto mode){if(const auto self=weak.lock())if(const auto nav=self->navigationController())nav->pushViewController(new_shared<LargeTitleDemoController>(mode),true);}));
    }
};
class NavigationDemoRow final : public UIControl {
    void willChangeHighlight(bool active) override {setBackgroundColor(active?std::optional<UIColor>(UIColor::systemFill):std::nullopt);}
};
class NavigationMorphView final : public UIView {
public:
    NavigationMorphView(size_t arrangement,std::function<void(size_t)> choose,std::function<void()> collection)
        :_arrangement(arrangement),_choose(std::move(choose)),_collection(std::move(collection)) {
        setBackgroundColor(UIColor::systemGroupedBackground);
        _scroll=new_shared<UIScrollView>();_scroll->setBounceVertically(true);_scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);addSubview(_scroll);
        _content=new_shared<UIView>();_scroll->addSubview(_content);
        _status=new_shared<UILabel>();_status->setText("Tap a navigation item to test its glass.");GlassScreenTypography::apply(_status,GlassScreenTypography::footnote);_status->setTextColor(UIColor::secondaryLabel);
        rebuild();
    }
    std::shared_ptr<UIControl> action(const std::string& id) const {const auto it=_actions.find(id);return it==_actions.end()?nullptr:it->second;}
    void setStatus(const std::string& status){_status->setText(status);}
    void layoutSubviews() override {UIView::layoutSubviews();if(_content->frame().width()!=bounds().width())rebuild();else arrange();}
    void safeAreaInsetsDidChange() override {UIView::safeAreaInsetsDidChange();if(_scroll)arrange();}
private:
    void rebuild() {
        const auto children=_content->subviews();for(const auto& child:children)child->removeFromSuperview();_actions.clear();
        const auto column=std::min(680.f,std::max(0.f,bounds().width()-safeAreaInsets().left-safeAreaInsets().right-32));
        const auto x=(bounds().width()-column)*.5f;NXFloat y=8;
        auto text=[&](const std::string& value,GlassScreenTypography::Style style,UIColor color=UIColor::label) {
            auto label=new_shared<UILabel>();label->setText(value);GlassScreenTypography::apply(label,style);label->setTextColor(color);label->setUserInteractionEnabled(false);
            const auto height=std::max(24.f,label->sizeThatFits({std::max(1.f,column-32),1000}).height);
            label->setFrame({x+16,y,std::max(0.f,column-32),height});_content->addSubview(label);y+=height+12;
        };
        text(std::to_string(_arrangement+1)+" of "+std::to_string(arrangements.size())+" · "+arrangements[_arrangement],GlassScreenTypography::headline);
        text(explanations[_arrangement],GlassScreenTypography::body);
        _status->setFrame({x+16,y,std::max(0.f,column-32),24});_content->addSubview(_status);y+=40;
        std::shared_ptr<UIView> card;NXFloat cardY=0;
        auto beginCard=[&]{card=new_shared<UIView>();card->setBackgroundColor(UIColor::secondarySystemGroupedBackground);card->layer()->setCornerRadius(20);card->setClipsToBounds(true);_content->addSubview(card);cardY=0;};
        auto row=[&](const std::string& id,const std::string& title,std::function<void()> action) {
            if(cardY>0){auto separator=new_shared<UIView>(NXRect{16,cardY-.5f,std::max(0.f,column-16),.5f});separator->setBackgroundColor(UIColor::separator);card->addSubview(separator);}
            auto control=new_shared<NavigationDemoRow>();control->setFrame({0,cardY,column,52});control->primaryAction=UIAction(title,std::move(action));card->addSubview(control);_actions[id]=control;
            auto label=new_shared<UILabel>();label->setText(title);GlassScreenTypography::apply(label,GlassScreenTypography::body);label->setTextColor(UIColor::tint);label->setUserInteractionEnabled(false);label->setFrame({16,0,std::max(0.f,column-54),52});control->addSubview(label);
            auto chevron=new_shared<UILabel>();chevron->setText("›");chevron->setTextColor(UIColor::secondaryLabel);chevron->setUserInteractionEnabled(false);chevron->setFrame({std::max(0.f,column-32),0,16,52});control->addSubview(chevron);cardY+=52;
        };
        auto endCard=[&]{card->setFrame({x,y,column,cardY});y+=cardY+24;};
        const auto choose=_choose;const auto next=(_arrangement+1)%arrangements.size();
        beginCard();row("next",std::string("Push next: ")+arrangements[next],[choose,next]{choose(next);});row("collection","Pop to collection",_collection);endCard();
        text("Push any arrangement",GlassScreenTypography::headline,UIColor::secondaryLabel);
        beginCard();for(size_t i=0;i<arrangements.size();++i)row("arrangement-"+std::to_string(i),arrangements[i],[choose,i]{choose(i);});endCard();
        text("Use Back to reverse each morph. Tabs retain their own navigation stacks.",GlassScreenTypography::footnote,UIColor::secondaryLabel);
        _content->setFrame({0,0,bounds().width(),y});_scroll->setContentSize({bounds().width(),y});arrange();
    }
    void arrange() {
        const auto safe=safeAreaInsets();const auto top=safe.top+8;
        const auto offset=_topInset<0?0:_scroll->contentOffset().y+_topInset;
        _scroll->setFrame(bounds());_scroll->setContentInset({top,0,safe.bottom+16,0});_scroll->setVerticalScrollIndicatorInsets({safe.top,0,safe.bottom,safe.right});
        if(_topInset!=top){_topInset=top;_scroll->setContentOffset({0,offset-top},false);}
    }
    size_t _arrangement;
    NXFloat _topInset=-1;
    std::function<void(size_t)> _choose;
    std::function<void()> _collection;
    std::shared_ptr<UIScrollView> _scroll;
    std::shared_ptr<UIView> _content;
    std::shared_ptr<UILabel> _status;
    std::map<std::string,std::shared_ptr<UIControl>> _actions;
};
}
GlassNavigationDemoViewController::GlassNavigationDemoViewController(size_t arrangement):_arrangement(arrangement) {
    navigationItem()->setLargeTitleDisplayMode(UINavigationItemLargeTitleDisplayMode::never);
    if(arrangement>=arrangementCount)throw std::out_of_range("Navigation demo arrangement");
    constexpr std::array barTitles={"Shared","Accent","Separated","Three items","Text","Leading","Unshared","Hidden glass"};setTitle(barTitles[arrangement]);
    const auto weak=weak_from_base<GlassNavigationDemoViewController>();
    auto image=[&](const char* symbol,const char* identifier,const char* title) {
        auto item=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed(symbol,20,400,3),UIBarButtonItemStyle::plain,UIAction(title,[weak,title]{if(const auto self=weak.lock())self->recordAction(title);}));
        item->setTitle(title);item->setIdentifier(identifier);return item;
    };
    auto add=image("plus","demo.add","Add"),more=image("ellipsis","demo.more","More");
    std::vector<std::shared_ptr<UIBarButtonItem>> right{add,more};
    if(arrangement==1)add->setStyle(UIBarButtonItemStyle::prominent);
    if(arrangement==2)right={add,UIBarButtonItem::fixedSpaceItemOfWidth(0),more};
    if(arrangement==3)right.push_back(image("trash","demo.delete","Delete"));
    if(arrangement==4) {
        add=std::make_shared<UIBarButtonItem>("Edit",UIBarButtonItemStyle::plain,UIAction("Edit",[weak]{if(const auto self=weak.lock())self->recordAction("Edit");}));add->setIdentifier("demo.add");
        more=std::make_shared<UIBarButtonItem>("Save",UIBarButtonItemStyle::plain,UIAction("Save",[weak]{if(const auto self=weak.lock())self->recordAction("Save");}));more->setIdentifier("demo.more");right={add,more};
    }
    if(arrangement==5){navigationItem()->setLeftItemsSupplementBackButton(true);navigationItem()->setLeftBarButtonItem(image("star","demo.favorite","Favorite"));}
    if(arrangement==6)add->setSharesBackground(false);
    if(arrangement==7)add->setHidesSharedBackground(true);
    navigationItem()->setRightBarButtonItems(std::move(right));
}
void GlassNavigationDemoViewController::loadView() {
    const auto weak=weak_from_base<GlassNavigationDemoViewController>();
    setView(new_shared<NavigationMorphView>(_arrangement,[weak](size_t next){if(const auto self=weak.lock())self->push(next);},[weak]{if(const auto self=weak.lock())if(const auto nav=self->navigationController())nav->popToRootViewController(true);}));
}
std::shared_ptr<UIControl> GlassNavigationDemoViewController::action(const std::string& id){loadViewIfNeeded();return std::static_pointer_cast<NavigationMorphView>(view())->action(id);}
void GlassNavigationDemoViewController::push(size_t arrangement){if(const auto nav=navigationController())nav->pushViewController(new_shared<GlassNavigationDemoViewController>(arrangement),true);}
void GlassNavigationDemoViewController::recordAction(const std::string& name){loadViewIfNeeded();std::static_pointer_cast<NavigationMorphView>(view())->setStatus(name+" tapped · "+std::to_string(++_presses));}
GlassListViewController::GlassListViewController(size_t tab):_tab(tab) {
    navigationItem()->setLargeTitleDisplayMode(UINavigationItemLargeTitleDisplayMode::never);
    if(tab>=4)throw std::out_of_range("Demo tab index");setTitle(titles[tab]);
    const auto weak=weak_from_base<GlassListViewController>();
    auto close=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("chevron.left",20,600,3),UIBarButtonItemStyle::plain,UIAction("Close",[weak]{if(const auto self=weak.lock();self&&self->onClose)self->onClose();}));
    close->setTitle("Back");navigationItem()->setLeftBarButtonItem(close);
    auto add=std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::add,UIAction("Add",[weak]{if(const auto self=weak.lock())self->list()->addItem();}));
    add->setStyle(UIBarButtonItemStyle::prominent);add->setIdentifier("demo.add");
    auto more=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("ellipsis",17,600,3),UIBarButtonItemStyle::plain,UIAction("More",[weak]{if(const auto self=weak.lock())self->list()->toggleDetails();}));
    more->setTitle("More");more->setIdentifier("demo.more");navigationItem()->setRightBarButtonItems({add,more});
}
void GlassListViewController::loadView() {
    auto screen=new_shared<GlassScreenView>();screen->selectTab(_tab);
    screen->onAccentChanged=[weak=weak_from_base<GlassListViewController>()](UIColor){if(const auto self=weak.lock())self->updateAccent();};
    screen->onLargeTitleDemo=[weak=weak_from_base<GlassListViewController>()]{if(const auto self=weak.lock())if(const auto nav=self->navigationController())nav->pushViewController(new_shared<LargeTitleDemoController>(),true);};
    screen->onNavigationDemo=[weak=weak_from_base<GlassListViewController>()]{if(const auto self=weak.lock())if(const auto nav=self->navigationController())nav->pushViewController(new_shared<GlassNavigationDemoViewController>(),true);};
    setView(screen);
}
std::shared_ptr<GlassScreenView> GlassListViewController::list(){loadViewIfNeeded();return std::static_pointer_cast<GlassScreenView>(view());}
void GlassListViewController::updateAccent() {
    if(!viewIsLoaded())return;const auto color=list()->accentColor();
    if(const auto nav=std::dynamic_pointer_cast<UINavigationController>(navigationController()))nav->navigationBar()->setTintColor(color);
    if(const auto tabs=std::dynamic_pointer_cast<UITabBarController>(tabBarController()))tabs->tabBar()->setTintColor(color);
}
void GlassListViewController::viewWillAppear(bool animated){UIViewController::viewWillAppear(animated);updateAccent();}
GlassScreenViewController::GlassScreenViewController() {
    setTitle("iOS screen layout");std::vector<std::shared_ptr<UIViewController>> tabs;
    for(size_t i=0;i<4;++i){auto root=new_shared<GlassListViewController>(i);
        root->onClose=[weak=weak_from_base<GlassScreenViewController>()]{if(const auto self=weak.lock())self->close();};
        auto nav=new_shared<UINavigationController>(root);nav->navigationBar()->setPrefersLargeTitles(true);nav->setTitle(titles[i]);
        auto image=UIImage::systemImageNamed(symbols[i],28,400,3);if(!image)image=UIImage::fromRes("img/moonlight_96.png",3);
        nav->setTabBarItem(std::make_shared<UITabBarItem>(titles[i],image,std::intptr_t(i)));tabs.push_back(nav);}
    setViewControllers(std::move(tabs));
}
void GlassScreenViewController::willMoveToParent(const std::shared_ptr<UIViewController>& parent) {
    UIViewController::willMoveToParent(parent);
    if(const auto nav=std::dynamic_pointer_cast<NXNavigationController>(parent)){auto item=nav->navigationItemFor(shared_from_this());item->setHidesNavigationBar(true);item->setHidesToolbar(true);}
}
std::shared_ptr<UINavigationController> GlassScreenViewController::selectedNavigationController(){return std::dynamic_pointer_cast<UINavigationController>(selectedViewController());}
std::shared_ptr<GlassScreenView> GlassScreenViewController::selectedList(){const auto nav=selectedNavigationController();const auto content=nav?std::dynamic_pointer_cast<GlassListViewController>(nav->topViewController()):nullptr;return content?content->list():nullptr;}
void GlassScreenViewController::close() {
    for(auto current=parent().lock();current;current=current->parent().lock())if(const auto nav=std::dynamic_pointer_cast<UINavigationControllerInterface>(current)){if(nav->topViewController().get()==this){nav->popViewController(true);return;}}
    if(presentingViewController())dismiss(true);
}
