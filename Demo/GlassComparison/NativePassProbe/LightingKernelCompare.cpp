// Compare only the production SDF-lighting transfer against native knockout
// pairs. The unlit backdrop is measured in the native no-highlight screenshot,
// so this deliberately does not measure body/refraction/blur image equivalence.
#include "NativeGlassLighting.hpp"
#include "NativeGlassProfile.hpp"
#include <BackdropEffect.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace NXKit;using namespace NXKit::Internal;
int main() {
 const std::string source=std::string("uniform float contentScale;\n")+
  std::string(nativeGlassLightingUniforms())+std::string(nativeGlassLightingShaderSource())+R"sksl(
uniform float testSDF;
uniform float2 testNormal;
uniform float3 testBackdrop;
half4 main(float2 xy) {return half4(nativeGlassHighlight(testSDF,testNormal,testNormal,half3(testBackdrop)),1);}
)sksl";
 auto compiled=SkRuntimeEffect::MakeForShader(SkString(source));
 if(!compiled.effect) {std::cerr<<compiled.errorText.c_str();return 1;}
 std::string line;while(std::getline(std::cin,line)) {
  std::istringstream input(line);int dark,r,g,b;float size,sdf,nx,ny;
  if(!(input>>dark>>size>>sdf>>nx>>ny>>r>>g>>b))return 2;
  auto profile=nativeGlassProfile(UIGlassEffect::Style::regular,dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light,.5f,{size,size},false);
  profile.keyFillHighlightAmount=0;profile.shadowAmount=0;
  BackdropEffect effect("unused");bindNativeGlassLighting(profile,effect,1);
  SkRuntimeEffectBuilder builder(compiled.effect);
  for(const auto& uniform:compiled.effect->uniforms())if(auto value=effect.uniform(std::string(uniform.name)))
   builder.uniform(std::string(uniform.name).c_str()).set(value->data(),value->size());
  builder.uniform("contentScale")=3.f;builder.uniform("testSDF")=sdf;
  builder.uniform("testNormal")=SkV2{nx,ny};builder.uniform("testBackdrop")=SkV3{r/255.f,g/255.f,b/255.f};
  auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1,1));SkPaint paint;paint.setShader(builder.makeShader());surface->getCanvas()->drawPaint(paint);
  SkBitmap bitmap;bitmap.allocN32Pixels(1,1);if(!surface->readPixels(bitmap,0,0))return 3;
  auto color=bitmap.getColor(0,0);std::cout<<SkColorGetR(color)<<' '<<SkColorGetG(color)<<' '<<SkColorGetB(color)<<'\n';
 }
}
