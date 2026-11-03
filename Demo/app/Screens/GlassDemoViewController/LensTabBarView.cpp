#include "LensTabBarView.hpp"
#include "GlassScreenTypography.hpp"
#include "DemoSystemSymbol.hpp"
#include "../../../../UIKit/lib/UITabBarViewInternal.hpp"
#include "../../../../UIKitCore/lib/LayerCornerGeometry.hpp"
#include "include/core/SkPathBuilder.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
using namespace NXKit;
namespace {
// Portable filled tab symbols for the full-screen phone example. The older
// three-item optical fixtures keep their existing independent source artwork.
class PhoneTabSymbolLayer final : public CALayer {
public:
    int shape; UIColor ink;
    PhoneTabSymbolLayer(int shape,UIColor ink):shape(shape),ink(ink) {}
    explicit PhoneTabSymbolLayer(PhoneTabSymbolLayer* old):CALayer(old),shape(old->shape),ink(old->ink) {}
    std::shared_ptr<CALayer> copy() override { return new_shared<PhoneTabSymbolLayer>(this); }
    void draw(SkCanvas* canvas) override {
        canvas->save();canvas->scale(bounds().width()/24,bounds().height()/24);
        SkPaint p; p.setColor(ink.raw()); p.setAntiAlias(true);
        if(shape==0) {
            SkPathBuilder roof; roof.moveTo(1,11);roof.lineTo(12,1);roof.lineTo(23,11);
            p.setStyle(SkPaint::kStroke_Style);p.setStrokeWidth(2.2f);p.setStrokeCap(SkPaint::kRound_Cap);p.setStrokeJoin(SkPaint::kRound_Join);canvas->drawPath(roof.detach(),p);
            p.setStyle(SkPaint::kFill_Style);SkPathBuilder house;house.setFillType(SkPathFillType::kEvenOdd);house.moveTo(4,11);house.lineTo(12,4);house.lineTo(20,11);house.lineTo(20,23);house.lineTo(4,23);house.close();house.addRect(SkRect::MakeXYWH(10,15,4,8));canvas->drawPath(house.detach(),p);
        } else if(shape==1) {
            SkPathBuilder star;for(int i=0;i<10;++i){float angle=-1.57079633f+i*.62831853f,r=i%2?4.8f:11.5f;float x=12+std::cos(angle)*r,y=12+std::sin(angle)*r;if(i)star.lineTo(x,y);else star.moveTo(x,y);}star.close();canvas->drawPath(star.detach(),p);
        } else if(shape==3) {
            canvas->drawRoundRect(SkRect::MakeXYWH(1,2,5,21),1,1,p);canvas->drawRoundRect(SkRect::MakeXYWH(8,4,5,19),1,1,p);
            canvas->save();canvas->rotate(-10,18,13);canvas->drawRoundRect(SkRect::MakeXYWH(16,2,5,21),1,1,p);canvas->restore();
        } else {
            for(int i=0;i<3;++i){canvas->drawRoundRect(SkRect::MakeXYWH(1,4+i*7,22,2),1,1,p);canvas->drawRoundRect(SkRect::MakeXYWH(i==1?6:15,1+i*7,4,8),1,1,p);}
        }
        canvas->restore();
    }
};
class PhoneTabSymbol final : public UIView {
public:
    PhoneTabSymbol(int shape,UIColor color):UIView({},new_shared<PhoneTabSymbolLayer>(shape,color)) { setUserInteractionEnabled(false); }
    void setColor(UIColor color) {
        auto ink=std::static_pointer_cast<PhoneTabSymbolLayer>(layer());ink->ink=color;
        if(auto active=ink->presentation())std::static_pointer_cast<PhoneTabSymbolLayer>(active)->ink=color;
        ink->setNeedsDisplay();
    }
};
std::shared_ptr<UIView> symbol(int index, const UIColor& color) {
    auto view = new_shared<UIView>(); view->setUserInteractionEnabled(false);
    auto line = [&](NXRect rect, float angle = 0) {
        auto stroke=new_shared<UIView>(rect); stroke->setBackgroundColor(color);
        stroke->layer()->setCornerRadius(1); stroke->setTransform(NXAffineTransform::rotationBy(angle)); view->addSubview(stroke);
    };
    if (index==0) { line({2,7,12,2},-40); line({10,7,12,2},40); line({5,11,2,11}); line({17,11,2,11}); line({5,20,14,2}); }
    if (index==1) { line({11,1,2,22}); line({1,11,22,2}); line({4,4,16,2},45); line({4,18,16,2},-45); }
    if (index==2) { for (int i=0;i<3;i++) { line({2,4+i*7.0f,20,2}); auto knob=new_shared<UIView>(NXRect(i==1?6:14,1+i*7.0f,4,8)); knob->setBackgroundColor(color); knob->layer()->setCornerRadius(2); view->addSubview(knob); } }
    if (index==3) { line({3,2,2,20}); line({9,2,2,20}); line({16,3,2,20},-14); }
    return view;
}
}
LensTabBarView::LensTabBarView(bool compact, size_t itemCount) {
    if(itemCount!=3 && itemCount!=4) throw std::invalid_argument("Demo supports three or four items");
    auto& s=Internal::TabBarViewAccess::state(*this); s._compact=compact;
    const char* titles3[]={"Home","Favorites","Settings"};
    const char* titles4[]={"Home","Favorites","Library","Settings"};
    const auto titles=itemCount==4?titles4:titles3;
    std::vector<std::shared_ptr<UITabBarItem>> values;
    for(size_t i=0;i<itemCount;++i) values.push_back(std::make_shared<UITabBarItem>(titles[i],nullptr,std::intptr_t(i)));
    setItems(std::move(values));
    for(size_t i=0;i<itemCount;++i) {
        const auto icon=itemCount==4?(i==2?3:(i==3?2:int(i))):int(i);
        const char* symbols[]={"house.fill","star.fill","books.vertical.fill","slider.horizontal.3"};
        auto normal=itemCount==4?std::static_pointer_cast<UIView>(new_shared<DemoSystemSymbol>(symbols[i],28,400,UIColor::label,new_shared<PhoneTabSymbol>(icon,UIColor::label))):symbol(icon,UIColor::label);
        auto selected=itemCount==4?std::static_pointer_cast<UIView>(new_shared<DemoSystemSymbol>(symbols[i],28,400,UIColor::systemBlue,new_shared<PhoneTabSymbol>(icon,UIColor::systemBlue))):symbol(icon,UIColor::systemBlue);
        if(itemCount==4) {
            std::static_pointer_cast<DemoSystemSymbol>(normal)->setFitsSymbolToBounds(true);
            std::static_pointer_cast<DemoSystemSymbol>(selected)->setFitsSymbolToBounds(true);
        }
        s._normalSymbols[i]->removeFromSuperview(); s._selectedSymbols[i]->removeFromSuperview();
        s._normalSymbols[i]=normal; s._selectedSymbols[i]=selected;
        s._controls[i]->insertSubviewBelow(normal,s._normalLabels[i]);
        s._selectedItems[i]->insertSubviewBelow(selected,s._selectedLabels[i]);
        s._normalLabels[i]->setFontSize(compact?17:11); s._selectedLabels[i]->setFontSize(compact?17:11);
        s._normalLabels[i]->setFontWeight(400);s._selectedLabels[i]->setFontWeight(400);
        if(itemCount==4) {
            GlassScreenTypography::apply(s._normalLabels[i],GlassScreenTypography::tabCaption);
            GlassScreenTypography::apply(s._selectedLabels[i],GlassScreenTypography::selectedTabCaption);
        }
        s._normalLabels[i]->setTextAlignment(itemCount==4?NSTextAlignment::natural:NSTextAlignment::center);
        s._selectedLabels[i]->setTextAlignment(itemCount==4?NSTextAlignment::natural:NSTextAlignment::center);
    }
    const auto weak=weak_from_base<LensTabBarView>();
    s.customLayout=[weak] {if(auto self=weak.lock())self->arrangeFixtures();};
    setSelectedIndex(0);
    s.legacySelectionChanged=[weak](size_t index) {if(auto self=weak.lock();self&&self->onSelectionChanged)self->onSelectionChanged(index);};
}
void LensTabBarView::setSelectedIndex(size_t index,bool animated) {
    const auto values=items();if(index>=values.size())throw std::out_of_range("Lens tab index");
    Internal::TabBarViewAccess::select(*this,values[index],animated);
}
size_t LensTabBarView::selectedIndex() const {return Internal::TabBarViewAccess::state(*this)._selected;}
void LensTabBarView::setSelectionPosition(float position) {Internal::TabBarViewAccess::position(*this,position);}
float LensTabBarView::selectionPosition() const {return Internal::TabBarViewAccess::state(*this)._position;}
void LensTabBarView::setLifted(bool lifted,bool animated) {Internal::TabBarViewAccess::lift(*this,lifted,animated);}
bool LensTabBarView::isLifted() const {return Internal::TabBarViewAccess::state(*this)._lifted;}
std::shared_ptr<LensView> LensTabBarView::lens() const {return Internal::TabBarViewAccess::state(*this)._lens;}
std::shared_ptr<BackdropEffectView> LensTabBarView::glass() const {return Internal::TabBarViewAccess::state(*this)._glass;}
const std::vector<std::shared_ptr<UIControl>>& LensTabBarView::controls() const {return Internal::TabBarViewAccess::state(*this)._controls;}
void LensTabBarView::setComparisonContent(std::shared_ptr<UIImage> normal,std::shared_ptr<UIImage> selected) {Internal::TabBarViewAccess::comparisonContent(*this,std::move(normal),std::move(selected));}
void LensTabBarView::setComparisonGeometry(NXRect frame) {Internal::TabBarViewAccess::comparisonGeometry(*this,frame);}
void LensTabBarView::advanceLift(double seconds) {Internal::TabBarViewAccess::advance(*this,seconds);}
bool LensTabBarView::hasPulse() const {return bool(Internal::TabBarViewAccess::state(*this)._liftLink);}
void LensTabBarView::arrangeFixtures() {
    auto& s=Internal::TabBarViewAccess::state(*this);
    const auto w=std::max(0.0f,bounds().width()),h=std::max(0.0f,bounds().height());
    s._glass->setFrame({bounds().origin.x,bounds().origin.y,w,h}); s._glass->layer()->setCornerRadius(h*.5f); s._normal->setFrame({0,0,w,h});
    s._normal->mask()->setFrame({0,0,w,h});
    s._selectedHost->setFrame({0,0,w,h});
    std::vector<float> left(s._count),width(s._count);
    if (s._compact && s._count==3) { const auto scale=w/293.5f; left[0]=4*scale; left[1]=92*scale; left[2]=195.5f*scale; width[0]=84*scale; width[1]=99.5f*scale; width[2]=94*scale; }
    else for (size_t i=0;i<s._count;i++) { left[i]=i*w/s._count+(w/s._count-std::min(116.0f,w/s._count-4))*.5f; width[i]=std::max(0.0f,std::min(116.0f,w/s._count-4)); }
    s._selectionStep=w/s._count;
    if(s._count==4) {
        const auto itemWidth=std::max(0.0f,std::min(116.0f,w/4+5));
        s._selectionStep=std::max(0.0f,(w-8-itemWidth)/3);
        for(size_t i=0;i<s._count;++i) { left[i]=4+i*s._selectionStep; width[i]=itemWidth; }
    }
    s._selectionCenters.resize(s._count);
    for(size_t i=0;i<s._count;++i) s._selectionCenters[i]=left[i]+width[i]*.5f;
    const int a=std::min(int(s._count)-2,int(s._position)),b=a+1; const float t=s._position-a;
    // UIKit's real tab control adds 8pt on each edge while lifted (both the
    // 54pt phone lens and 36pt compact lens), separately from platter inflation.
    const auto growth=s._lifted?16.0f:0.0f;
    const auto lensW=std::max(0.0f,width[a]+(width[b]-width[a])*t+growth);
    const auto lensH=std::max(0.0f,(s._compact?h-8:std::min(56.0f,h-8))+growth);
    const auto centerX=(left[a]+width[a]*.5f)*(1-t)+(left[b]+width[b]*.5f)*t;
    const auto lensFrame=s._comparisonFrame.value_or(NXRect(centerX-lensW*.5f,(h-lensH)*.5f,lensW,lensH));
    s._lens->setFrame(lensFrame);s._selectionFill->setFrame(s._lens->restingBackgroundView()->bounds()); s._selectedContent->setFrame({0,0,w,h});
    for (size_t i=0;i<s._count;i++) {
        // Full-row local coordinates preserve the supplied source placement.
        // Native selected items enlarge 16% around each item center, including
        // items partly visible through a lens held between two selections.
        const auto itemCenter=s._selectionCenters[i];
        auto itemLayer=s._selectedItems[i]->layer();
        itemLayer->setAnchorPoint({0,0});
        itemLayer->setBounds({0,0,w,h}); itemLayer->setPosition({0,0});
        // Use an explicit pivot translation. At rest this is exactly identity,
        // avoiding round-off in (center / width) * width at fractional sizes.
        const auto scale=s._lifted?1.16f:1.f;
        s._selectedItems[i]->setTransform({scale,0,0,scale,(1-scale)*itemCenter,(1-scale)*h*.5f});
        s._controls[i]->setFrame({i*w/s._count,0,w/s._count,h}); s._controls[i]->layer()->setCornerRadius(h*.5f);
        float localX=w/(s._count*2)-12;
        if (s._compact && s._count==3) {
            const float textWidth=i==0?45.0f:(i==1?70.5f:65.0f);
            const auto start=left[i]+(width[i]-textWidth-28)*.5f;
            s._normalSymbols[i]->setFrame({start-i*w/s._count,(h-24)*.5f,24,24}); s._selectedSymbols[i]->setFrame({start,(h-24)*.5f,24,24});
            s._normalLabels[i]->setFrame({start+28-i*w/s._count,0,textWidth,h}); s._selectedLabels[i]->setFrame({start+28,0,textWidth,h});
        } else if(s._count==4) {
            const auto center=left[i]+width[i]*.5f, slot=w/s._count;
            // UIImageView bounds measured independently in the native four-tab
            // fixture; intrinsic symbol padding differs between UIKit/AppKit.
            constexpr NXFloat iconWidth[]={100.0f/3,91.0f/3,100.0f/3,88.0f/3};
            constexpr NXFloat iconHeight[]={28,85.0f/3,88.0f/3,71.0f/3};
            constexpr NXFloat iconTop[]={10,9,9,12};
            s._normalSymbols[i]->setFrame({center-i*slot-iconWidth[i]*.5f,iconTop[i],iconWidth[i],iconHeight[i]});
            s._selectedSymbols[i]->setFrame({center-iconWidth[i]*.5f,iconTop[i],iconWidth[i],iconHeight[i]});
            const auto traits=traitCollection();
            const auto scale=traits?std::max(1.0f,traits->displayScale()):1;
            const auto captionFrame=[&](const auto& label,NXFloat offset) {
                // A geometry-only harness can construct chrome before the
                // application/font context exists. Real layout measures ink.
                constexpr NXFloat fallbackNormal[]={88.0f/3,46,107.0f/3,127.0f/3};
                constexpr NXFloat fallbackSelected[]={29,136.0f/3,35,124.0f/3};
                const auto size=SkiaCtx::main()?label->sizeThatFits({1000,18})
                    :NXSize(label->fontWeight()==600?fallbackNormal[i]:fallbackSelected[i],12);
                // Native tab labels align their tight leading edge upward to
                // the display grid. Avoid rounding a float epsilon to a pixel.
                return NXRect(std::ceil((center-size.width*.5f)*scale-1e-4f)/scale-offset,h-23,size.width,size.height);
            };
            s._normalLabels[i]->setFrame(captionFrame(s._normalLabels[i],i*slot));
            s._selectedLabels[i]->setFrame(captionFrame(s._selectedLabels[i],0));
        } else {
            s._normalSymbols[i]->setFrame({localX,std::min(13.0f,h-49),24,24}); s._selectedSymbols[i]->setFrame({i*w/s._count+localX,std::min(13.0f,h-49),24,24});
            s._normalLabels[i]->setFrame({0,h-32,w/s._count,18}); s._selectedLabels[i]->setFrame({i*w/s._count,h-32,w/s._count,18});
        }
    }
    if (s._normalImage) s._normalImage->setFrame({0,0,w,h});
    if (s._selectedImage) s._selectedImage->setFrame({0,0,w,h});
}
void LensTabBarView::setAccentColor(UIColor color) {
    setTintColor(color);
    auto& s=Internal::TabBarViewAccess::state(*this);
    // A literal accent opts out, just as a custom UITabBar.tintColor does.
    const UIColor semantic[]={UIColor::systemBlue,UIColor::systemRed,UIColor::systemGreen,
        UIColor::systemOrange,UIColor::systemPurple,UIColor::systemMint,UIColor::systemYellow,
        UIColor::systemIndigo,UIColor::systemGray,UIColor::systemTeal,UIColor::systemCyan,
        UIColor::systemPink,UIColor::systemBrown,UIColor::tint};
    s._lens->setContentVibrancyEnabled(std::any_of(std::begin(semantic),std::end(semantic),[&](const auto& tint){return color==tint;}));
    for(const auto& label:s._selectedLabels) label->setTextColor(color);
    const auto recolor=[&](const auto& self,const std::shared_ptr<UIView>& view)->void {
        view->setTintColor(color);
        if(auto symbol=std::dynamic_pointer_cast<PhoneTabSymbol>(view)) symbol->setColor(color);
        if(view->backgroundColor()) view->setBackgroundColor(color);
        for(const auto& child:view->subviews()) self(self,child);
    };
    for(const auto& icon:s._selectedSymbols) recolor(recolor,icon);
}
