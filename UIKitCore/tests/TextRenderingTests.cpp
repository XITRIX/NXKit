#include "../../UIKitCore/lib/platforms/apple/AppleSystemFont.hpp"
// Actual UILabel raster versus native UILabel; the custom-font cases share file bytes.
#include <UILabel.h>
#include "RobotoTestFontManager.hpp"
#include <UITextView.h>
#include <SkiaCtx.h>
#include "../../UIKitCore/lib/FontTracking.hpp"
#include "../../UIKitCore/lib/TextRendering.hpp"
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <limits>
#include <cmath>
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include <CoreText/CoreText.h>
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static auto traits(int scale,bool dark) {
        auto t=std::make_shared<UITraitCollection>(); t->_displayScale=scale;
        t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; return t;
    }
};
class TextTestRoot final : public UIView {
public:
    TextTestRoot(int scale,bool dark) { _traitCollection=GlassMaterialTestHarness::traits(scale,dark); }
};
}
namespace {
class Context final : public SkiaCtx {
public:
    bool custom = false;
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style = SkFontStyle(), NXFloat size = 17) const override { return custom ? SkiaCtx::getDefaultTypeface(style,size) : Internal::appleSystemTypeface(style,size); }
    void useCustom(bool custom) { this->custom=custom; fontMgr=custom?Internal::makeRobotoTestFontManager():SkFontMgr_New_CoreText(nullptr); }
    NXKit::NXSize getSize() override { return {400,592}; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
struct Sample { const char *id,*text; float size,weight; bool custom; float height=52; };
const Sample samples[]={
    {"caption","Home · Favorites 123",11,400,true},
    {"footnote","Agjpqy ÉÅ accents 0123",13,400,true},
    {"body","Background · Item 1",17,400,true},
    {"title","Glass Agjpqy",22,400,true},
    {"large","Typography Agjpqy",34,400,true},
    {"system-body","Background · Item 1",17,400,false},
    {"system-headline","Home · Collection",17,600,false},
    {"system-bold","Bold text 012345",22,700,false},
    {"multiline","Agjpqy first line\nSecond line ÉÅ",17,400,true,72},
    {"fallback","Hello Привет 😀",17,400,false},
    {"empty","",17,400,true}
};
}
int runTests(const std::shared_ptr<Context>& ctx) {
    int failures=0;
    const auto expect=[&](bool good,const char* message) { if(!good) { ++failures;std::cerr<<"FAIL: "<<message<<'\n'; } };
    auto root=new_shared<TextTestRoot>(3,false);root->setFrame({0,0,400,200});
    auto label=new_shared<UILabel>();root->addSubview(label);label->setFrame({0,0,376,52});
    auto unattached=new_shared<UILabel>();unattached->setText("Home");
    expect(unattached->sizeThatFits({1000,1000}).width>0,"labels can measure before inheriting window traits");
    ctx->useCustom(true);
    expect(label->sizeThatFits({376,1000})==NXKit::NXSize(),"empty labels measure zero instead of a negative width");
    label->setText("Background · Item 1");
    auto fits=label->sizeThatFits({376,1000});
    expect(fits.width>=151&&fits.width<=152&&std::abs(fits.height-20)<.01f,"same-file Roboto body measurement matches native UILabel within one display pixel");
    label->setText("Agjpqy first line\nSecond line ÉÅ");fits=label->sizeThatFits({376,1000});
    expect(std::abs(fits.height-40)<.01f,"two-line label uses typographic line heights");
    bool rejected=false;try { label->setFontSize(std::numeric_limits<float>::quiet_NaN()); } catch(const std::invalid_argument&) { rejected=true; }
    expect(rejected,"nonfinite font size is rejected");
    rejected=false;try { label->setFontWeight(std::numeric_limits<float>::infinity()); } catch(const std::invalid_argument&) { rejected=true; }
    expect(rejected,"nonfinite font weight is rejected");
    expect(label->applyXMLAttribute("fontWeight","600")&&label->fontWeight()==600,"XML font weight reaches the same rendering property");
    label->setFontWeight(400);
    ctx->useCustom(false);label->setText("Hello");auto plain=label->sizeThatFits({376,1000});label->setText("Hello Привет 😀");fits=label->sizeThatFits({376,1000});
    expect(fits.height==plain.height&&fits.width>plain.width,"fallback emoji retains the primary font line box");
    const auto italic=ctx->getDefaultTypeface(SkFontStyle(400,5,SkFontStyle::kItalic_Slant),17);
    SkString italicName;
    if (italic) italic->getPostScriptName(&italicName);
    // Skia's Core Text adapter can report an upright numeric slant for the
    // variable SF italic face. Check the selected native face itself.
    expect(italic&&std::string(italicName.c_str()).find("Italic")!=std::string::npos,"system font resolution selects the requested italic face");
    const auto coverage=[&](float weight) {
        label->setText("Home Collection");label->setFontWeight(weight);label->draw();
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1200,156));surface->getCanvas()->clear(SK_ColorWHITE);surface->getCanvas()->scale(3,3);label->layer()->skiaRender(surface->getCanvas());
        SkBitmap pixels;pixels.allocN32Pixels(1200,156);surface->readPixels(pixels,0,0);double ink=0;
        for(int y=0;y<156;++y)for(int x=0;x<1200;++x)ink+=255-SkColorGetR(pixels.getColor(x,y));return ink;
    };
    auto regular=coverage(400),semibold=coverage(600),bold=coverage(700);
    expect(regular>0&&semibold>regular*1.1&&bold>semibold,"requested system weights produce distinct real font weights");
    for(int scale:{1,2,3}) for(float weight:{500,600}) {
        auto captionRoot=new_shared<TextTestRoot>(scale,false);
        auto caption=new_shared<UILabel>();captionRoot->addSubview(caption);
        caption->setFontSize(10);caption->setFontWeight(weight);
        for(const char* text:{"Home","Favorites","Library","Settings"}) {
            caption->setText(text);
            const auto intrinsic=caption->sizeThatFits({1000,1000});
            const auto fitted=caption->sizeThatFits(intrinsic);
            expect(fitted.height==intrinsic.height,"a caption fitted to its Retina-pixel intrinsic width stays on one line");
            expect(caption->sizeThatFits({intrinsic.width*.5f,1000}).height>intrinsic.height,"captions still wrap when the width genuinely cannot fit the text");
        }
    }
    ctx->useCustom(true);label->setFontWeight(400);label->setText("Background · Item 1");label->draw();
    const auto inkFor=[&](const auto& draw) {
        SkSurfaceProps props(0,kUnknown_SkPixelGeometry,0,1);
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1200,156),&props);
        auto canvas=surface->getCanvas();canvas->clear(SK_ColorWHITE);canvas->scale(3,3);draw(canvas);
        SkBitmap pixels;pixels.allocN32Pixels(1200,156);surface->readPixels(pixels,0,0);double ink=0;
        for(int y=0;y<156;++y)for(int x=0;x<1200;++x)ink+=255-SkColorGetR(pixels.getColor(x,y));return ink;
    };
    const auto labelInk=inkFor([&](SkCanvas* c){label->layer()->skiaRender(c);});
    auto textLayer=new_shared<CATextLayer>();textLayer->setBounds({0,0,376,52});textLayer->setText(label->text());
    const auto layerInk=inkFor([&](SkCanvas* c){textLayer->draw(c);});
    expect(std::abs(layerInk/labelInk-1)<.001,"CATextLayer and UILabel share native-like glyph coverage");
    auto textView=new_shared<UITextView>();root->addSubview(textView);textView->setFrame({0,0,376,52});textView->setTextContainerInset({});textView->setText(label->text());textView->drawAndLayoutTreeIfNeeded();
    const auto viewInk=inkFor([&](SkCanvas* c){textView->layer()->skiaRender(c);});
    expect(std::abs(viewInk/labelInk-1)<.001,"UITextView tiles and UILabel share native-like glyph coverage");
    std::vector<uint8_t> table(40);
    const auto word=[&](size_t p,uint16_t v) { table[p]=v>>8;table[p+1]=v; };
    const auto wide=[&](size_t p,uint32_t v) { word(p,v>>16);word(p+2,v); };
    wide(0,0x10000);word(6,12);word(12,1);word(14,2);wide(16,28);wide(20,0);word(26,36);wide(28,10*65536);wide(32,20*65536);word(36,uint16_t(-20));word(38,uint16_t(-10));
    expect(std::abs(Internal::normalFontTracking(table.data(),table.size(),1000,15)+.225f)<.00001f,"AAT normal tracking interpolates in font units");
    expect(Internal::normalFontTracking(table.data(),10,1000,15)==0,"truncated tracking header is rejected");
    expect(Internal::normalFontTracking(table.data(),38,1000,15)==0,"truncated tracking values are rejected");
    wide(16,0xfffffff0);expect(Internal::normalFontTracking(table.data(),table.size(),1000,15)==0,"out-of-range tracking offsets are rejected");
    root.reset();if(!failures)std::cout<<"Text renderer regressions passed\n";return failures?1:0;
}
int main(int argc,char**argv) {
    const bool tests=argc==1;
    if(!tests&&argc!=5) { std::cerr<<"OUTPUT.ppm SCALE DARK FONT.ttf\n"; return 2; }
    const auto scale=tests?3:std::stoi(argv[2]);const bool dark=tests?false:bool(std::stoi(argv[3]));
    const char* fontPath=tests?NXKIT_TEXT_TEST_FONT:argv[4];
    auto url=CFURLCreateFromFileSystemRepresentation(nullptr,reinterpret_cast<const UInt8*>(fontPath),std::strlen(fontPath),false);
    if(!CTFontManagerRegisterFontsForURL(url,kCTFontManagerScopeProcess,nullptr)) { std::cerr<<"Cannot register test font\n";CFRelease(url);return 2; }CFRelease(url);
    auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;
    if(!tests&&std::string(argv[1])=="--tab-shaping") {
        ctx->useCustom(false);
        using namespace skia::textlayout;
        for(int weight:{600,500})for(const char* text:{"Home","Favorites","Library","Settings"}) {
            TextStyle style;style.setFontSize(10);style.setFontStyle(SkFontStyle(weight,5,SkFontStyle::kUpright_Slant));
            auto fonts=Internal::plainTextFonts(ctx,style.getFontStyle(),10,style);
            ParagraphStyle ps;ps.setTextStyle(style);ps.setApplyRoundingHack(false);
            auto builder=ParagraphBuilder::make(ps,fonts,SkUnicodes::ICU::Make());builder->addText(text);auto p=builder->Build();p->layout(1000);
            std::cout<<text<<' '<<weight<<" tracking="<<style.getLetterSpacing()<<" width="<<p->getLongestLine()<<'\n';
            p->visit([&](int,const Paragraph::VisitorInfo* run){if(run)for(int i=0;i<run->count;i++)std::cout<<run->glyphs[i]<<':'<<run->positions[i].x()<<' ';});std::cout<<'\n';
        }
        SkiaCtx::_main.reset();return 0;
    }
    if(tests) { int result=runTests(ctx);SkiaCtx::_main.reset();return result; }
    auto root=new_shared<TextTestRoot>(scale,dark);root->setFrame({0,0,400,592});
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400*scale,592*scale));auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SK_ColorWHITE);canvas->scale(scale,scale);
    std::ofstream meta(std::string(argv[1])+".json");meta<<"{\"samples\":[";float y=0;int index=0;
    for(const auto& s:samples) {
        ctx->useCustom(s.custom);
        auto label=new_shared<UILabel>();root->addSubview(label);label->setFrame({12,y,376,s.height});label->setText(s.text);label->setFontSize(s.size);label->setFontWeight(s.weight);label->setTextColor(dark?UIColor::white:UIColor::black);
        UITraitCollection::setCurrent(root->traitCollection()); label->draw();
        auto fits=label->sizeThatFits({376,1000});
        label->layer()->skiaRender(canvas);
        auto face=ctx->getDefaultTypeface(SkFontStyle(int(s.weight),5,SkFontStyle::kUpright_Slant),s.size);SkString font;face->getPostScriptName(&font);
        if(index++)meta<<',';meta<<"{\"id\":"<<std::quoted(s.id)<<",\"font\":"<<std::quoted(font.c_str())<<",\"fits\":["<<fits.width<<','<<fits.height<<"]}";y+=s.height;
        label->removeFromSuperview();
    }
    meta<<"]}\n";
    SkBitmap bitmap;bitmap.allocN32Pixels(400*scale,592*scale);surface->readPixels(bitmap,0,0);
    std::ofstream out(argv[1],std::ios::binary);out<<"P6\n"<<bitmap.width()<<' '<<bitmap.height()<<"\n255\n";
    for(int y=0;y<bitmap.height();++y)for(int x=0;x<bitmap.width();++x){auto c=bitmap.getColor(x,y);unsigned char rgb[]={uint8_t(SkColorGetR(c)),uint8_t(SkColorGetG(c)),uint8_t(SkColorGetB(c))};out.write(reinterpret_cast<char*>(rgb),3);}
    root.reset();SkiaCtx::_main.reset();
}
