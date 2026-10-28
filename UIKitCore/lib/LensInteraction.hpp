#pragma once

#include <Geometry.h>
#include <NXAffineTransform.h>
#include <algorithm>
#include <cmath>
#include <deque>
#include <numbers>

namespace NXKit::Internal {

// Portable transfer reconstruction from actual UIKit view-motion traces.
// Native diagnostics separately observe a 50ms hysteresis, five-sample,
// directionless velocity integrator and independent debounced scale/drift.
// This finite-window analytic response models their combined displayed result;
// it is not an implementation of Apple's private integrator classes.
struct LensInteraction {
  static constexpr double contactHysteresis = .05;
  static constexpr double omega = 11.106513384;
  static constexpr double damping = .596489182;
  static constexpr double window = .166959661;
  static constexpr double phase = .029928912;
  static constexpr double gain = .005374962;
  struct Pulse { double time; double x, y, ax, ay; };
  std::deque<Pulse> pulses;
  double time = 0;
  NXPoint lastPosition;
  NXPoint lastVelocity;
  bool hasPosition = false;
  double strainX = 0, strainY = 0, driftX = 0, driftY = 0;

  static bool valid(NXPoint p) { return std::isfinite(p.x) && std::isfinite(p.y); }
  static bool valid(NXSize s) { return std::isfinite(s.width) && std::isfinite(s.height) && s.width > 0 && s.height > 0; }
  void reset() { *this = {}; }
  void rebase(NXPoint p) { lastPosition = p; hasPosition = valid(p); }
  static double step(double t) {
    if(t <= 0) return 0;
    const double frequency = omega * std::sqrt(1-damping*damping);
    return (1-std::exp(-damping*omega*t)*(std::cos(frequency*t)+damping*omega/frequency*std::sin(frequency*t)))/(omega*omega);
  }
  static double impulse(double t) {
    if(t <= 0) return 0;
    const double frequency = omega * std::sqrt(1-damping*damping);
    return std::exp(-damping*omega*t)*std::sin(frequency*t)/frequency;
  }
  static double response(double t) { return (step(t-phase)-step(t-phase-window))/window; }
  static double responseVelocity(double t) { return (impulse(t-phase)-impulse(t-phase-window))/window; }

  // Velocity changes drive separate signed drift and directionless strain.
  // Reversing direction cannot instantly flip the displayed affine.
  void advanceVelocity(NXPoint velocity,double seconds,NXSize size) {
    if(!std::isfinite(seconds) || seconds <= 0) return;
    // After a multi-second suspension every retained response is below the
    // visible tail. Drop history, but retain a monotonic clock so contact
    // timestamps from before suspension cannot become fresh again.
    if(seconds >= 3) {
      const double resumedTime = time + 3;
      reset();
      time = resumedTime;
      return;
    }
    if(!valid(size) || !valid(velocity)) { reset(); return; }
    const double limit = std::clamp(double(std::max(size.width,size.height))*40, 400.0, 20000.0);
    velocity.x = std::clamp(double(velocity.x),-limit,limit);
    velocity.y = std::clamp(double(velocity.y),-limit,limit);
    const double dx = double(velocity.x)-lastVelocity.x, dy = double(velocity.y)-lastVelocity.y;
    if(std::abs(dx)+std::abs(dy)>1e-8)
      pulses.push_back({time,dx,dy,std::abs(double(velocity.x))-std::abs(double(lastVelocity.x)),std::abs(double(velocity.y))-std::abs(double(lastVelocity.y))});
    lastVelocity = velocity;
    time += seconds;
    while(!pulses.empty() && time-pulses.front().time > 3) pulses.pop_front();
    strainX = strainY = driftX = driftY = 0;
    double derivative = 0;
    for(const auto& p:pulses) {
      const double r = gain*response(time-p.time), v = gain*responseVelocity(time-p.time);
      strainX += p.ax*r; strainY += p.ay*r; driftX += p.x*r; driftY += p.y*r;
      derivative += (std::abs(p.x)+std::abs(p.y)+std::abs(p.ax)+std::abs(p.ay))*std::abs(v);
    }
    const double magnitude = std::max({std::abs(strainX),std::abs(strainY),std::abs(driftX),std::abs(driftY)});
    if(lastVelocity == NXPoint() && !pulses.empty() && time-pulses.back().time > phase+window && magnitude < .0001 && derivative < .001) {
      pulses.clear(); strainX = strainY = driftX = driftY = 0;
    }
  }
  void advancePosition(NXPoint position,double seconds,NXSize size,NXAffineTransform inverseBasis = NXAffineTransform::identity) {
    if(!valid(position)) { reset(); return; }
    NXPoint velocity;
    if(hasPosition && seconds > 0) {
      const double x = (double(position.x)-lastPosition.x)/seconds, y = (double(position.y)-lastPosition.y)/seconds;
      velocity = {NXFloat(std::clamp(x*inverseBasis.m11+y*inverseBasis.m21,-20000.0,20000.0)),NXFloat(std::clamp(x*inverseBasis.m12+y*inverseBasis.m22,-20000.0,20000.0))};
    }
    lastPosition = position; hasPosition = true;
    advanceVelocity(velocity,seconds,size);
  }
  bool atRest() const { return pulses.empty() && lastVelocity == NXPoint(); }
  NXAffineTransform transform(NXSize size) const {
    if(!valid(size)) return NXAffineTransform::identity;
    const double strain = std::clamp(strainX-strainY,-.25,.25);
    const double sx = 1+strain, sy = 1-strain;
    const double dx = -size.width*.5*std::clamp(driftX,-.3,.3), dy = -size.height*.5*std::clamp(driftY,-.3,.3);
    return {NXFloat(sx),0,0,NXFloat(sy),NXFloat(size.width*.5*(1-sx)+sx*dx),NXFloat(size.height*.5*(1-sy)+sy*dy)};
  }
};
}
