#include <UISlider.h>
#include <LensView.h>
#include <UIImageView.h>
#include <UIWindow.h>
#include <UIScrollView.h>
#include <UITouch.h>
#include <UIEvent.h>
#include <UINib.h>
#include <BackdropEffectView.h>
#include <UIKitRegistration.h>
#include "RasterTestContext.hpp"
#include "../../UIKitCore/lib/LayerAnimationTimeline.hpp"
#include "include/core/SkSurface.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <filesystem>
#include <fstream>
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
using namespace NXKit;
#include "SliderNativeGeometryData.hpp"
#include "SliderNativeEndpointData.hpp"
#include "SliderNativeTouchData.hpp"
#include "SliderNativeLiftData.hpp"
#include "SliderNativeReleaseData.hpp"
#include "../lib/SliderThumbTransition.hpp"
#include "../lib/LensMaterial.hpp"
namespace NXKit {
class GlassMaterialTestHarness {public:static auto traits(bool dark,float scale) {auto t=std::make_shared<UITraitCollection>();t->_displayScale=scale;t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;return t;}};
class UISliderTestHarness {
public:
 static auto lens(const std::shared_ptr<UISlider>& slider){return slider->_lens;}
 static auto track(const std::shared_ptr<UISlider>& slider){return slider->_minimumTrack;}
 static void hideTrack(const std::shared_ptr<UISlider>& slider){slider->_track->setHidden(true);}
 static void advance(const std::shared_ptr<UISlider>& slider,double seconds){slider->advanceThumb(seconds);}
 static void advanceValue(const std::shared_ptr<UISlider>& slider,double seconds){slider->advanceRelease(seconds);}
 static void release(const std::shared_ptr<UISlider>& slider,double seconds=.5){
  slider->advanceRelease(seconds);
  slider->advanceThumb(seconds);
  if(seconds>=.4){slider->_lens->layer()->removeAllAnimations();slider->_thumbImage->layer()->removeAllAnimations();}
 }
};
class LensViewTestHarness {public:
 static void advance(const std::shared_ptr<LensView>& lens,double dt){lens->advance(dt);}
 static auto progress(const std::shared_ptr<LensView>& lens){return lens->_progress;}
 static auto restingOpacity(const std::shared_ptr<LensView>& lens){return lens->_restingHost->layer()->presentationOrSelf()->opacity();}
 static auto backdrop(const std::shared_ptr<LensView>& lens){return lens->_warpBackdrop;}
 static auto opticalScale(const std::shared_ptr<LensView>& lens){return lens->_optics->layer()->contentsScale();}

};
class UIGestureRecognizerTestHarness {
public:
 static void send(const std::shared_ptr<UIWindow>& w,const std::shared_ptr<UITouch>& t,UITouchPhase phase,NXKit::NXPoint p,double timestamp=-1) {
  if(timestamp>=0){t->_timestamp.startTime.tv_sec=long(timestamp);t->_timestamp.startTime.tv_usec=long((timestamp-long(timestamp))*1000000);}
  t->_window=w;t->_phase=phase;t->updateAbsoluteLocation(p);auto e=new_shared<UIEvent>();e->_allTouches.insert(t);w->sendEvent(e);
 }
};
}
namespace {
int failures=0;
void expect(bool v,const char* m){if(!v){++failures;std::cerr<<"FAIL: "<<m<<'\n';}}
bool near(float a,float b){return std::abs(a-b)<.001f;}
void nativeLiftMotion() {
 Internal::SliderThumbTransition motion;motion.retarget(true);
 auto compare=[&](const auto& samples) {
  double last=0;float maximumGeometry=0,maximumAlpha=0;
  for(const auto& sample:samples) {
   motion.advance(sample.time-last);last=sample.time;
   maximumGeometry=std::max(maximumGeometry,std::abs(float(37+20.35*motion.size.value)-sample.width));
   maximumAlpha=std::max(maximumAlpha,std::abs(std::clamp(float(1-motion.glass.value),0.f,1.f)-sample.whiteOpacity));
  }
  expect(maximumGeometry<.12f,"native presentation-bounds curve matches the small-lens spring");
  expect(maximumAlpha<.015f,"native resting-fill opacity curve matches independently from geometry");
  std::cout<<"Native lens motion max geometry="<<maximumGeometry<<"pt alpha="<<maximumAlpha<<'\n';
 };
 compare(sliderNativeLift);motion.finish(true);motion.retarget(false);compare(sliderNativeRest);
 // Reversing an in-flight release must preserve both visible values; the
 // retarget impulse changes velocity rather than jumping either layer.
 motion.finish(true);motion.retarget(false);motion.advance(.1);
 const auto size=motion.size.value,glass=motion.glass.value;
 motion.retarget(true);expect(motion.size.value==size&&motion.glass.value==glass,"re-grab retarget preserves visible size and fill opacity");
 Internal::SliderThumbTransition a,b;a.retarget(true);b.retarget(true);
 for(int i=0;i<12;++i)a.advance(1./120);b.advance(.1);
 expect(std::abs(a.size.value-b.size.value)<.000001,"lens response is independent of refresh rate");
}
void rect(NXRect a,NXRect b,const char* m){if(!(near(a.minX(),b.minX())&&near(a.minY(),b.minY())&&near(a.width(),b.width())&&near(a.height(),b.height()))){++failures;std::cerr<<"FAIL: "<<m<<" actual="<<a.minX()<<','<<a.minY()<<','<<a.width()<<','<<a.height()<<" expected="<<b.minX()<<','<<b.minY()<<','<<b.width()<<','<<b.height()<<'\n';}}
auto image(int w,int h){auto s=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w,h));s->getCanvas()->clear(SkColorSetRGB(255,55,95));return std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(s->makeImageSnapshot()),1);}
class ThemeRoot:public UIView {
public:
 explicit ThemeRoot(std::shared_ptr<UITraitCollection> traits) {_traitCollection=std::move(traits);setFrame({0,0,340,110});setBackgroundColor(UIColor::systemBackground);}
};
class ThemeWindow:public UIWindow {
public:
 explicit ThemeWindow(const std::shared_ptr<UITraitCollection>& traits){_traitCollection=traits;}
};
void nativeGeometry() {
 for(const auto& c:sliderNativeGeometry) {
  auto s=new_shared<UISlider>();s->setFrame(c.bounds);const std::string id=c.id;
  if(id.find("thumb-image")!=std::string::npos)s->setThumbImage(image(30,40),UIControlState::normal);
  if(id=="min-image"||id=="both-images")s->setMinimumValueImage(image(18,20));
  if(id=="max-image"||id=="both-images")s->setMaximumValueImage(image(26,24));
  if(id.find("highlighted")!=std::string::npos)s->setHighlighted(true);
  if(id=="selected")s->setSelected(true);
  s->setMinimumValue(c.minimum);s->setMaximumValue(c.maximum);s->setValue(c.value);s->setEnabled(c.enabled);
  s->setSliderStyle(UISliderStyle(c.style));
  if(id=="ticks")s->setTrackConfiguration(UISliderTrackConfiguration::configurationWithNumberOfTicks(5));
  const auto track=s->trackRectForBounds(c.bounds);
  rect(track,c.track,c.id);rect(s->thumbRectForBounds(c.bounds,track,0),c.thumb0,c.id);
  rect(s->thumbRectForBounds(c.bounds,track,.5),c.thumbHalf,c.id);rect(s->thumbRectForBounds(c.bounds,track,1),c.thumb1,c.id);
  rect(s->minimumValueImageRectForBounds(c.bounds),c.minImage,c.id);rect(s->maximumValueImageRectForBounds(c.bounds),c.maxImage,c.id);
  expect(s->sizeThatFits({300,100})==NXKit::NXSize(300,34),"native sizeThatFits remains34pt high");
 }
 std::cout<<"Compared "<<std::size(sliderNativeGeometry)<<" native slider geometry cases\n";
}
void valuesAndImages() {
 int events=0;auto s=new_shared<UISlider>(NXRect(0,0,300,44),UIAction("Value",[&]{++events;}));
 expect(s->value()==0&&s->minimumValue()==0&&s->maximumValue()==1&&s->isContinuous(),"UIKit slider defaults");
 s->setValue(2,true);expect(s->value()==1&&events==0,"animated programmatic value pins without actions");
 s->setMinimumValue(2);expect(s->minimumValue()==2&&s->maximumValue()==2&&s->value()==2,"raising min above max collapses range and value");
 s->setMaximumValue(-2);expect(s->minimumValue()==-2&&s->maximumValue()==-2&&s->value()==-2,"lowering max below min collapses range and value");
 s->setMaximumValue(20);s->setValue(5);s->setMinimumValue(10);expect(s->value()==10&&events==0,"range changes silently pin value");
 for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}) {
  bool rejected=false;try{s->setValue(bad);}catch(const std::invalid_argument&){rejected=true;}expect(rejected,"nonfinite value rejected at boundary");
 }
 auto normal=image(20,24),selected=image(32,36);s->setThumbImage(normal,UIControlState::normal);s->setThumbImage(selected,UIControlState::selected);
 s->setSelected(true);expect(s->currentThumbImage()==selected,"selected state resolves its custom thumb");
 s->setSelected(false);expect(s->currentThumbImage()==normal,"normal image restored after state change");
 s->setMinimumTrackImage(normal,UIControlState::normal);s->setMaximumTrackImage(selected,UIControlState::normal);
 expect(s->currentMinimumTrackImage()==normal&&s->currentMaximumTrackImage()==selected,"track images are independently retained");
 s->setThumbImage(nullptr,UIControlState::normal);expect(!s->currentThumbImage(),"clearing custom thumb restores lens");
 expect(s->applyXMLAttribute("sliderStyle","thumbless")&&s->applyXMLAttribute("continuous","false")&&!s->isContinuous()&&!s->applyXMLAttribute("value","nan"),"XML slider properties reject invalid values");
 auto configuration=UISliderTrackConfiguration::configurationWithNumberOfTicks(5);s->setMinimumValue(0);s->setMaximumValue(1);s->setTrackConfiguration(configuration);
 configuration->setAllowsTickValuesOnly(false);
 expect(s->trackConfiguration()->allowsTickValuesOnly()&&s->trackConfiguration()->ticks().size()==5,"slider copies track configuration and its tick metadata");
 s->setTrackConfiguration(nullptr);s->setValue(.5);const auto before=events;
 s->accessibilityIncrement();expect(near(s->value(),.51f)&&events==before+1,"portable accessibility increment changes one percent and sends valueChanged");
 s->accessibilityDecrement();expect(near(s->value(),.5f)&&events==before+2,"portable accessibility decrement reverses adjustment");
 s->setEnabled(false);s->accessibilityIncrement();expect(near(s->value(),.5f)&&events==before+2,"disabled slider rejects input adjustment");
 registerUIKitViews();auto xml=UINib::fromString("<UISlider minimumValue=\"-2\" maximumValue=\"2\" value=\"1\" continuous=\"false\"/>")->instantiate<UISlider>();
 expect(xml&&xml->minimumValue()==-2&&xml->maximumValue()==2&&xml->value()==1&&!xml->isContinuous(),"UIKit XML registration instantiates usable sliders");
}
void tracking() {
 auto window=new_shared<UIWindow>();window->setFrame({0,0,400,600});auto scroll=new_shared<UIScrollView>();scroll->setFrame(window->bounds());scroll->setContentSize({400,1000});window->addSubview(scroll);
 auto s=new_shared<UISlider>(NXRect(40,200,300,44));s->setValue(.25);scroll->addSubview(s);
 int values=0,ups=0,cancels=0;s->addAction(UIAction("Value",[&]{++values;}),UIControlEvent::valueChanged);
 s->addAction(UIAction("Up",[&]{++ups;}),UIControlEvent::touchUpInside);s->addAction(UIAction("Cancel",[&]{++cancels;}),UIControlEvent::touchCancel);
 double clock=10;
 auto t=new_shared<UITouch>(1,NXKit::NXPoint(),Timer());auto send=[&](UITouchPhase phase,NXKit::NXPoint p){UIGestureRecognizerTestHarness::send(window,t,phase,p,clock);clock+=.1;};
 auto center=[&]{auto r=s->thumbRectForBounds(s->bounds(),s->trackRectForBounds(s->bounds()),s->value());return NXKit::NXPoint(r.midX()+40,r.midY()+200);};
 const auto p=center();send(UITouchPhase::began,p);
 auto lens=UISliderTestHarness::lens(s);
 expect(s->isTracking()&&lens->isLifted()&&near(lens->bounds().width(),37),"accepted thumb contact starts from its visible resting size");
 UISliderTestHarness::advance(s,.25);
 expect(near(lens->bounds().width(),57.35f)&&near(lens->bounds().height(),37.2f),"contact spring reaches native lifted geometry");
 send(UITouchPhase::moved,p+NXKit::NXPoint{60,0});expect(s->value()>.25f&&values==1&&scroll->contentOffset()==NXKit::NXPoint::zero,"slider drag owns input without scrolling its parent");
 send(UITouchPhase::ended,p+NXKit::NXPoint{60,0});expect(s->isTracking()&&!s->isHighlighted()&&near(lens->bounds().width(),57.35f),"finger-up clears highlight without jumping visible knob size");
 const auto fingerUpValue=s->value();
 UISliderTestHarness::release(s,.1);expect(s->isTracking()&&!lens->isLifted()&&s->value()>fingerUpValue&&lens->bounds().width()<55&&lens->bounds().width()>37,"value deceleration overlaps the native lowering transition");
 UISliderTestHarness::release(s,.29);expect(s->isTracking()&&std::abs(lens->bounds().width()-37)<2,"knob is nearly resting before value retirement finishes");
 UISliderTestHarness::release(s,.1);expect(!s->isTracking()&&ups==1&&near(lens->bounds().width(),37),"settled release emits touch-up once with knob already resting");
 LensViewTestHarness::advance(lens,3);expect(lens->interactionTransform().isIdentity(),"released lens elastic response settles");
 const auto before=s->value();t=new_shared<UITouch>(2,NXKit::NXPoint(),Timer());send(UITouchPhase::began,{300,222});send(UITouchPhase::ended,{300,222});expect(s->value()==before&&!s->isTracking(),"far track taps do not jump the thumb");
 s->setContinuous(false);t=new_shared<UITouch>(3,NXKit::NXPoint(),Timer());auto q=center();const auto count=values;
 send(UITouchPhase::began,q+NXKit::NXPoint{5,0});send(UITouchPhase::moved,q+NXKit::NXPoint{45,0});expect(values==count,"noncontinuous tracking defers valueChanged");send(UITouchPhase::ended,q+NXKit::NXPoint{45,0});UISliderTestHarness::release(s);expect(values==count+1,"noncontinuous release sends one changed value");
 t=new_shared<UITouch>(4,NXKit::NXPoint(),Timer());q=center();send(UITouchPhase::began,q);send(UITouchPhase::moved,q-NXKit::NXPoint{40,0});s->setEnabled(false);expect(!s->isTracking()&&!lens->isLifted()&&cancels==1,"disabling a held slider cancels lens and touch once");
 send(UITouchPhase::ended,q);expect(values==count+1,"cancelled noncontinuous gesture emits no deferred valueChanged");
 s->setEnabled(true);s->setSliderStyle(UISliderStyle::thumbless);s->setContinuous(true);
 t=new_shared<UITouch>(5,NXKit::NXPoint(),Timer());send(UITouchPhase::began,{200,222});expect(s->isTracking()&&!lens->isLifted(),"thumbless tracking has no glass knob");send(UITouchPhase::ended,{220,222});UISliderTestHarness::release(s);
 s->setSliderStyle(UISliderStyle::defaultStyle);s->setValue(.5);s->setTrackConfiguration(UISliderTrackConfiguration::configurationWithNumberOfTicks(5));
 t=new_shared<UITouch>(6,NXKit::NXPoint(),Timer());q=center();send(UITouchPhase::began,q);send(UITouchPhase::moved,q+NXKit::NXPoint{43,0});expect(near(s->value()*4,std::round(s->value()*4)),"discrete tick configuration snaps interaction values");send(UITouchPhase::cancelled,q);
 s->setTrackConfiguration(nullptr);s->setValue(.5);t=new_shared<UITouch>(7,NXKit::NXPoint(),Timer());q=center();send(UITouchPhase::began,q);send(UITouchPhase::ended,q);
 expect(!s->isTracking(),"stationary release completes tracking independently of the visual lift tail");
 UISliderTestHarness::release(s,1);t=new_shared<UITouch>(8,NXKit::NXPoint(),Timer());q=center();send(UITouchPhase::began,q);send(UITouchPhase::moved,q+NXKit::NXPoint{30,0});send(UITouchPhase::ended,q+NXKit::NXPoint{30,0});
 expect(s->isTracking(),"moving release retirement retains tracking");s->setMaximumValue(.4f);
 expect(!s->isTracking()&&near(s->value(),.4f),"range change cancels retirement and pins value safely");
}
void nativeOpticalMotion() {
 auto window=new_shared<UIWindow>();window->setFrame({0,0,340,110});
 auto slider=new_shared<UISlider>(NXRect(20,30,300,50));slider->setValue(.25);window->addSubview(slider);
 auto touch=new_shared<UITouch>(901,NXKit::NXPoint(),Timer());
 UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::began,{104,55},1);
 UISliderTestHarness::advance(slider,.6);
 UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::ended,{104,55},2);
 UISliderTestHarness::advance(slider,.0999);
 auto lens=UISliderTestHarness::lens(slider);const auto effect=LensViewTestHarness::backdrop(lens)->effect();
 // Direct CAFilter presentation input: release +.1164s, excluding the
 // measured .0165s commit interval. Native radius=3.087738339.
 expect(std::abs(effect.outputBlurRadius()-3.087738339f)<.03f,"native slider's Gaussian kernel follows optical spring independently of its white-fill fade");
 expect(effect.backdropBlurRadius()==0&&effect.outputBlurRadius()>0,"Gaussian is applied after displacement rather than blurring its input");
 expect(effect.uniform("displacement")->front()==0,"released slider retains zero below-glass displacement while its blur returns");
 expect(effect.outputBlurRadius()>6*LensViewTestHarness::restingOpacity(lens)+.2f,"optical and opaque-fill clocks are not conflated on release");
 UISliderTestHarness::advance(slider,.25);
 expect(near(LensViewTestHarness::backdrop(lens)->effect().outputBlurRadius(),6),"native optical radius finishes before the remaining geometry tail");
}
void loweringSurfaceRegression() {
 for(float scale:{1.f,2.f,3.f})for(bool dark:{false,true}) {
  auto traits=GlassMaterialTestHarness::traits(dark,scale);UITraitCollection::setCurrent(traits);
  auto window=new_shared<ThemeWindow>(traits);window->setFrame({0,0,340,110});
  auto root=new_shared<ThemeRoot>(traits);window->addSubview(root);
  auto slider=new_shared<UISlider>(NXRect(20,30,300,50));root->addSubview(slider);slider->setValue(.25);
  UISliderTestHarness::hideTrack(slider);root->drawAndLayoutTreeIfNeeded();
  auto render=[&] {
   const int w=int(340*scale),h=int(110*scale);
   auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w,h));
   auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SK_ColorWHITE);canvas->scale(scale,scale);
   root->layer()->skiaRender(canvas);SkBitmap pixels;pixels.allocN32Pixels(w,h);surface->readPixels(pixels,0,0);return pixels;
  };
  const auto before=render();auto lens=UISliderTestHarness::lens(slider);
  auto touch=new_shared<UITouch>(902,NXKit::NXPoint(),Timer());
  UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::began,{104,55},1);UISliderTestHarness::advance(slider,.6);
  if(!dark) {
   const auto held=render();const auto bottom=30+lens->frame().maxY();
   const auto near=held.getColor(int(104*scale),int((bottom+10)*scale));
   const auto far=held.getColor(int(104*scale),int((bottom+20)*scale));
   expect(SkColorGetR(near)<254&&SkColorGetR(far)>SkColorGetR(near),"lifted shadow retains its soft tail beyond the former SDF cutoff");
   int biggestStep=0;
   for(int y=int((bottom+6)*scale);y<int((bottom+17)*scale);++y)
    biggestStep=std::max(biggestStep,std::abs(int(SkColorGetR(held.getColor(int(104*scale),y)))-int(SkColorGetR(held.getColor(int(104*scale),y+1)))));
   expect(biggestStep<=6,"lower shadow falloff has no abrupt alpha cliff at any backing scale");
  }
  UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::ended,{104,55},2);UISliderTestHarness::advance(slider,.4);
  auto late=render();
  int low=255,high=0;
  for(int y=int(51*scale);y<=int(59*scale);++y) {
   const auto c=late.getColor(int(104*scale),y);low=std::min(low,int(SkColorGetR(c)));high=std::max(high,int(SkColorGetR(c)));
  }
  expect(high-low<12&&low>235,"late lowering has no persistent refracted dark band through the resting fill");
  UISliderTestHarness::advance(slider,1);LensViewTestHarness::advance(lens,1);
  auto after=render();int maxError=0;
  for(int y=0;y<after.height();++y)for(int x=0;x<after.width();++x) {
   const auto a=before.getColor(x,y),b=after.getColor(x,y);
   maxError=std::max(maxError,std::abs(int(SkColorGetR(a))-int(SkColorGetR(b))));
  }
  expect(maxError<=1,"knob and exterior shadow return exactly to their original resting footprint at every backing scale");
 }
}
void nativeMaterialInputs() {
 // Independent native material trace: release+.24643158s. The inner lane
 // has a small independently measured initial-velocity difference; keep its
 // bounded approximation explicit rather than asserting exact pixel parity.
 const auto effect=Internal::lensMaterial(false,.1194254967f,false,{37.2756f,24.1762f},false,NXSize{57.333333f,37},false);
 const auto value=[&](const char* key,std::size_t index=0){return effect.uniform(key)->at(index);};
 expect(std::abs(value("nativeFaceOpacity")-.1194254967f)<.000001f,"native material face opacity retires with its critical spring");
 expect(std::abs(value("nativeRingShadow",3)-.0179138252f)<.000001f&&std::abs(value("nativeRingShadowOffset")-.955403974f)<.000001f,"native ring opacity and offset return with the resting fill");
 expect(std::abs(value("nativeRefraction")-(-1.26676458f))<.065f,"native inner refraction retires instead of remaining fully lifted");
 expect(value("nativeShadowShape",3)==0,"native material's secondary shadow retires immediately on release");
 const auto rest=Internal::lensMaterial(false,0,false,{37,24},false,NXSize{57.333333f,37},false);
 expect(rest.uniform("nativeRefraction")->front()==0&&rest.uniform("nativeRingShadow")->at(3)==0&&rest.uniform("nativeFaceOpacity")->front()==0,"retired shader has no residual refraction or elevated shadow");
 const auto overshoot=Internal::lensMaterial(false,1.12f,false,{58,38},false,NXSize{57.333333f,37},true);
 expect(overshoot.uniform("nativeFaceOpacity")->front()>1.1f,"material keeps measured spring overshoot while fill opacity remains clamped");
}
void nativeTouchReplay() {
 for(const auto& trace:sliderNativeTouches) {
  auto window=new_shared<UIWindow>();window->setFrame({0,0,500,600});
  auto slider=new_shared<UISlider>(NXRect(80,200,300,50));slider->setValue(.25);window->addSubview(slider);
  std::vector<UIControlEvent> terminal;
  slider->addAction(UIAction("Up",[&]{expect(!slider->isTracking(),"native touch-up observes tracking=false");terminal.push_back(UIControlEvent::touchUpInside);}),UIControlEvent::touchUpInside);
  slider->addAction(UIAction("Changed",[&]{if(!slider->isTracking())terminal.push_back(UIControlEvent::valueChanged);}),UIControlEvent::valueChanged);
  auto touch=new_shared<UITouch>(501,NXKit::NXPoint(),Timer());
  for(const auto& sample:trace.samples) {
   // Apple's phase enum includes stationary; the current portable enum does not.
   const auto phase=sample.phase==3?UITouchPhase::ended:(sample.phase==4?UITouchPhase::cancelled:UITouchPhase(sample.phase));
   UIGestureRecognizerTestHarness::send(window,touch,phase,sample.point,sample.time);
  }
  UISliderTestHarness::release(slider);
  if(std::abs(slider->value()-trace.finalValue)>.0001f){++failures;std::cerr<<"FAIL native touch replay "<<trace.id<<" actual="<<slider->value()<<" expected="<<trace.finalValue<<'\n';}
  expect(terminal==std::vector<UIControlEvent>{UIControlEvent::touchUpInside,UIControlEvent::valueChanged},"native release terminal order: touch-up then final valueChanged");
 }
 std::cout<<"Compared "<<std::size(sliderNativeTouches)<<" independent native touch traces\n";
}
void nativeReleaseTiming(const std::filesystem::path& directory={}) {
 std::ofstream csv;
 if(!directory.empty()){std::filesystem::create_directories(directory);csv.open(directory/"curves.csv");csv<<"trace,fps,time,value,width,nativeWidth,tracking\n";}
 float maximumError=0;
 for(const auto& reference:sliderNativeReleaseTraces)for(int fps:{60,120}) {
  auto window=new_shared<UIWindow>();window->setFrame({0,0,500,600});
  auto slider=new_shared<UISlider>(NXRect(80,200,300,50));slider->setValue(.25);window->addSubview(slider);
  const auto& input=*std::find_if(std::begin(sliderNativeTouches),std::end(sliderNativeTouches),[&](const auto& t){return std::string(t.id)==reference.id;});
  auto touch=new_shared<UITouch>(522,NXKit::NXPoint(),Timer());double last=input.samples.front().time;
  for(const auto& sample:input.samples) {
   UISliderTestHarness::advance(slider,sample.time-last);last=sample.time;
   const auto phase=sample.phase==3?UITouchPhase::ended:UITouchPhase(sample.phase);
   UIGestureRecognizerTestHarness::send(window,touch,phase,sample.point,sample.time);
  }
  double elapsed=0;
  for(const auto& sample:reference.samples) {
   while(elapsed<sample.time) {
    const auto step=std::min(1./fps,sample.time-elapsed);
    UISliderTestHarness::release(slider,step);elapsed+=step;
   }
   const auto error=std::abs(UISliderTestHarness::lens(slider)->bounds().width()-sample.width);
   maximumError=std::max(maximumError,error);
   if(csv)csv<<reference.id<<','<<fps<<','<<sample.time<<','<<slider->value()<<','<<UISliderTestHarness::lens(slider)->bounds().width()<<','<<sample.width<<','<<slider->isTracking()<<'\n';
   expect(error<.35f,"moving knob follows native lowering samples while its value is decelerating");
  }
  UISliderTestHarness::release(slider,.5-elapsed);
  expect(!slider->isTracking()&&near(UISliderTestHarness::lens(slider)->bounds().width(),37),"position retirement and lowering are both complete by the native resting tail");
 }
 std::cout<<"Native moving-release maximum width error="<<maximumError<<"pt at60/120Hz\n";
}
void filledTrackCap() {
 auto traits=GlassMaterialTestHarness::traits(false,3);UITraitCollection::setCurrent(traits);
 auto root=new_shared<ThemeRoot>(traits);auto slider=new_shared<UISlider>(NXRect(20,30,300,50));root->addSubview(slider);
 slider->setTintColor(UIColor(0,136,255)); // Geometry check is independent of the platform's accent.
 slider->setSliderStyle(UISliderStyle::thumbless);slider->setValue(.25);
 auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1020,330));surface->getCanvas()->clear(SK_ColorWHITE);
 surface->getCanvas()->scale(3,3);root->layer()->skiaRender(surface->getCanvas());
 SkBitmap bitmap;bitmap.allocN32Pixels(1020,330);surface->readPixels(bitmap,0,0);
 // Native reference: the upper-right corner reveals the unfilled track while
 // the same column at the vertical center remains filled.
 const auto corner=bitmap.getColor(283,155),center=bitmap.getColor(283,165);
 if(!(SkColorGetR(corner)>200&&SkColorGetR(center)<32&&SkColorGetB(center)>200)) {
  auto layer=UISliderTestHarness::track(slider)->layer();
  std::cerr<<"Cap pixels corner="<<corner<<" center="<<center<<" radius="<<layer->cornerRadius()<<" presented="<<layer->presentationOrSelf()->cornerRadius()<<" bounds="<<layer->bounds().width()<<","<<layer->bounds().height()<<'\n';
 }
 expect(SkColorGetR(corner)>200&&SkColorGetR(center)<32&&SkColorGetB(center)>200,"filled track has a capsule end, including thumbless style");
}
void nativeEndpoints() {
 float maximumError=0;
 for(const auto& c:sliderNativeEndpoints) {
  const std::string kind=c.variant;
  auto traits=GlassMaterialTestHarness::traits(c.dark,3);UITraitCollection::setCurrent(traits);
  auto root=new_shared<ThemeRoot>(traits);auto slider=new_shared<UISlider>(NXRect(0,0,c.width,34));root->addSubview(slider);
  slider->setMinimumValue(c.minimum);slider->setMaximumValue(c.maximum);
  if(kind=="thumbless")slider->setSliderStyle(UISliderStyle::thumbless);
  if(kind=="custom-thumb")slider->setThumbImage(image(30,40),UIControlState::normal);
  if(kind=="images"){slider->setMinimumValueImage(image(18,20));slider->setMaximumValueImage(image(26,24));}
  if(kind=="disabled")slider->setEnabled(false);
  if(kind=="ticks"||kind.find("neutral")!=std::string::npos) {
   auto cfg=UISliderTrackConfiguration::configurationWithNumberOfTicks(5);cfg->setAllowsTickValuesOnly(false);
   if(kind.find("neutral")!=std::string::npos)cfg->setNeutralValue(.5);
   slider->setTrackConfiguration(cfg);
  }
  slider->setValue(c.value);root->drawAndLayoutTreeIfNeeded();
  const auto track=slider->trackRectForBounds(slider->bounds()),fill=UISliderTestHarness::track(slider)->frame();
  const auto error=std::max({std::abs(track.minX()-c.trackX),std::abs(track.width()-c.trackWidth),std::abs(fill.minX()-c.fillX),std::abs(fill.width()-c.fillWidth)});
  maximumError=std::max(maximumError,error);
  if(error>.001f){++failures;std::cerr<<"FAIL: native endpoint "<<kind<<" value="<<c.value<<" error="<<error<<"pt\n";}
 }
 std::cout<<"Compared "<<std::size(sliderNativeEndpoints)<<" native endpoint cases max error="<<maximumError<<"pt\n";
}
void endpointContinuity() {
 for(float width:{0.f,30.f,180.f,300.f})for(int kind:{0,1,2}) {
  auto slider=new_shared<UISlider>(NXRect(0,0,width,50));
  if(kind==1)slider->setSliderStyle(UISliderStyle::thumbless);
  if(kind==2)slider->setThumbImage(image(30,40),UIControlState::normal);
  const auto fillAt=[&](float value){slider->setValue(value);return UISliderTestHarness::track(slider)->frame();};
  for(float edge:{0.f,.02f,.98f,1.f}) {
   const auto a=fillAt(std::max(0.f,edge-.00001f)),b=fillAt(std::min(1.f,edge+.00001f));
   expect(std::abs(a.width()-b.width())<.1f,"filled track is continuous at endpoints and the native two-percent taper boundaries");
  }
  expect(near(fillAt(0).width(),0)&&near(fillAt(1).width(),width),"track still reaches exactly empty and full");
  float previous=0;
  if(width>=37)for(int i=0;i<=1000;++i) {
   const auto current=fillAt(float(i)/1000).width();
   expect(current>=previous-.0001f,"endpoint taper preserves monotone fill across the usable track");previous=current;
  }
  auto configuration=UISliderTrackConfiguration::configurationWithNumberOfTicks(5);configuration->setAllowsTickValuesOnly(false);configuration->setNeutralValue(.5);slider->setTrackConfiguration(configuration);
  for(float edge:{0.f,1.f}) {
   const auto a=fillAt(std::max(0.f,edge-.00001f)),b=fillAt(std::min(1.f,edge+.00001f));
   expect(std::abs(a.minX()-b.minX())<.1f&&std::abs(a.width()-b.width())<.1f,"configured track preserves endpoint continuity");
  }
 }
}
void renderEndpoints(const std::filesystem::path& directory) {
 std::filesystem::create_directories(directory);
 std::ofstream csv(directory/"samples.csv");csv<<"theme,variant,fraction,value,trackX,trackWidth,fillX,fillWidth,fillAlpha,thumbX,thumbWidth\n";
 const float samples[]={0,.0001f,.001f,.005f,.01f,.02f,.04f,.06f,.08f,.1f,.25f,.499f,.5f,.501f,.75f,.9f,.92f,.94f,.96f,.98f,.99f,.995f,.999f,.9999f,1};
 for(bool dark:{false,true})for(const std::string kind:{"default","thumbless","thumb-image","ticks","neutral","both-images","range","short"}) {
  auto traits=GlassMaterialTestHarness::traits(dark,3);UITraitCollection::setCurrent(traits);
  auto root=new_shared<ThemeRoot>(traits);auto slider=new_shared<UISlider>(NXRect(20,30,kind=="short"?180.f:300.f,50));root->addSubview(slider);
  if(kind=="thumbless")slider->setSliderStyle(UISliderStyle::thumbless);
  if(kind=="thumb-image")slider->setThumbImage(image(30,40),UIControlState::normal);
  if(kind=="ticks"||kind=="neutral") {
   auto cfg=UISliderTrackConfiguration::configurationWithNumberOfTicks(5);cfg->setAllowsTickValuesOnly(false);if(kind=="neutral")cfg->setNeutralValue(.5);slider->setTrackConfiguration(cfg);
  }
  if(kind=="both-images") {slider->setMinimumValueImage(image(18,20));slider->setMaximumValueImage(image(26,24));}
  if(kind=="range") {slider->setMinimumValue(-10);slider->setMaximumValue(10);}
  for(std::size_t i=0;i<std::size(samples);++i) {
   const auto f=samples[i];slider->setValue(slider->minimumValue()+f*(slider->maximumValue()-slider->minimumValue()));root->drawAndLayoutTreeIfNeeded();
   const auto track=slider->trackRectForBounds(slider->bounds()),fill=UISliderTestHarness::track(slider)->frame(),thumb=slider->thumbRectForBounds(slider->bounds(),track,slider->value());
   csv<<(dark?"dark":"light")<<','<<kind<<','<<f<<','<<slider->value()<<','<<track.minX()<<','<<track.width()<<','<<fill.minX()<<','<<fill.width()<<','<<UISliderTestHarness::track(slider)->alpha()<<','<<thumb.minX()<<','<<thumb.width()<<'\n';
   const auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1020,330));auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SK_ColorWHITE);canvas->scale(3,3);root->layer()->skiaRender(canvas);
   SkBitmap bitmap;bitmap.allocN32Pixels(1020,330);surface->readPixels(bitmap,0,0);
   const auto path=directory/((dark?"dark-":"light-")+kind+"-"+std::to_string(i)+".png");SkFILEWStream file(path.c_str());
   if(!SkPngEncoder::Encode(&file,bitmap.pixmap(),{}))throw std::runtime_error("Slider endpoint capture failed");
  }
 }
 std::ofstream metadata(directory/"metadata.json");metadata<<"{\"displayScale\":3,\"pixels\":[1020,330],\"logicalSize\":[340,110],\"samplesPerVariant\":25,\"themes\":2,\"variants\":8}\n";
}
void renderMotion(const std::filesystem::path& directory) {
 std::filesystem::create_directories(directory);
 for(bool dark:{false,true})for(double hold:{.7,.08}) {
  const auto name=std::string(dark?"dark-":"light-")+(hold>.1?"hold":"tap");
  const auto folder=directory/name;std::filesystem::create_directories(folder);
  auto traits=GlassMaterialTestHarness::traits(dark,3);UITraitCollection::setCurrent(traits);
  auto window=new_shared<ThemeWindow>(traits);window->setFrame({0,0,340,110});
  auto root=new_shared<ThemeRoot>(traits);window->addSubview(root);
  auto slider=new_shared<UISlider>(NXRect(20,30,300,50));root->addSubview(slider);slider->setValue(.25);
  root->drawAndLayoutTreeIfNeeded();auto lens=UISliderTestHarness::lens(slider);
  const auto checkScale=[&] {
   if(!slider->traitCollection()||slider->traitCollection()->displayScale()!=3||
      lens->layer()->contentsScale()!=3||LensViewTestHarness::opticalScale(lens)!=3||
      LensViewTestHarness::backdrop(lens)->layer()->contentsScale()!=3)
    throw std::runtime_error("Motion capture requires 3x traits and optical backing throughout the hierarchy");
  };
  checkScale();
  std::ofstream metadata(folder/"metadata.json");
  metadata<<"{\"backend\":\"Skia Raster\",\"pixelWidth\":1020,\"pixelHeight\":330,\"logicalWidth\":340,\"logicalHeight\":110,\"displayScale\":3,\"lensContentsScale\":3,\"opticalContentsScale\":3,\"warpContentsScale\":3,\"dark\":"<<(dark?"true":"false")<<"}\n";
  auto touch=new_shared<UITouch>(991,NXKit::NXPoint(104,55),Timer());
  std::unique_ptr<Internal::LayerAnimationTimeline> timeline;
  std::ofstream csv(folder/"frames.csv");csv<<"t,modelW,modelH,presentW,presentH,centerX,centerY,lift,restingAlpha,scaleX,scaleY,driftX,driftY\n";
  const int releaseFrame=int(std::round((.05+hold)*120));
  const double release=releaseFrame/120.;
  for(int frame=0;frame<=int((release+.65)*120);++frame) {
   const double time=frame/120.;
   if(frame==6) {
    timeline=std::make_unique<Internal::LayerAnimationTimeline>(root->layer(),.35);
    UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::began,{104,55},1+time);timeline->capture(root->layer());
   }
   if(frame==releaseFrame) {
    timeline.reset();timeline=std::make_unique<Internal::LayerAnimationTimeline>(root->layer(),.35);
    UIGestureRecognizerTestHarness::send(window,touch,UITouchPhase::ended,{104,55},1+time);timeline->capture(root->layer());
   }
   if(timeline)timeline->seek(std::clamp((time-(time>=release?release:.05))/.35,0.,1.));
   if(time>.05&&frame!=releaseFrame){UISliderTestHarness::advance(slider,1./120);LensViewTestHarness::advance(lens,1./120);}
   if(time>release)UISliderTestHarness::advanceValue(slider,1./120);
   root->drawAndLayoutTreeIfNeeded();
   checkScale();
   const auto layer=lens->layer()->presentationOrSelf();const auto pose=lens->interactionTransform();
   const auto size=layer->bounds().size;
   csv<<time<<','<<lens->bounds().width()<<','<<lens->bounds().height()<<','<<size.width<<','<<size.height<<','<<layer->position().x<<','<<layer->position().y<<','<<LensViewTestHarness::progress(lens)<<','<<LensViewTestHarness::restingOpacity(lens)<<','<<pose.m11<<','<<pose.m22<<','<<pose.tX<<','<<pose.tY<<'\n';
   if(frame%2)continue;
   auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1020,330));auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SK_ColorWHITE);canvas->scale(3,3);root->layer()->skiaRender(canvas);
   SkBitmap pixels;pixels.allocN32Pixels(1020,330);surface->readPixels(pixels,0,0);
   std::ofstream out(folder/(std::to_string(int(std::lround(time*1000)))+".ppm"),std::ios::binary);out<<"P6\n1020 330\n255\n";
   for(int y=0;y<330;++y)for(int x=0;x<1020;++x){const auto c=pixels.getColor(x,y);const char rgb[]={char(SkColorGetR(c)),char(SkColorGetG(c)),char(SkColorGetB(c))};out.write(rgb,3);}
  }
  slider->cancelTracking();
 }
}
void render(const std::filesystem::path& directory) {
 std::filesystem::create_directories(directory);
 for(bool dark:{false,true}) {
  auto traits=GlassMaterialTestHarness::traits(dark,3);UITraitCollection::setCurrent(traits);
  for(const auto& c:sliderNativeGeometry) {
   const std::string id=c.id;if(id.starts_with("plain-"))continue;
   auto root=new_shared<ThemeRoot>(traits);auto s=new_shared<UISlider>(NXRect(20,30,300,50));root->addSubview(s);
   s->setMinimumValue(c.minimum);s->setMaximumValue(c.maximum);s->setValue(c.value);s->setSliderStyle(UISliderStyle(c.style));
   if(id.find("thumb-image")!=std::string::npos)s->setThumbImage(image(30,40),UIControlState::normal);
   if(id=="min-image"||id=="both-images")s->setMinimumValueImage(image(18,20));
   if(id=="max-image"||id=="both-images")s->setMaximumValueImage(image(26,24));
   if(id.find("highlighted")!=std::string::npos)s->setHighlighted(true);
   if(id=="selected")s->setSelected(true);s->setEnabled(c.enabled);
   if(id=="ticks")s->setTrackConfiguration(UISliderTrackConfiguration::configurationWithNumberOfTicks(5));
   if(id=="tinted"){s->setThumbTintColor(UIColor::systemPink);s->setMinimumTrackTintColor(UIColor::systemGreen);s->setMaximumTrackTintColor(UIColor::systemOrange);}
   root->drawAndLayoutTreeIfNeeded();
   auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1020,330));surface->getCanvas()->clear(dark?SK_ColorBLACK:SK_ColorWHITE);surface->getCanvas()->scale(3,3);root->layer()->skiaRender(surface->getCanvas());
   SkBitmap pixels;pixels.allocN32Pixels(1020,330);surface->readPixels(pixels,0,0);
   std::ofstream out(directory/((dark?"dark-":"light-")+id+".ppm"),std::ios::binary);out<<"P6\n1020 330\n255\n";
   for(int y=0;y<330;++y)for(int x=0;x<1020;++x){auto c=pixels.getColor(x,y);const char rgb[]={char(SkColorGetR(c)),char(SkColorGetG(c)),char(SkColorGetB(c))};out.write(rgb,3);}
  }
 }
}

}
int main(int argc,char** argv){SkiaCtx::_main=std::make_shared<Internal::RasterTestContext>();if(argc==3&&std::string(argv[1])=="--release-timing"){nativeReleaseTiming(argv[2]);return failures?1:0;}if(argc==3&&std::string(argv[1])=="--endpoints"){renderEndpoints(argv[2]);return 0;}if(argc==3&&std::string(argv[1])=="--motion"){renderMotion(argv[2]);return 0;}if(argc==3&&std::string(argv[1])=="--render"){render(argv[2]);return 0;}nativeGeometry();nativeLiftMotion();valuesAndImages();tracking();nativeOpticalMotion();nativeMaterialInputs();loweringSurfaceRegression();nativeEndpoints();endpointContinuity();nativeTouchReplay();nativeReleaseTiming();filledTrackCap();if(!failures)std::cout<<"UISlider tests passed\n";return failures?1:0;}
