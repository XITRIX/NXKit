#include "LensInteraction.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace NXKit;
using NXKit::Internal::LensInteraction;
namespace {
int failures=0;
void expect(bool value,const char* message) { if(!value) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; } }
std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> result; std::istringstream input(line); std::string value;
  while(std::getline(input,value,',')) { if(!value.empty()&&value.back()=='\r') value.pop_back(); result.push_back(value); }
  return result;
}
void invariants() {
  LensInteraction a,b;
  a.advanceVelocity({240,0},.1,{100,52}); b.advanceVelocity({240,0},.05,{100,52}); b.advanceVelocity({240,0},.05,{100,52});
  expect(std::abs(a.strainX-b.strainX)<1e-12,"analytic elastic response is independent of display subdivision");
  auto before=a.transform({100,52});
  expect(before!=NXAffineTransform::identity,"real motion excites an elastic affine");
  a.advanceVelocity({-240,0},.001,{100,52});
  expect(std::abs(a.transform({100,52}).m11-before.m11)<.01,"reversal retains displayed strain instead of flipping it with velocity sign");
  for(int i=0;i<500;i++) a.advanceVelocity({},.01,{100,52});
  expect(a.atRest()&&a.transform({100,52}).isIdentity(),"release restores exact caller geometry and stops the response");
  LensInteraction extreme;
  for(int i=0;i<100;i++) extreme.advanceVelocity({i%2?1e30f:-1e30f,1e30f},.001,{1,10000});
  const auto t=extreme.transform({1,10000});
  expect(std::isfinite(t.tX)&&std::isfinite(t.tY)&&t.m11>=.75f&&t.m11<=1.25f&&t.m22>=.75f&&t.m22<=1.25f,"unmeasured extreme velocity and aspect ratio remain finite and bounded");
  extreme.advanceVelocity({},10,{100,52});
  expect(extreme.transform({100,52}).isIdentity(),"a stalled frame does not destabilize the analytic response");
}
}
int main(int argc,char** argv) {
  invariants();
  auto path=argc>1?std::filesystem::path(argv[1]):std::filesystem::path(NXKIT_LENS_MOTION_REFERENCE);
  if(argc==1) { const auto bundled=std::filesystem::path(argv[0]).parent_path()/"native-reference.csv"; if(std::filesystem::exists(bundled)) path=bundled; }
  std::ifstream input(path); if(!input) { std::cerr<<"Cannot open "<<path<<'\n'; return 1; }
  std::ofstream output; if(argc>2) output.open(argv[2]);
  std::string line; std::getline(input,line); auto header=split(line); std::map<std::string,size_t> index; for(size_t i=0;i<header.size();i++) index[header[i]]=i;
  if(output) output<<line<<'\n';
  struct Score { double before=0,after=0,max=0; size_t count=0; };
  std::map<std::string,Score> scores; LensInteraction motion; std::string current; double previous=0;
  while(std::getline(input,line)) {
    auto row=split(line); const auto name=row[index.at("case")];
    const auto value=[&](const char* key) { return std::stod(row[index.at(key)]); };
    const auto put=[&](const char* key,double number) { std::ostringstream stream; stream.precision(12); stream<<number; row[index.at(key)]=stream.str(); };
    const double time=value("t"), cx=value("inputCenterX"),cy=value("inputCenterY"),w=value("localWidth"),h=value("localHeight"),ax=value("ancestorScaleX"),ay=value("ancestorScaleY");
    if(name!=current) { motion.reset(); motion.rebase({NXFloat(cx),NXFloat(cy)}); previous=time; current=name; }
    motion.advancePosition({NXFloat(cx),NXFloat(cy)},time-previous,{NXFloat(w),NXFloat(h)},NXAffineTransform(1/ax,0,0,1/ay,0,0)); previous=time;
    const auto t=motion.transform({NXFloat(w),NXFloat(h)});
    const double dx=(t.tX+w*.5*(t.m11-1))*ax,dy=(t.tY+h*.5*(t.m22-1))*ay;
    const std::array<double,4> nx={cx+dx-w*ax*t.m11*.5,cy+dy-h*ay*t.m22*.5,w*ax*t.m11,h*ay*t.m22};
    const std::array<double,4> old={cx-w*ax*.5,cy-h*ay*.5,w*ax,h*ay};
    auto& score=scores[name]; size_t k=0;
    for(const auto* key:{"nativeX","nativeY","nativeW","nativeH"}) { const double truth=value(key);score.before+=(old[k]-truth)*(old[k]-truth);score.after+=(nx[k]-truth)*(nx[k]-truth);score.max=std::max(score.max,std::abs(nx[k]-truth));++score.count;++k; }
    put("nxX",nx[0]);put("nxY",nx[1]);put("nxW",nx[2]);put("nxH",nx[3]);put("nxScaleX",t.m11);put("nxScaleY",t.m22);put("nxDriftX",dx/ax);put("nxDriftY",dy/ay);
    if(output) { for(size_t i=0;i<row.size();i++) output<<(i?",":"")<<row[i];output<<'\n'; }
  }
  for(const auto& [name,score]:scores) {
    const double before=std::sqrt(score.before/score.count),after=std::sqrt(score.after/score.count);
    std::cout<<name<<" geometry RMSE "<<before<<" -> "<<after<<" pt; max "<<score.max<<" pt\n";
    expect(after<before,"native measured elastic geometry improves over a rigid lens in every retained case");
    if(name!="slider-cancel") expect(after<1.0&&score.max<4.0,"native gesture replay remains within documented reconstruction residuals");
  }
  expect(scores.size()>=8,"native replay retains slow/fast/left/reversal, tabs, switch, and control interruption");
  return failures?1:0;
}
