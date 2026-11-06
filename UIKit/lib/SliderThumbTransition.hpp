#pragma once
#include "GlassInteraction.hpp"

namespace NXKit::Internal {
// Native small-lens response/retarget impulse, verified against presentation
// bounds and resting-fill opacity. See docs/slider/lift-motion.
struct SliderThumbTransition {
    GlassSpring size;
    GlassSpring glass;
    GlassSpring optics;
    bool lifted=false;
    double elapsed=0;

    void retarget(bool value) {
        if(lifted==value)return;
        lifted=value;
        elapsed=0;
        const double omega=2*std::numbers::pi/(lifted?.27:.5);
        size.velocity+=(double(lifted)-size.value)*omega*omega*(lifted?.032:.016);
        optics.velocity+=(double(lifted)-optics.value)*omega*omega*(lifted?.032:.016);
        if(lifted)glass.velocity+=(1-glass.value)*omega*omega*.032;
        else glass.velocity=0;
    }
    void finish(bool value) {
        lifted=value;elapsed=0;size={double(value),0};glass=size;optics=size;
    }
    void advance(double seconds) {
        elapsed+=seconds;
        size.advance(lifted?1:0,seconds,lifted?.27:.5,lifted?.375:.3);
        optics.advance(lifted?1:0,seconds,lifted?.27:.5,lifted?.375:.3);
        if(lifted)glass.advance(1,seconds,.27,.375);
        else glass.advance(0,seconds,.4,0);
        // UIKit terminates bounds before its opacity spring's final tail.
        if(elapsed>=(lifted?.25:.46))size={lifted?1.:0.,0};
        if(elapsed>=(lifted?.33:.48))glass={lifted?1.:0.,0};
        if(elapsed>=(lifted?.5:.3))optics={lifted?1.:0.,0};
    }
    bool atRest() const {return size.atRest(lifted?1:0)&&glass.atRest(lifted?1:0)&&optics.atRest(lifted?1:0);}
};
}
