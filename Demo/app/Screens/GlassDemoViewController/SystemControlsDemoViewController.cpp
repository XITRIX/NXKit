#include "SystemControlsDemoViewController.hpp"
#include "GlassScreenTypography.hpp"
#include <UIKit.h>
#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>

using namespace NXKit;
namespace {
using Page=SystemControlsDemoViewController::Page;
std::string number(NXFloat value){std::ostringstream out;out<<std::fixed<<std::setprecision(2)<<value;return out.str();}
auto makeLabel(const std::string& text,GlassScreenTypography::Style style,UIColor color=UIColor::label) {
    auto label=new_shared<UILabel>();label->setText(text);GlassScreenTypography::apply(label,style);
    label->setTextColor(color);label->setUserInteractionEnabled(false);return label;
}
class SettingRow final:public UIControl {
public:
    explicit SettingRow(const std::string& title) {
        _title=makeLabel(title,GlassScreenTypography::body);_value=makeLabel("",GlassScreenTypography::body,UIColor::secondaryLabel);
        _value->setTextAlignment(NSTextAlignment::right);addSubview(_title);addSubview(_value);
    }
    void setValue(const std::string& value){_value->setText(value);}
    void setAccessory(const std::shared_ptr<UIView>& accessory){_accessory=accessory;accessory->sizeToFit();addSubview(accessory);}
    void layoutSubviews()override {
        UIView::layoutSubviews();const auto b=bounds();const auto accessory=_accessory?_accessory->bounds().size:NXSize{};
        const auto right=_accessory?accessory.width:std::min(152.f,b.width()*.46f);
        _title->setFrame({16,0,std::max(0.f,b.width()-right-40),b.height()});
        _value->setHidden(bool(_accessory));_value->setFrame({std::max(16.f,b.width()-right-16),0,right,b.height()});
        if(_accessory)_accessory->setFrame({b.maxX()-accessory.width-16,b.midY()-accessory.height*.5f,accessory.width,accessory.height});
    }
    void willChangeHighlight(bool active)override{setBackgroundColor(active?std::optional<UIColor>{UIColor::systemFill}:std::nullopt);}
private:
    std::shared_ptr<UILabel> _title,_value;
    std::shared_ptr<UIView> _accessory;
};
class Preview final:public UIView {
public:
    explicit Preview(const std::shared_ptr<UIControl>& control):_control(control) {
        setBackgroundColor(UIColor::secondarySystemGroupedBackground);layer()->setCornerRadius(22);
        addSubview(control);_value=makeLabel("",GlassScreenTypography::footnote,UIColor::secondaryLabel);
        _value->setTextAlignment(NSTextAlignment::center);addSubview(_value);
    }
    void setValue(const std::string& value){_value->setText(value);}
    void layoutSubviews()override {
        UIView::layoutSubviews();const auto b=bounds();
        if(std::dynamic_pointer_cast<UISlider>(_control))_control->setFrame({20,14,std::max(0.f,b.width()-40),52});
        else {const auto size=_control->bounds().size;_control->setFrame({b.midX()-size.width*.5f,27,size.width,size.height});}
        _value->setFrame({16,78,std::max(0.f,b.width()-32),32});
    }
private:
    std::shared_ptr<UIControl> _control;
    std::shared_ptr<UILabel> _value;
};
class ControlsView final:public UIScrollView {
public:
    ControlsView(Page page,std::function<void(Page)> push):_page(page),_push(std::move(push)) {
        setBackgroundColor(UIColor::systemGroupedBackground);setBounceVertically(true);
        setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes);
        _content=new_shared<UIView>();addSubview(_content);
        if(page==Page::catalog)buildCatalog();else if(page==Page::slider)buildSlider();else buildSwitch();
        refresh();
    }
    std::shared_ptr<UIControl> action(const std::string& id)const{const auto it=_actions.find(id);return it==_actions.end()?nullptr:it->second;}
    std::shared_ptr<UISlider> slider()const{return _slider;}
    std::shared_ptr<UISwitch> switchControl()const{return _switch;}
    void layoutSubviews()override {
        const auto safe=safeAreaInsets();
        const auto available=std::max(0.f,bounds().width()-safe.left-safe.right);
        const auto width=std::min(680.f,std::max(0.f,available-32));
        const auto x=safe.left+(available-width)*.5f;NXFloat y=12;
        for(const auto& block:_blocks) {
            const auto inset=block.text?16.f:0.f;
            const auto w=std::max(0.f,width-2*inset);
            const auto height=block.text?std::max(block.height,block.view->sizeThatFits({std::max(1.f,w),1000}).height):block.height;
            block.view->setFrame({x+inset,y,w,height});
            if(block.rows)for(size_t i=0;i<block.rows->size();++i)(*block.rows)[i]->setFrame({0,NXFloat(i)*52,width,52});
            y+=height+block.gap;
        }
        _content->setFrame({0,0,bounds().width(),y+16});setContentSize({bounds().width(),y+16});
        UIScrollView::layoutSubviews();
    }
private:
    struct Block {std::shared_ptr<UIView> view;NXFloat height,gap;bool text=false;std::shared_ptr<std::vector<std::shared_ptr<SettingRow>>> rows;};
    Page _page;
    std::function<void(Page)> _push;
    std::shared_ptr<UIView> _content;
    std::vector<Block> _blocks;
    std::shared_ptr<std::vector<std::shared_ptr<SettingRow>>> _currentRows;
    std::shared_ptr<UIView> _currentCard;
    size_t _currentBlock=0;
    std::map<std::string,std::shared_ptr<UIControl>> _actions;
    std::map<std::string,std::shared_ptr<SettingRow>> _rows;
    std::map<std::string,std::shared_ptr<UISwitch>> _toggles;
    std::shared_ptr<UISlider> _slider;
    std::shared_ptr<UISwitch> _switch;
    std::shared_ptr<Preview> _preview;
    std::shared_ptr<UILabel> _events;
    bool _animated=true,_endImages=false;
    size_t _range=0,_valueStep=1,_minimumTint=0,_maximumTint=0,_thumbTint=0,_switchTint=0,_ticks=0,_background=0;
    std::array<unsigned,5> _counts{};
    std::string _lastEvent="No input yet";
    void text(const std::string& value,bool heading=false) {
        auto label=makeLabel(value,heading?GlassScreenTypography::headline:GlassScreenTypography::footnote,UIColor::secondaryLabel);
        _content->addSubview(label);_blocks.push_back({label,heading?24.f:36.f,heading?8.f:20.f,true,{}});
    }
    void section(const std::string& title) {
        text(title,true);_currentCard=new_shared<UIView>();_currentCard->setBackgroundColor(UIColor::secondarySystemGroupedBackground);
        _currentCard->layer()->setCornerRadius(20);_currentCard->setClipsToBounds(true);_content->addSubview(_currentCard);
        _currentRows=std::make_shared<std::vector<std::shared_ptr<SettingRow>>>();_blocks.push_back({_currentCard,0,24,false,_currentRows});_currentBlock=_blocks.size()-1;
    }
    std::shared_ptr<SettingRow> row(const std::string& id,const std::string& title,std::function<void()> callback) {
        auto control=new_shared<SettingRow>(title);control->primaryAction=UIAction(title,std::move(callback));
        _currentRows->push_back(control);_currentCard->addSubview(control);_blocks[_currentBlock].height=NXFloat(_currentRows->size())*52;
        _actions[id]=control;_rows[id]=control;return control;
    }
    void toggle(const std::string& id,const std::string& title,bool value,std::function<void(bool)> apply) {
        auto setting=new_shared<UISwitch>();setting->setOn(value);_toggles[id]=setting;
        auto control=row(id,title,[weak=std::weak_ptr<UISwitch>(setting),apply]{if(const auto setting=weak.lock()){setting->setOn(!setting->isOn(),true);apply(setting->isOn());}});
        control->setAccessory(setting);
        setting->addAction(UIAction(title,[weak=std::weak_ptr<UISwitch>(setting),apply]{if(const auto setting=weak.lock())apply(setting->isOn());}),UIControlEvent::valueChanged);
    }
    void buildCatalog() {
        text("Open a control to try touch input, change its public settings, and inspect the events it sends.");section("Controls");
        const auto push=_push;
        row("slider","Slider",[push]{push(Page::slider);})->setValue("UISlider ›");
        row("switch","Switch",[push]{push(Page::switchControl);})->setValue("UISwitch ›");
    }
    void observe(const std::shared_ptr<UIControl>& control) {
        const auto weak=weak_from_base<ControlsView>();
        const std::array events={UIControlEvent::valueChanged,UIControlEvent::touchDown,UIControlEvent::touchUpInside,UIControlEvent::touchUpOutside,UIControlEvent::touchCancel};
        const std::array names={"valueChanged","touchDown","touchUpInside","touchUpOutside","touchCancel"};
        for(size_t i=0;i<events.size();++i)control->addAction(UIAction(names[i],[weak,i,name=names[i]]{if(const auto self=weak.lock()){
            ++self->_counts[i];self->_lastEvent=name;self->refresh();
        }}),events[i]);
    }
    void preview(const std::shared_ptr<UIControl>& control) {
        _actions["control"]=control;_preview=new_shared<Preview>(control);_content->addSubview(_preview);_blocks.push_back({_preview,122,24,false,{}});observe(control);
    }
    void eventSection() {
        text("Events",true);_events=makeLabel("",GlassScreenTypography::footnote,UIColor::secondaryLabel);
        _content->addSubview(_events);_blocks.push_back({_events,80,20,true,{}});
        section("Session");const auto weak=weak_from_base<ControlsView>();
        row("clear-events","Clear event counts",[weak]{if(const auto self=weak.lock()){self->_counts={};self->_lastEvent="Cleared";self->refresh();}});
    }
    static std::optional<UIColor> tint(size_t index) {
        return index==0?std::nullopt:std::optional<UIColor>{index==1?UIColor::systemBlue:index==2?UIColor::systemPink:UIColor::systemMint};
    }
    static const char* tintName(size_t index){return std::array{"Default","Blue","Pink","Mint"}[index];}
    void buildSlider() {
        text("Drag the knob, then try continuous delivery, discrete ticks, custom tints and the thumbless style. Programmatic changes do not send valueChanged.");
        _slider=new_shared<UISlider>();_slider->setValue(.5);preview(_slider);const auto weak=weak_from_base<ControlsView>();
        section("Value and interaction");
        row("value","Set value",[weak]{if(const auto self=weak.lock()){
            self->_valueStep=(self->_valueStep+1)%3;self->_slider->setValue(self->_slider->minimumValue()+(self->_slider->maximumValue()-self->_slider->minimumValue())*NXFloat(self->_valueStep)*.5f,self->_animated);
            self->_lastEvent="setValue (no event)";self->refresh();
        }});
        row("range","Range",[weak]{if(const auto self=weak.lock()){
            self->_range=(self->_range+1)%3;self->_slider->setMinimumValue(self->_range==1?-1:0);self->_slider->setMaximumValue(self->_range==2?100:1);self->configureTicks();self->refresh();
        }});
        toggle("continuous","Continuous",true,[weak](bool value){if(const auto self=weak.lock()){self->_slider->setContinuous(value);self->refresh();}});
        toggle("enabled","Enabled",true,[weak](bool value){if(const auto self=weak.lock()){self->_slider->setEnabled(value);self->refresh();}});
        toggle("animated","Animate value changes",true,[weak](bool value){if(const auto self=weak.lock())self->_animated=value;});
        section("Appearance");
        row("style","Style",[weak]{if(const auto self=weak.lock()){self->_slider->setSliderStyle(self->_slider->sliderStyle()==UISliderStyle::defaultStyle?UISliderStyle::thumbless:UISliderStyle::defaultStyle);self->refresh();}});
        row("minimum-tint","Filled track tint",[weak]{if(const auto self=weak.lock()){self->_minimumTint=(self->_minimumTint+1)%4;self->_slider->setMinimumTrackTintColor(tint(self->_minimumTint));self->refresh();}});
        row("maximum-tint","Empty track tint",[weak]{if(const auto self=weak.lock()){self->_maximumTint=(self->_maximumTint+1)%4;self->_slider->setMaximumTrackTintColor(tint(self->_maximumTint));self->refresh();}});
        row("thumb-tint","Thumb tint",[weak]{if(const auto self=weak.lock()){self->_thumbTint=(self->_thumbTint+1)%4;self->_slider->setThumbTintColor(tint(self->_thumbTint));self->refresh();}});
        row("ticks","Discrete ticks",[weak]{if(const auto self=weak.lock()){self->_ticks=(self->_ticks+1)%3;self->configureTicks();self->refresh();}});
        toggle("end-images","Endpoint images",false,[weak](bool value){if(const auto self=weak.lock()){
            self->_endImages=value;self->_slider->setMinimumValueImage(value?UIImage::systemImageNamed("speaker.fill",18):nullptr);
            self->_slider->setMaximumValueImage(value?UIImage::systemImageNamed("speaker.wave.3.fill",18):nullptr);self->refresh();
        }});
        backgroundSetting();eventSection();
    }
    void configureTicks() {
        if(!_ticks){_slider->setTrackConfiguration(nullptr);return;}
        const auto count=_ticks==1?5:11;std::vector<std::shared_ptr<UISliderTick>> ticks;
        for(int i=0;i<count;++i)ticks.push_back(UISliderTick::tickWithPosition(_slider->minimumValue()+(_slider->maximumValue()-_slider->minimumValue())*NXFloat(i)/NXFloat(count-1)));
        auto configuration=UISliderTrackConfiguration::configurationWithTicks(std::move(ticks));
        configuration->setMinimumEnabledValue(_slider->minimumValue());configuration->setMaximumEnabledValue(_slider->maximumValue());_slider->setTrackConfiguration(configuration);
    }
    void buildSwitch() {
        text("Tap or drag the switch. Change On below to compare programmatic animation with the lifted knob's touch interaction.");
        _switch=new_shared<UISwitch>();preview(_switch);const auto weak=weak_from_base<ControlsView>();
        section("State and interaction");
        toggle("on","On",false,[weak](bool value){if(const auto self=weak.lock()){self->_switch->setOn(value,self->_animated);self->_lastEvent="setOn (no event)";self->refresh();}});
        toggle("enabled","Enabled",true,[weak](bool value){if(const auto self=weak.lock()){self->_switch->setEnabled(value);self->refresh();}});
        toggle("animated","Animate state changes",true,[weak](bool value){if(const auto self=weak.lock())self->_animated=value;});
        section("Appearance");
        row("on-tint","On tint",[weak]{if(const auto self=weak.lock()){self->_switchTint=(self->_switchTint+1)%4;self->_switch->setOnTintColor(tint(self->_switchTint));self->refresh();}});
        row("thumb-tint","Thumb tint",[weak]{if(const auto self=weak.lock()){self->_thumbTint=(self->_thumbTint+1)%4;self->_switch->setThumbTintColor(tint(self->_thumbTint));self->refresh();}});
        row("style","Preferred style",[weak]{if(const auto self=weak.lock()){
            self->_switch->setPreferredStyle(UISwitchStyle((int(self->_switch->preferredStyle())+1)%3));self->refresh();
        }});
        text("Automatic and checkbox requests resolve to the sliding style on this portable implementation, matching the measured iPhone provider.");
        backgroundSetting();eventSection();
    }
    void backgroundSetting() {
        const auto weak=weak_from_base<ControlsView>();
        row("background","Preview background",[weak]{if(const auto self=weak.lock()){
            self->_background=(self->_background+1)%4;
            self->_preview->setBackgroundColor(self->_background==0?UIColor::secondarySystemGroupedBackground:self->_background==1?UIColor::white:self->_background==2?UIColor::black:UIColor(209,71,61));self->refresh();
        }});
    }
    void refresh() {
        if(_page==Page::catalog)return;
        if(_slider) {
            const auto value=number(_slider->value());_preview->setValue("Value "+value+"   ·   "+number(_slider->minimumValue())+" … "+number(_slider->maximumValue()));
            _rows["value"]->setValue(value+" ›");_rows["range"]->setValue(std::array{"0 … 1","−1 … 1","0 … 100"}[_range]);
            _rows["style"]->setValue(_slider->sliderStyle()==UISliderStyle::defaultStyle?"Default":"Thumbless");
            _rows["minimum-tint"]->setValue(tintName(_minimumTint));_rows["maximum-tint"]->setValue(tintName(_maximumTint));
            _rows["ticks"]->setValue(std::array{"Off","5 ticks","11 ticks"}[_ticks]);
        }else {
            _preview->setValue(std::string(_switch->isOn()?"On":"Off")+"   ·   "+(_switch->isEnabled()?"Enabled":"Disabled")+"   ·   Sliding");
            if(_toggles["on"]->isOn()!=_switch->isOn())_toggles["on"]->setOn(_switch->isOn());
            _rows["on-tint"]->setValue(tintName(_switchTint));
            _rows["style"]->setValue(std::array{"Automatic","Checkbox","Sliding"}[int(_switch->preferredStyle())]);
        }
        _rows["thumb-tint"]->setValue(tintName(_thumbTint));_rows["background"]->setValue(std::array{"System","White","Black","Coral"}[_background]);
        if(_events)_events->setText("Last: "+_lastEvent+"\nvalueChanged: "+std::to_string(_counts[0])+"   touchDown: "+std::to_string(_counts[1])+"\nUp inside: "+std::to_string(_counts[2])+"   Up outside: "+std::to_string(_counts[3])+"   Cancel: "+std::to_string(_counts[4]));
    }
};
}
SystemControlsDemoViewController::SystemControlsDemoViewController(Page page):_page(page) {
    setTitle(page==Page::catalog?"System UI elements":page==Page::slider?"Slider":"Switch");
    navigationItem()->setLargeTitleDisplayMode(UINavigationItemLargeTitleDisplayMode::always);
}
void SystemControlsDemoViewController::loadView() {
    const auto weak=weak_from_base<SystemControlsDemoViewController>();
    setView(new_shared<ControlsView>(_page,[weak](Page page){if(const auto self=weak.lock())if(const auto nav=self->navigationController())nav->pushViewController(new_shared<SystemControlsDemoViewController>(page),true);}));
}
std::shared_ptr<UIControl> SystemControlsDemoViewController::action(const std::string& id){loadViewIfNeeded();return std::static_pointer_cast<ControlsView>(view())->action(id);}
std::shared_ptr<UISlider> SystemControlsDemoViewController::slider(){loadViewIfNeeded();return std::static_pointer_cast<ControlsView>(view())->slider();}
std::shared_ptr<UISwitch> SystemControlsDemoViewController::switchControl(){loadViewIfNeeded();return std::static_pointer_cast<ControlsView>(view())->switchControl();}
