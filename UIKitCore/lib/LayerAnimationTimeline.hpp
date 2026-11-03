#pragma once
#include <CALayer.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace NXKit::Internal {
// A reversible private clock for a specific transaction's animations. Ordinary
// animations (including application animations in the same subtree) keep time.
// Completion callbacks are held until the owner commits; cancellation discards
// them only after removing all captured animations, before restoring UI state.
class LayerAnimationTimeline {
    struct Clock {
        double progress=0,from=0,target=0,velocity=0,duration=0,span;
        Timer origin;
        bool settling=false;
        std::function<double(double)> easing;
        explicit Clock(double seconds):span(seconds){}
        double at(Timer now) const {
            if(!settling)return progress;
            const auto t=std::clamp((now-origin)/(duration*1000),0.,1.);
            if(easing)return std::clamp(from+(target-from)*easing(t),0.,1.);
            // Cubic Hermite continuation preserves the release velocity and
            // arrives with zero velocity, also when returning to the source.
            const auto value=(2*t*t*t-3*t*t+1)*from+(t*t*t-2*t*t+t)*duration*velocity+(-2*t*t*t+3*t*t)*target;
            return std::clamp(value,0.,1.);
        }
    };
    struct Entry {std::shared_ptr<CALayer> layer;std::string key;std::shared_ptr<CABasicAnimation> animation;};
    std::shared_ptr<Clock> clock;
    std::vector<Entry> entries;
    std::vector<std::shared_ptr<CABasicAnimation>> baseline;
    template<class F> static void visit(const std::shared_ptr<CALayer>& layer,const F& work) {
        work(layer);const auto children=layer->sublayers();for(const auto& child:children)visit(child,work);
    }
public:
    explicit LayerAnimationTimeline(const std::shared_ptr<CALayer>& root,double duration):clock(std::make_shared<Clock>(duration)) {
        if(!std::isfinite(duration)||duration<=0)throw std::invalid_argument("Interactive timeline duration must be positive");
        visit(root,[&](const auto& layer){for(const auto& [key,animation]:layer->animations)baseline.push_back(animation);});
    }
    ~LayerAnimationTimeline(){discard();}
    void capture(const std::shared_ptr<CALayer>& root) {
        visit(root,[&](const auto& layer){for(const auto& [key,animation]:layer->animations) {
            if(std::find(baseline.begin(),baseline.end(),animation)!=baseline.end()||animation->_interactiveElapsed)continue;
            animation->_interactiveElapsed=[c=clock](Timer now){return c->at(now)*c->span*1000;};
            entries.push_back({layer,key,animation});
        }});
        sample();
    }
    double progress(Timer now=Timer()) const{return clock->at(now);}
    void seek(double progress) {
        if(!std::isfinite(progress))throw std::invalid_argument("Interactive progress must be finite");
        clock->progress=std::clamp(progress,0.,1.);clock->settling=false;sample();
    }
    void settle(double target,double velocity,double duration,std::function<double(double)> easing={}) {
        clock->from=progress();clock->target=std::clamp(target,0.,1.);clock->velocity=velocity;clock->duration=std::max(.001,duration);clock->easing=std::move(easing);clock->origin=Timer();clock->settling=true;
    }
    void sample() {
        const auto now=Timer();std::unordered_set<CALayer*> sampled;
        for(const auto& entry:entries)if(entry.layer->animations.contains(entry.key)&&entry.layer->animations.at(entry.key)==entry.animation&&sampled.insert(entry.layer.get()).second)entry.layer->animateAt(now);
    }
    void discard(){remove(false);}
    void complete(){seek(1);remove(true);}
private:
    void remove(bool complete) {
        auto removed=std::move(entries);entries.clear();
        for(const auto& entry:removed) {
            if(entry.layer->animations.contains(entry.key)&&entry.layer->animations.at(entry.key)==entry.animation)entry.layer->removeAnimation(entry.key);
            entry.animation->_interactiveElapsed={};
        }
        baseline.clear();
        // No observer sees a partially removed transaction.
        if(complete)for(const auto& entry:removed)if(entry.animation->animationGroup)entry.animation->animationGroup->animationDidStop(true);
    }
};
}
