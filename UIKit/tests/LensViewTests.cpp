#include "CABackdropEffectLayer.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <BackdropEffectView.h>
#include <LensView.h>
#include <cmath>
#include <iostream>
#include <limits>
#include "NativeTabShadowData.hpp"

using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
  static auto traits(UIUserInterfaceStyle style, NXFloat scale) {
    auto value = std::make_shared<UITraitCollection>();
    value->_userInterfaceStyle = style;
    value->_displayScale = scale;
    return value;
  }
};
class LensViewTestHarness {
public:
  static void advance(const std::shared_ptr<LensView> &lens, double seconds) {
    lens->advance(seconds);
  }
  static float progress(const std::shared_ptr<LensView> &lens) {
    return lens->_progress;
  }
  static void elastic(const std::shared_ptr<LensView>& lens,NXAffineTransform value) {
    lens->applyInteractionTransform(value);
  }
  static auto optics(const std::shared_ptr<LensView> &lens) {
    return lens->_optics;
  }
  static auto backdrop(const std::shared_ptr<LensView> &lens) {return lens->_warpBackdrop;}
};
} // namespace NXKit
namespace {
class Root : public UIView {
public:
  Root(UIUserInterfaceStyle style=UIUserInterfaceStyle::light,NXFloat scale=2) {
    _traitCollection =
        GlassMaterialTestHarness::traits(style, scale);
  }
  void changeTraits(NXFloat scale = 3) {
    const auto previous = _traitCollection;
    _traitCollection =
        GlassMaterialTestHarness::traits(UIUserInterfaceStyle::dark, scale);
    traitCollectionDidChange(previous);
  }
};
class DrawnSource final : public UIView {
public:
  explicit DrawnSource(NXRect frame): UIView(frame) {}
  int draws = 0;
  bool blue = true;
  void draw() override {
    ++draws;
    setBackgroundColor(blue ? UIColor::blue : UIColor::red);
  }
  void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) override {
    UIView::traitCollectionDidChange(std::move(previous));
    setNeedsDisplay();
  }
};
int failures = 0;
void expect(bool value, const char *reason) {
  if (!value) {
    ++failures;
    std::cerr << "FAIL: " << reason << '\n';
  }
}
SkColor pixel(const std::shared_ptr<UIView> &view, int x, int y) {
  auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(240, 180));
  surface->getCanvas()->clear(SkColorSetRGB(128, 128, 128));
  view->layer()->skiaRender(surface->getCanvas());
  SkBitmap image;
  image.allocN32Pixels(240, 180);
  surface->readPixels(image, 0, 0);
  return image.getColor(x, y);
}
void testNativeTabRetirement() {
 float maximum=0;
 for(bool dark:{false,true})for(int scale:{1,2,3}) {
  auto root=new_shared<Root>(dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light,scale);
  auto lens=new_shared<LensView>(LensView::Style::large);lens->setFrame({40,40,120.5f,70});root->addSubview(lens);
  lens->setLifted(true);lens->setLifted(false,true);lens->setFrame({48,48,104.5f,54});
  double elapsed=0;
  for(const auto& native:nativeTabShadowSamples)if(native.dark==dark) {
   LensViewTestHarness::advance(lens,native.time-elapsed);elapsed=native.time;
   const auto& material=LensViewTestHarness::optics(lens)->effect();
   const auto get=[&](const char* key,int index=0){return material.uniform(key)->at(index);};
   const auto error=std::max({std::abs(get("nativeFaceOpacity")-native.face),std::abs(get("nativeRefraction")-native.refraction),std::abs(get("nativeRefraction",1)-native.height),std::abs(get("nativeShadowResponse")-native.shadow),std::abs(get("nativeShadowShape")-native.radius),std::abs(get("nativeRingShadow",3)-native.ring)});
   maximum=std::max(maximum,error);
   expect(error<.006f,"automatic large lens retires native shadow/refraction/face inputs from lifted reference size while shrinking");
   expect(get("nativeShadowShape",3)==0,"secondary shader shadow switches off on release for every lens caller");
  }
  LensViewTestHarness::advance(lens,.480001-elapsed);
  expect(LensViewTestHarness::progress(lens)==0&&LensViewTestHarness::optics(lens)->isHidden(),"retired automatic lens removes its optical passes exactly");
 }
 std::cout<<"Native tab material retirement maximum parameter error="<<maximum<<'\n';
}
void testRestingConfiguration() {
 auto background=new_shared<UIView>();background->setBackgroundColor(UIColor::white);
 auto lens=new_shared<LensView>(background,LensView::Style::small);lens->setFrame({40,40,60,40});
 expect(lens->hasCustomRestingBackground()&&lens->restingBackgroundView()==background&&background->superview().lock()!=lens,"custom resting background is owned by the shared fading host");
 lens->setRestingBackgroundColor(UIColor::blue);expect(lens->restingBackgroundColor()==UIColor::blue&&background->backgroundColor()==UIColor::white,"native color override does not rewrite a custom resting view");
 auto shadow=std::make_shared<UIShadowProperties>();
 expect(shadow->opacity()==0&&shadow->radius()==0&&shadow->offset()==NXSize(0,0)&&shadow->color()==UIColor::black,"native shadow value defaults");
 shadow->setRadius(5);shadow->setOffset({1,2});shadow->setOpacity(.2f);lens->setRestingShadowProperties(shadow);
 const auto host=background->superview().lock();
 expect(host->layer()->shadowRadius()==5&&host->layer()->shadowOffset()==NXPoint(1,2)&&host->layer()->shadowOpacity()==.2f&&!host->clipsToBounds(),"resting shadow uses unclipped host behind the fill");
 const auto copy=UIShadowProperties::propertiesFromView(host);copy->scaleBy(2);
 expect(copy->radius()==10&&copy->offset()==NXSize(2,4)&&shadow->radius()==5,"shadow scaling preserves original value object");
 expect(copy->updatedShadowWithIntensity(.5)->opacity()==.1f&&copy->updatedShadowWithIntensity(2)->opacity()==.2f&&copy->updatedShadowWithIntensity(-1)->opacity()==0,"native shadow intensity clamps its multiplier");
 lens->setRestingBorderColor(UIColor::red);expect(lens->layer()->borderWidth()==3,"resting border uses native three-point width");
 lens->setLifted(true);expect(lens->layer()->borderWidth()==0&&host->alpha()==0,"lifting retires resting border and its shadow host");
 lens->setLifted(false,true);LensViewTestHarness::advance(lens,.1);
 expect(host->alpha()>0&&host->alpha()<1,"shared resting shadow returns with its fill during lowering");
 lens->setRestingShadowProperties(nullptr);expect(host->layer()->shadowOpacity()==0&&host->layer()->shadowRadius()==0&&host->layer()->shadowOffset()==NXPoint::zero,"nil shadow properties clear prior configuration");
 lens->setRestingBorderColor(std::nullopt);expect(lens->layer()->borderWidth()==0,"clearing border removes native decoration");
}
void testPublicLensLifecycleAndModes() {
 auto root=new_shared<Root>();root->setFrame({0,0,240,180});
 auto source=new_shared<UIView>(NXRect(20,20,180,100));source->setBackgroundColor(UIColor::blue);root->addSubview(source);
 auto lens=new_shared<LensView>();lens->setFrame({40,40,120,64});root->addSubview(lens);
 expect(!lens->warpsContentBelow()&&lens->liftedContentMode()==LensView::LiftedContentMode::preserveSource,"fresh public lens matches native source and backdrop policy defaults");
 lens->setLiftedContentView(source);
 expect(SkColorGetB(pixel(root,30,30))==255,"preserveSource leaves the external source's ordinary output visible");
 lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);
 expect(pixel(root,30,30)==SkColorSetRGB(128,128,128)&&!source->isHidden()&&source->alpha()==1,"hideSource owns ordinary rendering without altering caller visibility");
 lens->setLiftedContentMode(LensView::LiftedContentMode::preserveSource);
 expect(SkColorGetB(pixel(root,30,30))==255,"changing content mode releases ordinary source ownership immediately");
 int alongside=0,completed=0;
 const auto completion=[&](bool a,bool b){expect(a&&!b,"successful lens group uses the observed native completion pair");++completed;};
 lens->setLifted(true,false,[&]{++alongside;},completion);
 expect(alongside==1&&completed==1&&lens->isLifted(),"nonanimated public lift runs alongside and completion synchronously");
 lens->setLifted(true,true,[&]{++alongside;},completion);
 expect(alongside==2&&completed==1,"same-state native lift runs alongside without completion");
 lens->setLifted(false,true,[&]{++alongside;},completion);LensViewTestHarness::advance(lens,.08);
 lens->setLifted(true,true,[&]{++alongside;},completion);LensViewTestHarness::advance(lens,2);
 expect(alongside==4&&completed==3&&lens->isLifted(),"interrupted lift completions collect until the current group settles");
 expect(lens->applyXMLAttribute("style","large")&&lens->applyXMLAttribute("liftedContentMode","hideSource")&&lens->applyXMLAttribute("restingShadowRadius","5")&&lens->applyXMLAttribute("restingShadowOpacity",".14")&&!lens->applyXMLAttribute("restingShadowRadius","-1")&&!lens->applyXMLAttribute("liftedContentMode","invalid"),"XML exposes usable lens configuration and rejects invalid input");
}
void testStateAndRendering() {
  auto lens = new_shared<LensView>(LensView::Style::selection);
  lens->setFrame({40, 40, 120, 64});
  const auto frame = lens->frame();
  expect(!lens->isLifted() && !lens->restingBackgroundColor(),
         "default resting lens is clear");
  expect(pixel(lens, 100, 70) == SkColorSetRGB(128, 128, 128),
         "clear rest leaves backdrop unchanged");
  lens->setRestingBackgroundColor(UIColor::black.withAlphaComponent(.1f));
  const auto filled = pixel(lens, 100, 70);
  expect(SkColorGetR(filled) >= 114 && SkColorGetR(filled) <= 116,
         "caller resting fill is rendered");
  auto ink = new_shared<UIView>(NXRect(50, 25, 12, 12));
  ink->setBackgroundColor(UIColor::blue);
  lens->contentView()->addSubview(ink);
  expect(SkColorGetB(pixel(lens, 95, 70)) > 240,
         "selected content appears above resting fill");
  lens->restingBackgroundView()->setAlpha(.6f);
  lens->setLifted(true);
  expect(lens->isLifted() && lens->frame() == frame,
         "lift changes state without inflating caller geometry");
  expect(lens->restingBackgroundView()->alpha() == .6f,
         "lifting preserves caller resting-view opacity");
  expect(SkColorGetR(pixel(lens, 120, 70)) == 128,
         "lifted interior has no regular frosted veil");
  expect(SkColorGetR(pixel(lens, 42, 72)) > 115,
         "exterior contact contour is not refracted into a second dark ring");
  expect(SkColorGetB(pixel(lens, 95, 70)) > 240,
         "selected content remains visible above the lifted optical pass");
  lens->setBounds({7.25f, -4.5f, 120, 64});
  expect(SkColorGetB(pixel(lens, 95, 70)) > 240 &&
             lens->contentView()->frame().origin == NXPoint(7.25f, -4.5f),
         "nonzero fractional bounds origin preserves the displayed selected "
         "content");
  auto outside = new_shared<UIView>(NXRect(-40, -40, 300, 180));
  outside->setBackgroundColor(UIColor::red);
  lens->contentView()->addSubview(outside);
  expect(pixel(lens, 41, 41) == SkColorSetRGB(128, 128, 128),
         "selected content is clipped to capsule corners");
  lens->setLifted(false);
  expect(!lens->isLifted() && LensViewTestHarness::progress(lens) == 0,
         "immediate release restores resting surface");
}
void testSelectedContentKeepsInteriorGeometryAndColor() {
  for (const auto style :
       {LensView::Style::regular, LensView::Style::selection}) {
    auto lens = new_shared<LensView>(style);
    lens->setFrame({40, 40, 120, 64});
    lens->setRestingBackgroundColor(UIColor::black.withAlphaComponent(.1f));
    auto mark = new_shared<UIView>(NXRect(50, 24, 12, 16));
    mark->setBackgroundColor(UIColor(17, 94, 231));
    lens->contentView()->addSubview(mark);
    const auto source = SkColorSetRGB(17, 94, 231);
    expect(pixel(lens, 92, 65) == source,
           "resting selected content retains its source ink");
    lens->setLifted(true, true);
    LensViewTestHarness::advance(lens, .1);
    expect(pixel(lens, 92, 65) == source,
           "lifting preserves selected center coordinates and ink");
    LensViewTestHarness::advance(lens, 2);
    expect(
        pixel(lens, 92, 65) == source,
        "lifted selected center retains source coordinates and solid color");
    lens->setLifted(false, true);
    LensViewTestHarness::advance(lens, .1);
    expect(pixel(lens, 92, 65) == source,
           "release preserves the selected-content portal ink");
  }
}
void testBelowGlassBlurPolicy() {
  auto lens=new_shared<LensView>();lens->setFrame({0,0,57.35f,37.2f});
  lens->setLifted(true);lens->setLifted(false,true);
  LensViewTestHarness::advance(lens,.1);
  auto backdrop=LensViewTestHarness::backdrop(lens);
  expect(!backdrop->isHidden()&&backdrop->effect().outputBlurRadius()>0,
         "small lens has a Gaussian backdrop lane while lowering toward its opaque resting surface");
  auto parent=new_shared<Root>();parent->addSubview(lens);pixel(lens,20,20);
  lens->setWarpsContentBelow(false);
  expect(!backdrop->isHidden()&&backdrop->effect().outputBlurRadius()>0&&
         backdrop->effect().uniform("displacement")->front()==0&&!LensViewTestHarness::optics(lens)->isHidden(),
         "slider backdrop policy disables sustained displacement while preserving Gaussian blur and ClearGlass optics");
  lens->setWarpsContentBelow(true);
  expect(!backdrop->isHidden(),"restoring backdrop policy invalidates active lens material immediately");
}
void testVibrantForeground() {
  struct Case { bool dark;UIColor source,background,expected; };
  // Independent public UITabBar template probes; the input is the actual
  // material pixel beneath the template, not the original screen background.
  const Case cases[]={
    {true,UIColor(0,145,255),UIColor(99,99,98),UIColor(58,203,255)},
    {true,UIColor(0,145,255),UIColor(0,85,210),UIColor(97,242,255)},
    {true,UIColor(142,142,147),UIColor(0,159,55),UIColor(144,221,176)},
    {false,UIColor(0,136,255),UIColor(169,169,169),UIColor(0,112,224)},
    {false,UIColor(255,56,60),UIColor(21,223,133),UIColor(206,38,32)},
    {false,UIColor(52,199,89),UIColor(169,169,169),UIColor(23,162,58)}
  };
  const auto near=[](SkColor actual,UIColor expected,int tolerance) {
    return std::abs(int(SkColorGetR(actual))-expected.r())<=tolerance &&
           std::abs(int(SkColorGetG(actual))-expected.g())<=tolerance &&
           std::abs(int(SkColorGetB(actual))-expected.b())<=tolerance;
  };
  for(const auto& value:cases) {
    auto root=new_shared<Root>();root->setFrame({0,0,240,180});
    if(value.dark)root->changeTraits(2);
    root->setBackgroundColor(value.background);
    auto lens=new_shared<LensView>(LensView::Style::selection);lens->setFrame({40,40,120,64});root->addSubview(lens);
    auto ink=new_shared<UIView>(NXRect(40,20,40,28));ink->setBackgroundColor(value.source);lens->contentView()->addSubview(ink);
    expect(!lens->isContentVibrancyEnabled()&&near(pixel(root,100,70),value.source,0),"literal source RGB stays exact until vibrancy is enabled");
    lens->setContentVibrancyEnabled(true);
    expect(near(pixel(root,100,70),value.expected,4),"vibrant lens source matches the independently captured native template color");
    ink->setAlpha(.5f);const auto mixed=value.background.interpolationTo(value.expected,.5f);
    expect(near(pixel(root,100,70),mixed,4),"translucent vibrant source composites its coverage exactly once");
    ink->setAlpha(1);
    lens->setContentVibrancyEnabled(false);
    expect(near(pixel(root,100,70),value.source,0),"disabling vibrancy restores original source bytes without another mutation");

    lens->removeFromSuperview();
    UIGlassEffect glass(UIGlassEffect::Style::clear);glass.setFrostRadius(0);glass.setRefraction(0);glass.setSaturation(1);glass.setContrast(1);glass.setLuminosity(0);glass.setTintOpacity(0);glass.setEdgeStrength(0);glass.setSpecularStrength(0);
    glass.setContentVibrancyEnabled(true);
    auto material=new_shared<BackdropEffectView>(glass);material->setFrame({40,40,120,64});root->addSubview(material);
    auto foreground=new_shared<UIView>(NXRect(40,20,40,28));foreground->setBackgroundColor(value.source);material->addSubview(foreground);
    expect(near(pixel(root,100,70),value.expected,4),"ordinary glass children use the same color transfer without altering the material itself");
    expect(material->applyXMLAttribute("contentVibrancyEnabled","false") && !material->glassEffect()->isContentVibrancyEnabled() && !material->applyXMLAttribute("contentVibrancyEnabled","invalid"),"glass XML validates the copied foreground blend option");
    expect(near(pixel(root,100,70),value.source,0),"ordinary glass source-over is restored when the copied effect is replaced");
    expect(lens->applyXMLAttribute("contentVibrancyEnabled","true") && lens->isContentVibrancyEnabled() && !lens->applyXMLAttribute("contentVibrancyEnabled","invalid"),"lens XML validates the independent source blend option");
  }
}
void testOptionalMagnification() {
  auto lens=new_shared<LensView>();
  expect(lens->liftedContentMagnification()==1, "standalone lens defaults to native unit source magnification");
  lens->setLiftedContentMagnification(1.16f);
  expect(lens->liftedContentMagnification()==1.16f, "caller can opt into lifted source enlargement");
  for(auto invalid:{.99f,std::numeric_limits<NXFloat>::infinity(),std::numeric_limits<NXFloat>::quiet_NaN()}) {
    bool rejected=false;try { lens->setLiftedContentMagnification(invalid); } catch(const std::invalid_argument&) { rejected=true; }
    expect(rejected&&lens->liftedContentMagnification()==1.16f,"invalid magnification preserves usable source state");
  }
  expect(lens->applyXMLAttribute("liftedContentMagnification","1.2") &&
         !lens->applyXMLAttribute("liftedContentMagnification","0.8") &&
         !lens->applyXMLAttribute("liftedContentMagnification","nan"), "XML validates optional magnification");
}
void testAnimationAndLayout() {
  auto a = new_shared<LensView>();
  auto b = new_shared<LensView>();
  a->setFrame({5, 10, 80, 48});
  b->setFrame(a->frame());
  a->setLifted(true, true);
  b->setLifted(true, true);
  for (int i = 0; i < 12; i++)
    LensViewTestHarness::advance(a, 1.0 / 60);
  LensViewTestHarness::advance(b, .2);
  expect(std::abs(LensViewTestHarness::progress(a) -
                  LensViewTestHarness::progress(b)) < .0001,
         "lift transition is independent of frame rate");
  a->setLifted(false, true);
  LensViewTestHarness::advance(a, .1);
  a->setLifted(true, true);
  LensViewTestHarness::advance(a, 2);
  expect(a->isLifted() && LensViewTestHarness::progress(a) == 1,
         "retargeted transition reaches final lift state");
  a->setBounds({0, 0, 180, 64});
  a->layoutSubviews();
  expect(a->contentView()->frame().size == NXSize(180, 64) &&
             a->restingBackgroundView()->frame().size == NXSize(180, 64),
         "content and resting view follow caller size changes");
  a->setStyle(LensView::Style::selection);
  expect(a->style() == LensView::Style::selection,
         "variant is explicit and independent of size");
  expect(!a->applyXMLAttribute("style", "invalid") &&
             a->applyXMLAttribute("style", "regular") &&
             a->applyXMLAttribute("isLifted", "false") && !a->isLifted(),
         "XML validates style and lifted state");
  expect(a->applyXMLAttribute("restingBackgroundColor", "#10203080") &&
             !a->applyXMLAttribute("restingBackgroundColor", "nonsense"),
         "XML validates resting color");
  bool rejected = false;
  try {
    a->setStyle(static_cast<LensView::Style>(99));
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  expect(rejected && a->style() == LensView::Style::regular,
         "invalid variants leave the current style intact");
  a->setLifted(true);
  auto root = new_shared<Root>();
  root->addSubview(a);
  auto optics = LensViewTestHarness::optics(a);
  expect(optics->layer()->contentsScale() == 2,
         "lens optical backing inherits Retina display scale");
  root->changeTraits();
  expect(optics->layer()->contentsScale() == 3 &&
             a->contentView()->traitCollection()->userInterfaceStyle() ==
                 UIUserInterfaceStyle::dark,
         "resizing display scale and appearance reach optics and selected "
         "content");
  const auto darkFace = optics->effect().uniform("nativeFaceBlack");
  expect(darkFace && darkFace->front() > 0,
         "regular lens uses the captured dark-appearance face transfer");
  a->setBounds({0,0,160,80});
  const auto refraction=optics->effect().uniform("nativeRefraction");
  expect(refraction && std::abs(refraction->at(0)+24)<.001f &&
         std::abs(refraction->at(1)-14.4f)<.001f,
         "resizing a lifted lens refreshes size-dependent native material distances");
  root->changeTraits(1);
  expect(optics->layer()->contentsScale() == 1 &&
             a->subviews().back()->layer()->contentsScale() == 1,
         "one-point display scale reaches both optical and contact passes");
  a->removeFromSuperview();
  root->addSubview(a);
  expect(a->contentView()->superview().lock() == a,
         "reparenting retains selected-content ownership");
  bool badGeometry = false;
  const auto before = a->bounds();
  try {
    a->setBounds({0, 0, std::numeric_limits<NXFloat>::infinity(), 20});
  } catch (const std::invalid_argument &) {
    badGeometry = true;
  }
  expect(badGeometry && a->bounds() == before,
         "invalid geometry cannot poison the optical shader");
  a->setBounds({3.25f, 4.5f, 0, 0});
  expect(a->contentView()->frame().size == NXSize(0, 0),
         "empty lens geometry is supported");
  a->setBounds({0, 0, 80.5f, 48.5f});
  UIView::animate(1, [a] { a->setBounds({0, 0, 180.5f, 64.5f}); });
  const auto tick = [&](Timer time) {
    const auto visit = [&](const auto &self,
                           const std::shared_ptr<CALayer> &layer) -> void {
      layer->animateAt(time);
      for (const auto &child : layer->sublayers())
        self(self, child);
    };
    visit(visit, a->layer());
  };
  tick(Timer(10));
  tick(Timer(510));
  // Each layer records its own animation creation timestamp; allow less than
  // a tenth of a Retina pixel for the intervening clock ticks.
  expect(std::abs(a->layer()->presentationOrSelf()->bounds().size.width -
                  optics->layer()->presentationOrSelf()->bounds().size.width) <
             .05f,
         "UIView bounds animation keeps the optical presentation size aligned");
  a->setLifted(true);
  a->setLifted(false, true);
  LensViewTestHarness::advance(a, .1);
  const auto presented = std::static_pointer_cast<CABackdropEffectLayer>(
      optics->layer()->presentationOrSelf());
  expect(
      std::abs(presented->effect().uniform("lensProgress")->front() -
               LensViewTestHarness::progress(a)) < .0001f,
      "interrupted lifted state reaches active material presentation copies");
  expect(
      !presented->isHidden() &&
          std::abs(
              a->subviews()[1]->layer()->presentationOrSelf()->opacity() -
              (1 - LensViewTestHarness::progress(a))) < .0001f,
      "active size animation receives lifted visibility and resting opacity "
      "changes");
  tick(Timer(2010));
}
void testExternalSourceLifetimeAndCompositing() {
  auto root = new_shared<Root>(); root->setFrame({0,0,240,180});
  auto host = new_shared<UIView>(NXRect(20,20,180,100)); root->addSubview(host);
  auto source = new_shared<UIView>(NXRect(0,0,180,100));
  source->setBackgroundColor(UIColor(16,96,232,128)); host->addSubview(source);
  auto lens = new_shared<LensView>(LensView::Style::selection);
  lens->setFrame({40,40,120,64}); root->addSubview(lens);
  lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);
  lens->setLiftedContentView(source);
  const auto actual = pixel(root,100,72);
  expect(std::abs(int(SkColorGetR(actual))-72)<=1 && std::abs(int(SkColorGetG(actual))-112)<=1 && std::abs(int(SkColorGetB(actual))-180)<=1,
         "external translucent source composites once while its ordinary rendering is owned by the lens");
  host->setAlpha(0);
  expect(pixel(root,100,72)==SkColorSetRGB(128,128,128),
         "source ancestor visibility remains effective for portal input");
  host->setAlpha(1);
  expect(pixel(root,100,72)==actual,"restoring source ancestor opacity restores optical input");
  auto drawn = new_shared<DrawnSource>(NXRect(0,0,180,100)); source->addSubview(drawn);
  expect(SkColorGetB(pixel(root,100,72))==255 && drawn->draws>0,
         "owned optical source subtrees display live content independently of ordinary layer traversal");
  drawn->blue=false; drawn->setNeedsDisplay();
  expect(SkColorGetR(pixel(root,100,72))==255,
         "source draw invalidation reaches optical input on the next frame");
  const auto draws=drawn->draws; root->changeTraits();
  pixel(root,100,72);
  expect(drawn->draws>draws,"trait changes redraw live selected-source content");
  drawn->removeFromSuperview();
  auto mask = new_shared<CALayer>(); mask->setBounds({0,0,1,1}); mask->setBackgroundColor(UIColor::white);
  source->layer()->setMask(mask);
  expect(pixel(root,100,72)==actual && source->layer()->mask()==mask,
         "recording ignores only the source-root output mask without mutating its ownership");
  source->setBackgroundColor(UIColor::blue);
  expect(SkColorGetB(pixel(root,100,72))==255,
         "suppressed ordinary hosts do not prevent source content mutations reaching the lens");
  source->removeFromSuperview();
  expect(pixel(root,100,72)==SkColorSetRGB(128,128,128),
         "a detached source stops rendering safely");
  host->addSubview(source);
  expect(SkColorGetB(pixel(root,100,72))==255,
         "reparenting a source into the lens hierarchy restores its content");
  bool rejected=false;
  try { lens->setLiftedContentView(root); } catch(const std::invalid_argument&) { rejected=true; }
  expect(rejected && lens->liftedContentView()==source,
         "a source containing the lens is rejected without replacing the current source");
  source->removeFromSuperview(); source.reset();
  expect(!lens->liftedContentView() && pixel(root,100,72)==SkColorSetRGB(128,128,128),
         "external sources are weak and may expire without a render cycle");
  lens->setLiftedContentView(nullptr);
  auto owned = new_shared<UIView>(NXRect(50,24,12,12)); owned->setBackgroundColor(UIColor::red);
  lens->contentView()->addSubview(owned);
  expect(SkColorGetR(pixel(root,96,70))==255,
         "clearing the external source restores the owned selected-content portal");
}
void testElasticRootPreservesStationarySource() {
  for(auto style:{LensView::Style::regular,LensView::Style::selection}) {
    auto root=new_shared<Root>(); root->setFrame({0,0,240,180});
    auto source=new_shared<UIView>(NXRect(20,20,180,100)); root->addSubview(source);
    auto mark=new_shared<UIView>(NXRect(74,48,12,12)); mark->setBackgroundColor(UIColor::red); source->addSubview(mark);
    auto lens=new_shared<LensView>(style); lens->setFrame({40,40,120,64}); root->addSubview(lens);
    lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);
  lens->setLiftedContentView(source); lens->setLifted(true);
    expect(pixel(root,95,78)==UIColor::red.raw(),"stationary source marker initially occupies its parent coordinates");
    const NXAffineTransform elastic(1.2f,0,.1f,.9f,-11.2f,1.2f);
    LensViewTestHarness::elastic(lens,elastic);
    expect(lens->interactionTransform()==elastic,"public elastic geometry matches the root presentation transform");
    expect(pixel(root,95,78)==UIColor::red.raw(),"deforming the lens aperture does not deform the stationary selected row twice");
    expect(lens->frame()==NXRect(40,40,120,64)&&lens->transform().isIdentity(),"elastic root rendering preserves caller model frame and application transform");
    lens->setBounds({7.25f,-4.5f,120,64});
    expect(pixel(root,95,78)==UIColor::red.raw(),"stationary optical input remains aligned with fractional nonzero lens bounds origin");
    LensViewTestHarness::elastic(lens,NXAffineTransform::identity);
  }
}
void testInteractiveSourcesAndTermination() {
  auto root=new_shared<Root>(); root->setFrame({0,0,240,180});
  auto parent=new_shared<UIView>(NXRect(10,20,220,120)); root->addSubview(parent);
  auto lens=new_shared<LensView>(); lens->setFrame({40,25,100,52}); parent->addSubview(lens);
  expect(!lens->isInteractive()&&lens->interactionSource()==LensView::InteractionSource::contact,"lens elasticity is opt-in with a usable fixed-contact default");
  lens->setInteractive(true); lens->setUserInteractionEnabled(false);
  lens->beginInteraction({90,50});
  for(int i=0;i<24;i++) { lens->updateInteraction({90+i*2.0f,50},{240,0}); LensViewTestHarness::advance(lens,1.0/120); }
  const auto contact=lens->interactionTransform();
  expect(!contact.isIdentity(),"explicit contact excites a fixed knob even when ordinary lens hit testing is disabled");
  expect(lens->frame()==NXRect(40,25,100,52)&&lens->transform().isIdentity(),"contact deformation preserves caller model geometry");
  LensViewTestHarness::advance(lens,10);
  for(int i=0;i<24;i++) LensViewTestHarness::advance(lens,1.0/120);
  expect(lens->interactionTransform().isIdentity(),"resuming a held contact after suspension does not revive expired velocity");
  for(int i=0;i<24;i++) { lens->updateInteraction({90+i*2.0f,50},{240,0}); LensViewTestHarness::advance(lens,1.0/120); }
  const auto beforeCancel=lens->interactionTransform();
  expect(!beforeCancel.isIdentity(),"fresh contact input still excites the lens after suspension");
  lens->endInteraction({},true);
  expect(lens->interactionTransform()==beforeCancel,"cancellation retains current displayed geometry before the next damped step");
  for(int i=0;i<500;i++) LensViewTestHarness::advance(lens,1.0/120);
  expect(lens->interactionTransform().isIdentity(),"cancelled contact settles to the exact unmodified aperture");
  lens->setInteractionSource(LensView::InteractionSource::viewMotion);
  lens->beginInteraction({90,50});
  for(int i=0;i<24;i++) { lens->updateInteraction({90+i*2.0f,50},{240,0}); LensViewTestHarness::advance(lens,1.0/120); }
  expect(lens->interactionTransform().isIdentity(),"view-motion mode does not double-count finger velocity when model geometry stays fixed");
  for(int i=0;i<24;i++) { parent->setFrame({10+i*2.0f,20,220,120}); LensViewTestHarness::advance(lens,1.0/120); }
  expect(!lens->interactionTransform().isIdentity(),"global ancestor movement excites a lens whose local frame is unchanged");
  lens->setInteractive(false);
  expect(lens->interactionTransform().isIdentity(),"disabling interaction immediately clears force and visual history");
  lens->setInteractive(true); lens->beginInteraction({90,50});
  for(int i=0;i<24;i++) { lens->setFrame({40+i*2.0f,25,100,52}); LensViewTestHarness::advance(lens,1.0/120); }
  expect(!lens->interactionTransform().isIdentity(),"caller-controlled lens movement excites view-motion elasticity");
  const auto frame=lens->frame();
  auto other=new_shared<UIView>(NXRect(0,0,240,180)); root->addSubview(other); other->addSubview(lens);
  LensViewTestHarness::advance(lens,1.0/120);
  expect(lens->interactionTransform().isIdentity()&&lens->frame()==frame,"reparenting clears stale world-space input without changing caller placement");
  lens->setInteractionSource(LensView::InteractionSource::contact); lens->beginInteraction({90,50});
  for(int i=0;i<24;i++) { lens->updateInteraction({90+i*2.0f,50},{240,0}); LensViewTestHarness::advance(lens,1.0/120); }
  other->setHidden(true); LensViewTestHarness::advance(lens,1.0/120);
  expect(lens->interactionTransform().isIdentity(),"hiding an ancestor terminates active contact and force history");
  bool rejected=false;
  try { lens->updateInteraction({std::numeric_limits<NXFloat>::quiet_NaN(),0},{}); } catch(const std::invalid_argument&) { rejected=true; }
  expect(rejected,"nonfinite public motion input is rejected before state mutation");
}
} // namespace
int main() {
  testNativeTabRetirement();
  testRestingConfiguration();
  testPublicLensLifecycleAndModes();
  testStateAndRendering();
  testSelectedContentKeepsInteriorGeometryAndColor();
  testBelowGlassBlurPolicy();
  testVibrantForeground();
  testOptionalMagnification();
  testAnimationAndLayout();
  testExternalSourceLifetimeAndCompositing();
  testElasticRootPreservesStationarySource();
  testInteractiveSourcesAndTermination();
  return failures ? 1 : 0;
}
