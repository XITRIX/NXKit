#include "NativeGlassAccent.hpp"
#include "NativeGlassAccentData.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace NXKit;
using namespace NXKit::Internal;
static void check(bool value,const char* reason) { if(!value) { std::cerr<<reason<<'\n';std::exit(1); } }
int main() {
    size_t checks=0;
    for(const auto& variant:nativeGlassAccentVariants) {
        NativeGlassProfile profile{};profile.accent.vibrant=variant.reference;
        for(const auto& sample:variant.samples) {
            const auto exact=nativeGlassAccentMatrix(profile,sample.color);
            for(size_t i=0;i<20;++i)
                check(std::abs(exact.matrix.values[i]-sample.vibrant.matrix.values[i])<1e-6,"Native CPU matrix differs from captured coefficients");
            // Native gray matrices form a separate diagonal family. Approach its
            // anchors along the gray axis, and chromatic anchors through RGB.
            auto close=sample.color;
            close.r=std::clamp(close.r+(close.r<.5 ? NXFloat(.0001):NXFloat(-.0001)),NXFloat(0),NXFloat(1));
            close.g=std::clamp(close.g+(close.g<.5 ? NXFloat(.00013):NXFloat(-.00013)),NXFloat(0),NXFloat(1));
            if(std::max({sample.color.r,sample.color.g,sample.color.b})-std::min({sample.color.r,sample.color.g,sample.color.b})<=NXFloat(1e-6))
                close.g=close.b=close.r;
            const auto adjacent=nativeGlassAccentMatrix(profile,close);
            for(size_t i=0;i<20;++i)
                check(std::isfinite(adjacent.matrix.values[i])&&std::abs(adjacent.matrix.values[i]-exact.matrix.values[i])<.001,"Native tint equation does not converge to its captured limit");
            ++checks;
        }
        for(int i=0;i<=100;++i) {
            const NXFloat gray=NXFloat(i)/100;
            const auto result=nativeGlassAccentMatrix(profile,{gray,gray,gray,1});
            for(auto value:result.matrix.values) check(std::isfinite(value),"Achromatic tint equation is not finite");
        }
        const auto invalid=nativeGlassAccentMatrix(profile,{std::numeric_limits<NXFloat>::quiet_NaN(),0,0,1});
        check(invalid.matrix.values==profile.accent.vibrant.matrix.values,"Nonfinite tint must preserve resolved profile");
    }
    struct Holdout { bool dark,clear; NativeGlassColor color; NativeGlassMatrix matrix; };
    const Holdout holdouts[]={
#include "fixtures/NativeGlassAccentHoldouts.inc"
    };
    double totalError=0,maximumError=0;size_t components=0;
    for(const auto& sample:holdouts) {
        const auto& variant=*std::find_if(nativeGlassAccentVariants.begin(),nativeGlassAccentVariants.end(),
            [&](const auto& v){return v.dark==sample.dark && v.clear==sample.clear;});
        NativeGlassProfile profile{};profile.accent.vibrant=variant.reference;
        const auto result=nativeGlassAccentMatrix(profile,sample.color);
        for(size_t i=0;i<20;++i)
            check(std::abs(result.matrix.values[i]-sample.matrix.values[i])<1e-6,
                "Withheld native accent coefficient differs from the CPU equation");
        const auto gray=std::max({sample.color.r,sample.color.g,sample.color.b})-std::min({sample.color.r,sample.color.g,sample.color.b})<=NXFloat(1e-6);
        for(const double luma:{0.,.125,.25,.5,.75,.875,1.}) for(size_t row=0;row<3;++row) {
            double actual=result.matrix.values[row*5+4],native=sample.matrix.values[row*5+4];
            for(size_t column=0;column<3;++column) {
                actual+=result.matrix.values[row*5+column]*luma;
                native+=sample.matrix.values[row*5+column]*luma;
            }
            const double error=255*std::abs(std::clamp(actual,0.,1.)-std::clamp(native,0.,1.));
            check(error<.001,"Withheld native accent response exceeded its measured bound");
            totalError+=error;maximumError=std::max(maximumError,error);++components;
        }
        if(!gray) for(size_t row=0;row<3;++row) {
            double reference=result.matrix.values[row*5+4];
            for(size_t column=0;column<3;++column) reference+=result.matrix.values[row*5+column]*(sample.dark&&!sample.clear ? .15 : 1.);
            const std::array<NXFloat,3> color={sample.color.r,sample.color.g,sample.color.b};
            check(std::abs(reference-color[row])<1e-5,"Arbitrary accent tint drifts at the native reference luminance");
        }
    }
    check(totalError/components<.0001,"Mean error across the independent accent palette regressed");
    std::cout<<"32 withheld native accent matrices: mean "<<totalError/components<<", max "<<maximumError<<"/255\n";
    std::cout<<checks<<" native coefficient sets and anchor limits passed\n";
}
