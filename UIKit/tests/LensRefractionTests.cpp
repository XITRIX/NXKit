// Regression coverage for selected-source optics. Sharp center ink alone is
// insufficient: the previous overlay composition passed that check while
// losing every refracted copy at the perimeter.
#include <LensView.h>
#include <SkiaCtx.h>
#include <UILabel.h>
#include <UIImageView.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include <cmath>
#include <fstream>
#include <array>
#include <iostream>
#include <vector>
#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static auto traits(bool dark) {
        auto t=std::make_shared<UITraitCollection>();
        t->_displayScale=2;
        t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;
        return t;
    }
};
class LensViewTestHarness {
public:
    static void progress(const std::shared_ptr<LensView>& lens,float p) { lens->_progress=p;lens->updateMaterial(); }
};
class RefractionTestRoot final : public UIView {
public:
    explicit RefractionTestRoot(bool dark) {
        _traitCollection=GlassMaterialTestHarness::traits(dark);
        setFrame({0,0,280,200});
        setBackgroundColor(UIColor(128,128,128));
    }
};
}
namespace {
int failures=0;
class FontContext final : public SkiaCtx {
public:
    FontContext() {
#ifdef __APPLE__
        fontMgr=SkFontMgr_New_CoreText(nullptr);
#endif
    }
    NXKit::NXSize getSize() override { return {280,200}; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
void expect(bool value,const char* reason) {
    if(!value){++failures;std::cerr<<"FAIL: "<<reason<<'\n';}
}
SkBitmap render(const std::shared_ptr<UIView>& root) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(560,400));
    surface->getCanvas()->scale(2,2);
    surface->getCanvas()->clear(SK_ColorBLACK);
    root->layer()->skiaRender(surface->getCanvas());
    SkBitmap image;image.allocN32Pixels(560,400);surface->readPixels(image,0,0);return image;
}
auto mark(NXRect frame,UIColor color) {
    auto view=new_shared<UIView>(frame);view->setBackgroundColor(color);return view;
}
bool cyan(SkColor color) {
    return SkColorGetB(color)>SkColorGetR(color)+45 && SkColorGetG(color)>SkColorGetR(color)+35;
}
struct Scene {
    std::shared_ptr<RefractionTestRoot> root;
    std::shared_ptr<UIView> sourceHost,source;
    std::shared_ptr<LensView> lens;
    std::vector<NXRect> bars;
    explicit Scene(LensView::Style style,bool dark,float width=116,float height=56) {
        root=new_shared<RefractionTestRoot>(dark);
        sourceHost=new_shared<UIView>(root->bounds());root->addSubview(sourceHost);
        source=new_shared<UIView>(root->bounds());sourceHost->addSubview(source);
        lens=new_shared<LensView>(style);lens->setFrame({60,40,width,height});root->addSubview(lens);
        const float cx=60+width/2,cy=40+height/2;
        // Native one-point sweeps: selection source at8pt reaches the edge;
        // regular source at12pt has a second support near.5pt depth. A source
        // immediately beside the boundary can instead disappear entirely.
        const float depth=style==LensView::Style::selection?8:12;
        bars={{cx-12,40+depth,24,1},{cx-12,40+height-depth-1,24,1},{60+depth,cy-8,1,16},{60+width-depth-1,cy-8,1,16}};
        for(auto rect:bars)source->addSubview(mark(rect,UIColor(0,210,255)));
        source->addSubview(mark({cx-4,cy-4,8,8},UIColor(17,94,231)));
#ifdef NXKIT_LENS_REFRACTION_BEFORE
        // Compile the same regression against the frozen old core to establish
        // that a sharp selected overlay cannot satisfy the edge-copy checks.
        source->removeFromSuperview();source->setFrame({-60,-40,280,200});lens->contentView()->addSubview(source);
#else
        lens->setLiftedContentView(source);
#endif
        lens->setLifted(true);
    }
};
float difference(SkColor a,SkColor b) {
    return (std::abs(int(SkColorGetR(a))-int(SkColorGetR(b)))+std::abs(int(SkColorGetG(a))-int(SkColorGetG(b)))+std::abs(int(SkColorGetB(a))-int(SkColorGetB(b))))/3.0f;
}
SkBitmap emptyReference(LensView::Style style,bool dark) {
    Scene scene(style,dark);const auto children=scene.source->subviews();
    for(const auto& child:children)child->removeFromSuperview();
    return render(scene.root);
}
int copies(const SkBitmap& image,const SkBitmap& reference,NXRect region,NXRect original) {
    int count=0;
    for(int y=int(region.minY()*2);y<int(region.maxY()*2);++y)
        for(int x=int(region.minX()*2);x<int(region.maxX()*2);++x) {
            const float px=(x+.5f)/2,py=(y+.5f)/2;
            const bool originalSupport=px>original.minX()-1 && px<original.maxX()+1 && py>original.minY()-1 && py<original.maxY()+1;
            if(!originalSupport && cyan(image.getColor(x,y)) && difference(image.getColor(x,y),reference.getColor(x,y))>10)++count;
        }
    return count;
}
void testCopiesAtEveryEdge() {
    for(bool dark:{false,true})for(auto style:{LensView::Style::regular,LensView::Style::selection}) {
        Scene scene(style,dark);auto image=render(scene.root);const auto reference=emptyReference(style,dark);
        const std::vector<NXRect> regions={{102,40,32,17},{102,79,32,17},{60,56,17,24},{159,56,17,24}};
        for(size_t i=0;i<regions.size();++i) {
            const auto count=copies(image,reference,regions[i],scene.bars[i]);
            if(count<4)std::cerr<<"edge "<<i<<", style "<<int(style)<<", dark "<<dark<<", copy pixels "<<count<<'\n';
            expect(count>=4,"lifted selected source has visible ink outside original support at every edge");
        }
        auto center=image.getColor(236,136);
        expect(SkColorGetB(center)>210 && SkColorGetR(center)<45,"selected center ink remains sharp while perimeter copies are sampled");
    }
}
void testNearRimSourceDoesNotStayAsSharpOverlay() {
    Scene scene(LensView::Style::selection,false);
    const auto children=scene.source->subviews();
    for(const auto& child:children)child->removeFromSuperview();
    const auto reference=render(scene.root);
    scene.source->addSubview(mark({106,44,24,1},UIColor(0,210,255)));
    const auto image=render(scene.root);
    // Native source y14 with lens top10 (4pt depth) is absent in its complete
    // top profile. This specifically rejects retaining the unwarped source tip.
    int visible=0;
    for(int y=80;y<112;++y)for(int x=204;x<272;++x)if(difference(image.getColor(x,y),reference.getColor(x,y))>10)++visible;
    expect(visible==0,"near-rim selected ink is remapped away instead of left as a sharp overlay");
}
void testSourceUpdatesAndOwnAlpha() {
#ifndef NXKIT_LENS_REFRACTION_BEFORE
    Scene scene(LensView::Style::selection,false);
    scene.source->addSubview(mark({10,10,12,12},UIColor(0,210,255)));
    auto before=render(scene.root);
    const auto outside=before.getColor(30,30);
    expect(SkColorGetR(outside)==128 && SkColorGetB(outside)==128,"selected source ordinary pass is suppressed outside the lens");
    scene.source->setAlpha(.35f);auto faded=render(scene.root);
    const auto b=faded.getColor(236,136);
    expect(std::abs(int(SkColorGetR(b))-89)<=4 && std::abs(int(SkColorGetB(b))-164)<=4,"source own alpha composites once while its ordinary pass is suppressed");
    scene.source->setAlpha(1);
    auto center=scene.source->subviews()[4];center->setBackgroundColor(UIColor(231,42,17));
    auto changed=render(scene.root).getColor(236,136);
    expect(SkColorGetR(changed)>210 && SkColorGetB(changed)<45,"mutating selected ink invalidates the sampled source without replacing it");
    scene.sourceHost->setAlpha(0);
    const auto hidden=render(scene.root).getColor(236,136);
    expect(std::abs(int(SkColorGetR(hidden))-int(SkColorGetB(hidden)))<8,"an actually invisible source ancestor hides the optical input");
    scene.sourceHost->setAlpha(1);
    const auto restored=render(scene.root).getColor(236,136);
    expect(SkColorGetR(restored)>210 && SkColorGetB(restored)<45,"restoring ancestor visibility restores the sampled source");
    scene.lens->setLiftedContentView(nullptr);
    expect(!scene.lens->liftedContentView(),"external content source can be cleared without changing ownership");
    const auto released=render(scene.root).getColor(30,30);
    expect(cyan(released),"clearing the source restores its caller-owned ordinary rendering");
#endif
}
void testCenterInkKeepsRetinaPixelEdges() {
#ifndef NXKIT_LENS_REFRACTION_BEFORE
    Scene scene(LensView::Style::selection,false);scene.lens->setFrame({60.25f,40,116,56});const auto image=render(scene.root);
    // The8pt center mark has integer point bounds114,64..122,72. Its first
    // Retina pixel is fully covered; sampling may not add a second blur/phase.
    const auto inside=image.getColor(228,134),outside=image.getColor(227,134);
    expect(std::abs(int(SkColorGetR(inside))-17)<=1 && std::abs(int(SkColorGetB(inside))-231)<=1,"identity center mapping preserves the first fully covered Retina ink pixel");
    expect(std::abs(int(SkColorGetR(outside))-128)<=1 && std::abs(int(SkColorGetB(outside))-128)<=1,"identity center mapping does not leak ink into the adjacent pixel");
    // Also preserve a caller-supplied2×RGBA texture. Vector rectangles alone
    // miss an extra resampling pass on already-rasterized text and symbols.
    SkBitmap texture;texture.allocN32Pixels(560,400);texture.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas textureCanvas(texture);SkPaint texturePaint;texturePaint.setColor(SkColorSetRGB(17,94,231));textureCanvas.drawRect(SkRect::MakeXYWH(228,128,16,16),texturePaint);
    auto picture=std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(texture.asImage()),2);
    auto imageSource=new_shared<UIImageView>(picture);imageSource->setFrame(scene.root->bounds());scene.sourceHost->addSubview(imageSource);scene.lens->setLiftedContentView(imageSource);scene.source->removeFromSuperview();
    const auto raster=render(scene.root);const auto rasterInside=raster.getColor(228,134),rasterOutside=raster.getColor(227,134);
    expect(std::abs(int(SkColorGetR(rasterInside))-17)<=1 && std::abs(int(SkColorGetB(rasterInside))-231)<=1,"identity center mapping preserves a pre-rasterized Retina texture pixel");
    expect(std::abs(int(SkColorGetR(rasterOutside))-128)<=1 && std::abs(int(SkColorGetB(rasterOutside))-128)<=1,"Retina texture sampling does not add a second edge filter");
    scene.lens->setContentVibrancyEnabled(true);
    const auto vibrant=render(scene.root);
    expect(difference(vibrant.getColor(228,134),vibrant.getColor(236,134))<=1,"vibrant image-filter sampling preserves the first fully covered Retina texture pixel");
    expect(difference(vibrant.getColor(227,134),SkColorSetRGB(128,128,128))<=1,"vibrancy adds no extra resampling halo to an adjacent Retina pixel");
    scene.lens->setContentVibrancyEnabled(false);
    imageSource->setAlpha(.5f);const auto translucent=render(scene.root).getColor(228,134);
    expect(std::abs(int(SkColorGetR(translucent))-73)<=2 && std::abs(int(SkColorGetB(translucent))-180)<=2,"fractional-origin translucent texture composites once without double attenuation");
#endif
}
// Exercise an entire sharp center patch, not just the exact center point.
// Continuous distance approximation overshoots half-height away from the axes;
// a post-blend clamp used to bend this patch as the lens passed over its ink.
void testCenterTextureIsIdentityDuringTranslation(const std::string& output={}) {
    SkBitmap texture;texture.allocN32Pixels(560,400);
    for(int y=0;y<400;++y)for(int x=0;x<560;++x)
        *texture.getAddr32(x,y)=SkPreMultiplyColor(SkColorSetRGB((x%8<4)?23:231,(y%8<4)?41:207,91+(x+y)%93));
    auto picture=std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(texture.asImage()),2);
    for(bool dark:{false,true})for(auto style:{LensView::Style::regular,LensView::Style::selection})
        for(NXKit::NXSize size:{NXKit::NXSize{95,54},NXKit::NXSize{111,70},NXKit::NXSize{116,56},NXKit::NXSize{70,110}}) {
            Scene scene(style,dark,size.width,size.height);
            const auto children=scene.source->subviews();
            for(const auto& child:children)child->removeFromSuperview();
            auto source=new_shared<UIImageView>(picture);source->setFrame(scene.root->bounds());
            scene.sourceHost->addSubview(source);scene.lens->setLiftedContentView(source);
            for(float offset:{0.f,.25f,8.5f}) {
                const NXRect frame{60+offset,40.25f,size.width,size.height};scene.lens->setFrame(frame);
                const auto actual=render(scene.root);
                if(!output.empty() && size.width==95 && offset==8.5f) {
                    std::ofstream file(output+"-"+(dark?"dark":"light")+"-"+(style==LensView::Style::selection?"selection":"regular")+".ppm",std::ios::binary);
                    file<<"P6\n560 400\n255\n";
                    for(int y=0;y<400;++y)for(int x=0;x<560;++x) {
                        const auto color=actual.getColor(x,y);
                        const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(color)),static_cast<unsigned char>(SkColorGetG(color)),static_cast<unsigned char>(SkColorGetB(color))};
                        file.write(reinterpret_cast<const char*>(rgb),3);
                    }
                }
                int mismatches=0;
                for(int y=int((frame.minY()+size.height*.4f)*2);y<int((frame.minY()+size.height*.6f)*2);++y)
                    for(int x=int((frame.minX()+size.width*.3f)*2);x<int((frame.minX()+size.width*.7f)*2);++x)
                        if(difference(actual.getColor(x,y),texture.getColor(x,y))>1)++mismatches;
                expect(mismatches==0,"lifted lens preserves the complete center texture during fractional translation and resizing");
            }
        }
}
// An independently transformed ordinary source is the reference. Matching at
// intermediate progress also rejects blending unscaled and enlarged glyphs.
void testUniformLiftedMagnification() {
    for(bool dark:{false,true})for(auto style:{LensView::Style::regular,LensView::Style::selection}) {
        Scene scene(style,dark,160,100);
        const auto children=scene.source->subviews();for(const auto& child:children)child->removeFromSuperview();
        for(int i=0;i<5;++i) scene.source->addSubview(mark({111.f+i*9,78,2,24},UIColor(17+i*33,94,231-i*29)));
        scene.lens->setLiftedContentMagnification(1.16f);
        for(float progress:{0.f,.5f,1.f}) {
            LensViewTestHarness::progress(scene.lens,progress);
            const auto actual=render(scene.root);
            scene.source->setHidden(true);const auto noInk=render(scene.root);scene.source->setHidden(false);
            auto reference=new_shared<RefractionTestRoot>(dark);
            const auto background=noInk.getColor(280,180);
            reference->setBackgroundColor(UIColor(SkColorGetR(background),SkColorGetG(background),SkColorGetB(background)));
            auto content=new_shared<UIView>(reference->bounds());reference->addSubview(content);
            const auto pivot=NXKit::NXPoint(140,90);
            content->layer()->setAnchorPoint({pivot.x/280,pivot.y/200});content->layer()->setPosition(pivot);
            content->setTransform(NXAffineTransform::scale(1+.16f*progress));
            for(int i=0;i<5;++i)content->addSubview(mark({111.f+i*9,78,2,24},UIColor(17+i*33,94,231-i*29)));
            const auto expected=render(reference);
            float error=0;int count=0;
            for(int y=164;y<196;++y)for(int x=214;x<310;++x) {error+=difference(actual.getColor(x,y),expected.getColor(x,y));++count;}
            if(error/count>1)std::cerr<<"magnification error "<<error/count<<" style "<<int(style)<<" dark "<<dark<<" progress "<<progress<<'\n';
            expect(error/count<1,"optional magnification matches an independent uniform source transform throughout lift");
        }
        scene.lens->setLifted(false);
        expect(scene.lens->liftedContentMagnification()==1.16f,"release retains caller magnification for the next lift");
    }
}
void testLazyLabelSourceIsPrepared() {
#if defined(__APPLE__) && !defined(NXKIT_LENS_REFRACTION_BEFORE)
    const auto previous=SkiaCtx::_main;
    SkiaCtx::_main=std::make_shared<FontContext>();
    Scene scene(LensView::Style::selection,false);
    const auto children=scene.source->subviews();
    for(const auto& child:children)child->removeFromSuperview();
    auto label=new_shared<UILabel>();label->setFrame({98,56,44,24});label->setFontSize(20);label->setTextColor(UIColor(0,210,255));label->setText("XX");scene.source->addSubview(label);
    const auto first=render(scene.root);
    int visible=0;
    for(int y=110;y<164;++y)for(int x=194;x<286;++x)if(cyan(first.getColor(x,y)))++visible;
    expect(visible>20,"a lazily drawn UILabel is prepared even when its ordinary source pass is suppressed");
    label->setText("O");const auto second=render(scene.root);int changed=0;
    for(int y=110;y<164;++y)for(int x=194;x<286;++x)if(first.getColor(x,y)!=second.getColor(x,y))++changed;
    expect(changed>20,"label text mutations reach the lens source without an ordinary draw pass");
    SkiaCtx::_main=previous;
#endif
}
struct Centroid { float x=0,y=0,mass=0; };
std::array<Centroid,3> channelCentroids(const SkBitmap& image,const SkBitmap& control,NXRect region,SkColor source=SK_ColorBLACK) {
    std::array<Centroid,3> result;
    const int sourceRGB[]={int(SkColorGetR(source)),int(SkColorGetG(source)),int(SkColorGetB(source))};
    for(int y=int(region.minY()*2);y<int(region.maxY()*2);++y)for(int x=int(region.minX()*2);x<int(region.maxX()*2);++x) {
        const auto a=image.getColor(x,y),b=control.getColor(x,y);
        const int ink[]={int(SkColorGetR(a)),int(SkColorGetG(a)),int(SkColorGetB(a))};
        const int background[]={int(SkColorGetR(b)),int(SkColorGetG(b)),int(SkColorGetB(b))};
        for(int c=0;c<3;++c)if(std::abs(sourceRGB[c]-background[c])>=16) {
            // Normalize each channel against the same no-ink lens. This also
            // distinguishes tinted ink from colored background/rim light.
            const auto weight=std::max(0.0f,float(ink[c]-background[c])/(sourceRGB[c]-background[c])-.1f);
            result[c].x+=(x+.5f)/2*weight;result[c].y+=(y+.5f)/2*weight;result[c].mass+=weight;
        }
    }
    for(auto& c:result)if(c.mass>0){c.x/=c.mass;c.y/=c.mass;}
    return result;
}
void testTintedInkHasVisibleChromaticTransport() {
#ifndef NXKIT_LENS_REFRACTION_BEFORE
    // UIKit's isolated marker captures give about 1.6pt G/B separation at the
    // top and 1.05pt at the curved side. Saturated ink must follow that map too;
    // background fringes alone cannot pass this test.
    const std::array<NXKit::NXPoint,4> points={NXKit::NXPoint(118,48),NXKit::NXPoint(118,88),NXKit::NXPoint(68,68),NXKit::NXPoint(168,68)};
    for(bool dark:{false,true})for(const auto color:{SkColorSetRGB(0,122,255),SkColorSetRGB(0,210,255)}) {
        Scene scene(LensView::Style::selection,dark);
        scene.root->setBackgroundColor(UIColor(32,32,32));
        const auto children=scene.source->subviews();for(const auto& child:children)child->removeFromSuperview();
        const auto control=render(scene.root);
        for(size_t i=0;i<points.size();++i) {
            const auto p=points[i];auto dot=mark({p.x-2,p.y-2,4,4},UIColor(SkColorGetR(color),SkColorGetG(color),SkColorGetB(color)));
            dot->layer()->setCornerRadius(2);dot->layer()->setCornerCurve(CALayerCornerCurve::circular);scene.source->addSubview(dot);
            const auto centers=channelCentroids(render(scene.root),control,{p.x-18,std::max(40.5f,p.y-18),36,std::min(95.5f,p.y+18)-std::max(40.5f,p.y-18)},color);
            expect(centers[1].mass>1&&centers[2].mass>1,"selected blue/cyan ink contributes both channels independently of the ordinary backdrop");
            const float separation=(i<2?centers[1].x-centers[2].x:centers[1].y-centers[2].y)*(i==1||i==3?-1:1);
            expect(separation>(i<2?1.45f:.9f)&&separation<(i<2?1.9f:1.4f),"tinted selected ink retains the native-sized G/B fringe at straight and curved edges");
            if(i>=2) {
                const auto radial=(centers[2].x-centers[1].x)*(i==2?1:-1);
                expect(radial>.15f&&radial<.55f,"selected blue/cyan markers reproduce UIKit's additional side-normal channel separation");
            }
            dot->removeFromSuperview();
        }
    }
#endif
}
void testBlueAndWhiteStrokeRadialTransport() {
    // A long vertical stroke removes the tangential-direction cue that let
    // round-marker tests pass despite missing side-normal channel separation.
    // UIKit's retained axial markers place B about .3pt inward from G.
    float gaps[2][2][2]={};
    for(bool dark:{false,true})for(const auto color:{SkColorSetRGB(0,122,255),SK_ColorWHITE}) {
        Scene scene(LensView::Style::selection,dark);
        scene.root->setBackgroundColor(UIColor(32,32,32));
        const auto children=scene.source->subviews();for(const auto& child:children)child->removeFromSuperview();
        const auto control=render(scene.root);
        for(bool right:{false,true}) {
            auto stroke=mark({right?167.5f:67.5f,58,1,20},UIColor(SkColorGetR(color),SkColorGetG(color),SkColorGetB(color)));
            scene.source->addSubview(stroke);
            const auto centers=channelCentroids(render(scene.root),control,{right?161.f:60.5f,65,14,6},color);
            const auto separation=(centers[2].x-centers[1].x)*(right?-1:1);
            gaps[dark][color==SK_ColorWHITE][right]=separation;
            if(separation<.15f || separation>1.25f)
                std::cerr<<"Stroke radial gap="<<separation<<" dark="<<dark<<" right="<<right<<" blue="<<(color!=SK_ColorWHITE)<<'\n';
            expect(centers[1].mass>1&&centers[2].mass>1&&separation>.15f&&separation<1.25f,
                   "blue and white vertical glyph strokes both retain inward blue-channel dispersion at the side rim");
            stroke->removeFromSuperview();
        }
    }
    for(int dark=0;dark<2;++dark)for(int side=0;side<2;++side)
        expect(std::abs(gaps[dark][0][side]-gaps[dark][1][side])<.03f,
               "tint changes color, while blue and white glyphs retain the same chromatic ray positions");
}
void testBlackInkHasObservedChannelTransport() {
#ifndef NXKIT_LENS_REFRACTION_BEFORE
    // The native colored-marker probes isolate each channel's coverage. These
    // direction constraints also apply to black ink: the effect must transport
    // destination transmission, not rely on a source hue to manufacture fringes.
    const std::array<NXKit::NXPoint,5> points={NXKit::NXPoint(118,48),NXKit::NXPoint(118,88),NXKit::NXPoint(68,68),NXKit::NXPoint(168,68),NXKit::NXPoint(80,52)};
    for(bool dark:{false,true})for(auto style:{LensView::Style::regular,LensView::Style::selection}) {
        Scene scene(style,dark);const auto children=scene.source->subviews();
        for(const auto& child:children)child->removeFromSuperview();
        const auto control=render(scene.root);
        for(size_t i=0;i<points.size();++i) {
            const auto p=points[i];auto dot=mark({p.x-2,p.y-2,4,4},UIColor::black);dot->layer()->setCornerRadius(2);dot->layer()->setCornerCurve(CALayerCornerCurve::circular);scene.source->addSubview(dot);
            auto image=render(scene.root);auto centers=channelCentroids(image,control,{p.x-20,std::max(40.0f,p.y-20),40,std::min(96.0f,p.y+20)-std::max(40.0f,p.y-20)});
            expect(centers[0].mass>1 && centers[1].mass>1 && centers[2].mass>1,"black sparse marker produces measurable source-induced coverage in every channel");
            if(style==LensView::Style::selection) {
                const float tangentR=(i<2||i==4)?centers[0].x:centers[0].y;
                const float tangentG=(i<2||i==4)?centers[1].x:centers[1].y;
                const float tangentB=(i<2||i==4)?centers[2].x:centers[2].y;
                const float sign=(i==1||i==3)?-1:1;
                expect(sign*(tangentR-tangentB)>.5f && sign*(tangentR-tangentG)>0 && sign*(tangentG-tangentB)>0,"selection lens reproduces observed opposite-edge RGB centroid ordering");
                if(i==4)expect(centers[0].y-centers[2].y<-.2f,"curved-rim channel transport reproduces the native fold reversal in its second axis");
            } else {
                const auto separation=std::hypot(centers[0].x-centers[2].x,centers[0].y-centers[2].y);
                expect(separation<.3f,"regular lens keeps the low channel separation observed in its marker controls");
            }
            dot->removeFromSuperview();
        }
    }
#endif
}
void testBackgroundRefractionHasObservedNearEdgePlacement() {
    // Native3pt-deep backdrop dots land about6.9pt inside the selection rim;
    // a12pt-deep line lands about14.3pt inside. Matching only one depth can
    // hide an incorrect displacement falloff.
    for(bool dark:{false,true}) {
        auto root=new_shared<RefractionTestRoot>(dark);
        auto backdrop=new_shared<UIView>(root->bounds());root->addSubview(backdrop);
        auto lens=new_shared<LensView>(LensView::Style::selection);lens->setFrame({60.25f,40,116,56});root->addSubview(lens);lens->setLifted(true);
        const auto control=render(root);
        auto dot=mark({116.25f,41,4,4},UIColor::black);dot->layer()->setCornerRadius(2);dot->layer()->setCornerCurve(CALayerCornerCurve::circular);backdrop->addSubview(dot);
        const auto near=channelCentroids(render(root),control,{108,40.5f,20,14});
        expect(near[1].mass>1 && near[1].y>46.2f && near[1].y<47.6f,"near-rim ordinary backdrop marker reaches the observed native green-channel depth");
        dot->removeFromSuperview();backdrop->addSubview(mark({106,52,24,1},UIColor::black));
        const auto deeper=channelCentroids(render(root),control,{110,50,16,10});
        expect(deeper[1].mass>1 && deeper[1].y>53.5f && deeper[1].y<55.25f,"deeper backdrop line retains the native displacement falloff");
    }
}

void testBackgroundHasIndependentChromaticTransport() {
    // Native backdrop-only monochrome grids split along the tangent: the
    // upper R centroid lies right of B, and the ordering reverses below.
    // This source is an ordinary sibling behind the lens, never a selected
    // portal. A selected-source-only shader cannot satisfy this regression.
    for(bool dark:{false,true})for(auto style:{LensView::Style::regular,LensView::Style::selection}) {
        auto root=new_shared<RefractionTestRoot>(dark);
        auto background=new_shared<UIView>(root->bounds());root->addSubview(background);
        auto lens=new_shared<LensView>(style);lens->setFrame({60,40,116,56});root->addSubview(lens);lens->setLifted(true);
        const auto control=render(root);
        auto vertical=mark({117.5f,30,1,76},UIColor::black);background->addSubview(vertical);
        const auto image=render(root);
        const auto top=channelCentroids(image,control,{108,40.5f,20,4});
        const auto bottom=channelCentroids(image,control,{108,91.5f,20,4});
        const auto center=channelCentroids(image,control,{110,63,16,10});
        for(const auto& c:top)expect(c.mass>1,"ordinary backdrop stripe has measurable per-channel coverage at the upper rim");
        for(const auto& c:bottom)expect(c.mass>1,"ordinary backdrop stripe has measurable per-channel coverage at the lower rim");
        if(style==LensView::Style::selection) {
            expect(top[0].x-top[2].x>2 && top[0].x>top[1].x && top[1].x>top[2].x,"selection lens splits ordinary backdrop RGB along the upper-edge tangent");
            expect(bottom[2].x-bottom[0].x>2 && bottom[0].x<bottom[1].x && bottom[1].x<bottom[2].x,"ordinary backdrop chromatic ordering reverses at the lower edge");
        } else {
            expect(std::abs(top[0].x-top[2].x)<.3f && std::abs(bottom[0].x-bottom[2].x)<.3f,"regular backdrop retains the low separation observed in native controls");
        }
        expect(std::abs(center[0].x-center[2].x)<.1f,"background chromatic separation does not affect the sharp center");
        vertical->removeFromSuperview();
        auto horizontal=mark({50,67.5f,136,1},UIColor::black);background->addSubview(horizontal);
        const auto sideImage=render(root);
        const auto left=channelCentroids(sideImage,control,{60.5f,58,4,20});
        const auto right=channelCentroids(sideImage,control,{171.5f,58,4,20});
        if(style==LensView::Style::selection) {
            expect(left[0].y-left[2].y>.5f && left[0].y>left[1].y && left[1].y>left[2].y,"ordinary backdrop RGB also separates tangentially at the left edge");
            expect(right[2].y-right[0].y>.5f && right[0].y<right[1].y && right[1].y<right[2].y,"ordinary backdrop RGB ordering reverses at the right edge");
        }
        lens->setLifted(false);const auto resting=render(root);
        background->removeFromSuperview();const auto restingControl=render(root);
        const auto rest=channelCentroids(resting,restingControl,{60.5f,58,4,20});
        expect(std::abs(rest[0].y-rest[2].y)<.1f,"resting lens introduces no chromatic transport");
    }
}

}
int main(int argc,char** argv) {
    if(argc>1 && std::string(argv[1])=="--sampling-only") {testCenterInkKeepsRetinaPixelEdges();return failures?1:0;}
    if(argc>1 && std::string(argv[1])=="--zoom-only") { testUniformLiftedMagnification(); return failures?1:0; }
    if(argc>1 && std::string(argv[1])=="--ink-only") {
        testBlueAndWhiteStrokeRadialTransport();testTintedInkHasVisibleChromaticTransport();
        if(failures)return 1;
        std::cout<<"Tinted radial transport checks passed\n";return 0;
    }
    if(argc>1 && std::string(argv[1])=="--center-only") {
        testCenterTextureIsIdentityDuringTranslation(argc>2?argv[2]:"");
        if(failures)return 1;
        std::cout<<"Lens center texture checks passed\n";
        return 0;
    }
    testUniformLiftedMagnification();
    testCenterTextureIsIdentityDuringTranslation();
    testBlueAndWhiteStrokeRadialTransport();
    testCopiesAtEveryEdge();testNearRimSourceDoesNotStayAsSharpOverlay();testSourceUpdatesAndOwnAlpha();testCenterInkKeepsRetinaPixelEdges();testLazyLabelSourceIsPrepared();testBlackInkHasObservedChannelTransport();testTintedInkHasVisibleChromaticTransport();testBackgroundHasIndependentChromaticTransport();testBackgroundRefractionHasObservedNearEdgePlacement();
    if(failures){std::cerr<<failures<<" lens refraction checks failed\n";return 1;}
    std::cout<<"Lens refraction checks passed\n";
}
