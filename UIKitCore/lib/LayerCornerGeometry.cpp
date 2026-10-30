// Rounded-superellipse parameters adapted from Flutter/Impeller:
// Copyright 2013 The Flutter Authors. All rights reserved.
// BSD-3-Clause; see licenses/Flutter-BSD-3-Clause.txt.
// Reference: flutter/flutter@5d93b2e71b3380bd0d2e4f0daf14379fcbd64a4d,
// engine/src/flutter/impeller/geometry/round_superellipse_param.cc.
#include "LayerCornerGeometry.hpp"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <vector>

namespace NXKit::Internal {
namespace {
bool valid(NXSize size) {
    return std::isfinite(size.width) && std::isfinite(size.height)
        && size.width>0 && size.height>0;
}
NXFloat clampedRadius(NXSize size,NXFloat radius) {
    return std::isfinite(radius) ? std::clamp(radius,0.0f,std::min(size.width,size.height)*.5f) : 0;
}
struct Octant {
    double a,n,jx,jy,cx,cy,r;
};
Octant octant(double a,double radius) {
    // Flutter's fitted exponent and reciprocal join coordinate. Interpolation
    // preserves its shape model; rendering below uses analytic Hermite cubics
    // rather than Flutter's conic approximation.
    constexpr double table[][2] = {
        {2.00000000,1.13276676},{2.18349805,1.20311921},
        {2.33888662,1.28698796},{2.48660575,1.36351941},
        {2.62226596,1.44717976},{2.75148990,1.53385819},
        {3.36298265,1.98288283},{4.08649929,2.23811846},
        {4.85481134,2.47563463},{5.62945551,2.72948597},
        {6.43023796,2.98020421}
    };
    const auto ratio=2*a/radius;
    double n,k;
    if(ratio>5) {
        n=table[10][0]+1.559599389*(ratio-5);
        k=table[10][1]+.522807185*(ratio-5);
    } else {
        const auto steps=ratio<2.5 ? (ratio-2)*10 : (ratio-2.5)*2+5;
        const auto index=std::clamp(int(std::floor(steps)),0,9);
        const auto t=steps-index;
        n=table[index][0]*(1-t)+table[index+1][0]*t;
        k=table[index][1]*(1-t)+table[index+1][1]*t;
    }
    const auto jx=(1-1/k)*a;
    const auto jy=std::pow(std::max(0.0,1-std::pow(jx/a,n)),1/n)*a;
    const auto slope=std::pow(jx/jy,n-1);
    const auto d=(jx-slope*jy)/(1-slope);
    const auto gap=.29289321881*radius;
    const auto r=std::max(0.0,(a-d-gap)*std::sqrt(2.0));
    const auto mx=a-gap,my=mx;
    const auto dx=mx-jx,dy=my-jy,chord=std::hypot(dx,dy);
    const auto offset=std::sqrt(std::max(0.0,r*r-chord*chord*.25));
    return {a,n,jx,jy,(jx+mx)*.5+offset*dy/chord,(jy+my)*.5-offset*dx/chord,r};
}
Octant unpack(const std::array<NXFloat,4>& se,const std::array<NXFloat,4>& circle) {
    return {se[0],se[1],se[2],se[3],circle[0],circle[1],circle[2]};
}
NXFloat octantDistance(double x,double y,const Octant& o) {
    // Select at the join's normal ray, rather than a vertical line through J.
    // Both fields therefore meet along their shared normal, avoiding an
    // artificial optical seam at the superellipse/circle boundary.
    if((x-o.cx)*(o.jy-o.cy)-(y-o.cy)*(o.jx-o.cx)>0)
        return std::hypot(x-o.cx,y-o.cy)-o.r;
    const auto m=std::max({x,y,1e-12});
    const auto norm=m*std::pow(std::pow(x/m,o.n)+std::pow(y/m,o.n),1/o.n);
    const auto divisor=std::max(norm,1e-12);
    const auto gx=std::pow(x/divisor,o.n-1),gy=std::pow(y/divisor,o.n-1);
    return (norm-o.a)/std::max(std::hypot(gx,gy),1e-6);
}
struct Cubic { SkPoint p,c1,c2,q; };
std::vector<Cubic> octantCubics(const Octant& o) {
    std::vector<Cubic> curves;
    constexpr int steps=16;
    const auto sample=[&](double t) {
        const auto x=o.jx*(1-std::pow(1-t,3));
        const auto y=o.a*std::pow(std::max(0.0,1-std::pow(x/o.a,o.n)),1/o.n);
        const auto derivative=-std::pow(x/y,o.n-1);
        return std::array<double,3>{x,y,derivative};
    };
    auto p=sample(0);
    for(int i=1;i<=steps;++i) {
        const auto q=sample(double(i)/steps);
        const auto dx=(q[0]-p[0])/3;
        double y1=p[1]+p[2]*dx,y2=q[1]-q[2]*dx;
        if(i==1 && o.n>2.00001) y1=y2=o.a;
        curves.push_back({SkPoint::Make(p[0],p[1]),SkPoint::Make(p[0]+dx,std::min(y1,o.a)),
            SkPoint::Make(q[0]-dx,std::min(y2,o.a)),SkPoint::Make(q[0],q[1])});
        p=q;
    }
    const auto mx=o.cx+o.r/std::sqrt(2.0),my=o.cy+o.r/std::sqrt(2.0);
    const auto start=std::atan2(o.jy-o.cy,o.jx-o.cx);
    const auto k=4.0/3*std::tan((start-std::acos(-1.0)*.25)*.25);
    curves.push_back({SkPoint::Make(o.jx,o.jy),SkPoint::Make(o.jx+k*(o.jy-o.cy),o.jy-k*(o.jx-o.cx)),
        SkPoint::Make(mx-k*(my-o.cy),my+k*(mx-o.cx)),SkPoint::Make(mx,my)});
    return curves;
}
SkPath buildCornerPath(NXSize size,NXFloat radius,CALayerCornerCurve curve) {
    SkPathBuilder path;
    const auto rect=SkRect::MakeWH(size.width,size.height);
    if(radius<.0001f || curve==CALayerCornerCurve::circular) {
        path.addRRect(SkRRect::MakeRectXY(rect,radius,radius));return path.detach();
    }
    const auto data=layerCornerShaderData(size,radius);
    auto top=octantCubics(unpack(data[0],data[1]));
    auto right=octantCubics(unpack(data[2],data[3]));
    const auto c=(size.width-size.height)*.5f;
    const auto moveTop=[&](SkPoint p) {return SkPoint::Make(p.x(),p.y()-c);};
    const auto moveRight=[&](SkPoint p) {return SkPoint::Make(p.y()+c,p.x());};
    std::vector<Cubic> quadrant;
    for(const auto& b:top)quadrant.push_back({moveTop(b.p),moveTop(b.c1),moveTop(b.c2),moveTop(b.q)});
    for(auto it=right.rbegin();it!=right.rend();++it)
        quadrant.push_back({moveRight(it->q),moveRight(it->c2),moveRight(it->c1),moveRight(it->p)});
    path.moveTo(size.width*.5f,0);
    for(int side=0;side<4;++side) {
        const bool reverse=side==1||side==3;
        const float sx=side<2?1:-1,sy=side==0||side==3?-1:1;
        const auto map=[&](SkPoint p) {return SkPoint::Make(size.width*.5f+sx*p.x(),size.height*.5f+sy*p.y());};
        const auto add=[&](const Cubic& b) {
            path.cubicTo(map(reverse?b.c2:b.c1),map(reverse?b.c1:b.c2),map(reverse?b.p:b.q));
        };
        if(reverse)for(auto it=quadrant.rbegin();it!=quadrant.rend();++it)add(*it);
        else for(const auto& b:quadrant)add(b);
    }
    path.close();return path.detach();
}
}

LayerCornerShaderData layerCornerShaderData(NXSize size,NXFloat radius) {
    if(!valid(size))return {};
    radius=clampedRadius(size,radius);
    if(radius<.0001f)return {};
    const auto top=octant(size.width*.5,radius),right=octant(size.height*.5,radius);
    return {{{NXFloat(top.a),NXFloat(top.n),NXFloat(top.jx),NXFloat(top.jy)},
             {NXFloat(top.cx),NXFloat(top.cy),NXFloat(top.r),0},
             {NXFloat(right.a),NXFloat(right.n),NXFloat(right.jx),NXFloat(right.jy)},
             {NXFloat(right.cx),NXFloat(right.cy),NXFloat(right.r),0}}};
}
SkPath layerCornerPath(NXSize size,NXFloat radius,CALayerCornerCurve curve) {
    if(!valid(size))return SkPath();
    radius=clampedRadius(size,radius);
    struct Entry {NXSize size;NXFloat radius;CALayerCornerCurve curve;SkPath path;};
    thread_local std::list<Entry> cache;
    for(auto it=cache.begin();it!=cache.end();++it)if(it->size==size&&it->radius==radius&&it->curve==curve) {
        cache.splice(cache.begin(),cache,it);return cache.front().path;
    }
    cache.push_front({size,radius,curve,buildCornerPath(size,radius,curve)});
    if(cache.size()>128)cache.pop_back();return cache.front().path;
}
NXFloat layerCornerDistance(NXPoint p,NXSize size,NXFloat radius,CALayerCornerCurve curve) {
    if(!valid(size)||!std::isfinite(p.x)||!std::isfinite(p.y))return std::numeric_limits<NXFloat>::infinity();
    radius=clampedRadius(size,radius);
    const auto x=std::abs(p.x),y=std::abs(p.y),hx=size.width*.5f,hy=size.height*.5f;
    if(radius<.0001f)return std::max(x-hx,y-hy);
    if(curve==CALayerCornerCurve::circular) {
        const auto dx=x-hx+radius,dy=y-hy+radius;
        return std::hypot(std::max(dx,0.0f),std::max(dy,0.0f))+std::min(std::max(dx,dy),0.0f)-radius;
    }
    const auto data=layerCornerShaderData(size,radius);const auto c=hx-hy;
    return y+c>=x ? octantDistance(x,y+c,unpack(data[0],data[1]))
        : octantDistance(y,x-c,unpack(data[2],data[3]));
}

const char* layerCornerShaderSource() {
    return R"SKSL(
float circularBoxSDF(float2 p,float2 halfDim,float r) {
    float2 q=abs(p)-halfDim+r;
    return length(max(q,0.0))+min(max(q.x,q.y),0.0)-r;
}
float3 continuousOctant(float2 p,float4 se,float4 circle) {
    float2 radial=p-circle.xy;
    float2 join=se.zw-circle.xy;
    if(radial.x*join.y-radial.y*join.x>0.0) {
        float len=length(radial);
        return float3(len-circle.z,radial/max(len,0.000001));
    }
    // Rescale before exponentiation: tiny radii have large exponents, and
    // direct pow(p/a,n) can overflow outside the shape.
    float m=max(max(p.x,p.y),0.000001);
    float norm=m*pow(pow(p.x/m,se.y)+pow(p.y/m,se.y),1.0/se.y);
    float2 gradient=pow(p/max(norm,0.000001),float2(se.y-1.0));
    float magnitude=max(length(gradient),0.000001);
    return float3((norm-se.x)/magnitude,gradient/magnitude);
}
float3 continuousCornerSample(float2 p,float2 halfDim) {
    float2 q=abs(p);float c=halfDim.x-halfDim.y;
    float3 result;
    if(q.y+c>=q.x)result=continuousOctant(float2(q.x,q.y+c),cornerTop,cornerTopCircle);
    else {
        result=continuousOctant(float2(q.y,q.x-c),cornerRight,cornerRightCircle);
        result.yz=result.zy;
    }
    float2 direction=float2(p.x>=0.0?1.0:-1.0,p.y>=0.0?1.0:-1.0);
    result.yz*=direction;return result;
}
float boxRoundedSDF(float2 p,float2 halfDim,float r) {
    return cornerCurve<0.5||r<=0.0 ? circularBoxSDF(p,halfDim,r) : continuousCornerSample(p,halfDim).x;
}
float2 lensNormalDirection(float2 p,float2 halfDim,float r) {
    if(cornerCurve>0.5&&r>0.0)return continuousCornerSample(p,halfDim).yz;
    float2 q=abs(p)-halfDim+r;
    float2 direction=float2(p.x>=0.0?1.0:-1.0,p.y>=0.0?1.0:-1.0);
    if(max(q.x,q.y)>0.0) {
        float2 n=max(q,0.0);return direction*n/max(length(n),0.000001);
    }
    return q.x>q.y ? float2(direction.x,0.0) : float2(0.0,direction.y);
}
)SKSL";
}
}
