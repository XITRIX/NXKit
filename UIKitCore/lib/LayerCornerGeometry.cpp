#include "LayerCornerGeometry.hpp"

#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <numbers>

namespace NXKit::Internal {
namespace {
struct Corner {
    NXFloat radius, x, y, power;
};
bool valid(NXSize size) {
    return std::isfinite(size.width) && std::isfinite(size.height)
        && size.width > 0 && size.height > 0;
}
Corner parameters(NXSize size, NXFloat radius, CALayerCornerCurve curve) {
    const auto half = std::min(size.width, size.height) * 0.5f;
    radius = std::isfinite(radius) ? std::clamp(radius, 0.0f, half) : 0.0f;
    if (radius < 0.0001f) radius = 0;
    if (curve == CALayerCornerCurve::circular || radius == 0) return {radius, radius, radius, 2};
    auto amount = std::clamp((half / radius - 1.0f) / 0.4f, 0.0f, 1.0f);
    amount = amount * amount * (3.0f - 2.0f * amount);
    const auto extent = radius * (1.05f + 0.35f * amount);
    return {radius, std::min(size.width * 0.5f, extent),
            std::min(size.height * 0.5f, extent), 2.05f + 0.95f * amount};
}
SkPoint point(NXFloat angle, const Corner& c) {
    const auto power = 2.0f / c.power;
    const auto component = [power](NXFloat value) {
        return std::abs(value) < 0.000001f ? 0.0f
            : std::copysign(std::pow(std::abs(value), power), value);
    };
    return {component(std::cos(angle)) * c.x, component(std::sin(angle)) * c.y};
}
SkPoint tangent(SkPoint p, const Corner& c) {
    const auto gx = std::copysign(std::pow(std::abs(p.x() / c.x), c.power - 1), p.x()) / c.x;
    const auto gy = std::copysign(std::pow(std::abs(p.y() / c.y), c.power - 1), p.y()) / c.y;
    const auto length = std::hypot(gx, gy);
    return {-gy / length, gx / length};
}
}

static SkPath buildCornerPath(NXSize size, NXFloat radius, CALayerCornerCurve curve) {
    SkPathBuilder path;
    if (!valid(size)) return path.detach();
    const auto c = parameters(size, radius, curve);
    const auto rect = SkRect::MakeWH(size.width, size.height);
    if (c.radius == 0 || curve == CALayerCornerCurve::circular) {
        path.addRRect(SkRRect::MakeRectXY(rect, c.radius, c.radius));
        return path.detach();
    }
    const SkPoint centers[] = {{size.width - c.x, c.y}, {size.width - c.x, size.height - c.y},
                               {c.x, size.height - c.y}, {c.x, c.y}};
    constexpr auto quarter = std::numbers::pi_v<NXFloat> * 0.5f;
    constexpr int segments = 8;
    path.moveTo(size.width - c.x, 0);
    for (int corner = 0; corner < 4; ++corner) {
        const auto center = centers[corner];
        const auto startAngle = (corner - 1) * quarter;
        const auto start = point(startAngle, c);
        path.lineTo(center.x() + start.x(), center.y() + start.y());
        for (int segment = 0; segment < segments; ++segment) {
            const auto a0 = startAngle + quarter * segment / segments;
            const auto a1 = startAngle + quarter * (segment + 1) / segments;
            const auto p0 = point(a0, c), p1 = point(a1, c), mid = point((a0 + a1) * 0.5f, c);
            const auto t0 = tangent(p0, c), t1 = tangent(p1, c);
            const auto dx = (mid.x() - (p0.x() + p1.x()) * 0.5f) / 0.375f;
            const auto dy = (mid.y() - (p0.y() + p1.y()) * 0.5f) / 0.375f;
            const auto determinant = t1.x() * t0.y() - t0.x() * t1.y();
            const auto chord = std::hypot(p1.x() - p0.x(), p1.y() - p0.y());
            auto l0 = std::abs(determinant) > 0.000001f
                ? (t1.x() * dy - t1.y() * dx) / determinant : chord / 3;
            auto l1 = std::abs(determinant) > 0.000001f
                ? (t0.x() * dy - t0.y() * dx) / determinant : chord / 3;
            if (segment == 0) {
                // Both controls lie on the first straight edge: zero
                // curvature at the join, while preserving the exit tangent.
                const SkPoint normal = {t0.y(), -t0.x()};
                const auto divisor = t1.x() * normal.x() + t1.y() * normal.y();
                l1 = ((p1.x() - p0.x()) * normal.x() + (p1.y() - p0.y()) * normal.y()) / divisor;
                const auto c2 = SkPoint::Make(p1.x() - t1.x() * l1, p1.y() - t1.y() * l1);
                l0 = ((mid.x() - p0.x() * 0.5f - c2.x() * 0.375f - p1.x() * 0.125f) * t0.x()
                    + (mid.y() - p0.y() * 0.5f - c2.y() * 0.375f - p1.y() * 0.125f) * t0.y()) / 0.375f;
            } else if (segment == segments - 1) {
                const SkPoint normal = {t1.y(), -t1.x()};
                const auto divisor = t0.x() * normal.x() + t0.y() * normal.y();
                l0 = ((p1.x() - p0.x()) * normal.x() + (p1.y() - p0.y()) * normal.y()) / divisor;
                const auto c1 = SkPoint::Make(p0.x() + t0.x() * l0, p0.y() + t0.y() * l0);
                l1 = ((p0.x() * 0.125f + c1.x() * 0.375f + p1.x() * 0.5f - mid.x()) * t1.x()
                    + (p0.y() * 0.125f + c1.y() * 0.375f + p1.y() * 0.5f - mid.y()) * t1.y()) / 0.375f;
            }
            const auto clampX = [&](NXFloat x) { return center.x() + std::clamp(x, -c.x, c.x); };
            const auto clampY = [&](NXFloat y) { return center.y() + std::clamp(y, -c.y, c.y); };
            const auto a = std::clamp(l0, 0.0f, chord * 1.5f);
            const auto b = std::clamp(l1, 0.0f, chord * 1.5f);
            path.cubicTo(clampX(p0.x() + t0.x() * a), clampY(p0.y() + t0.y() * a),
                         clampX(p1.x() - t1.x() * b), clampY(p1.y() - t1.y() * b),
                         center.x() + p1.x(), center.y() + p1.y());
        }
    }
    path.close();
    return path.detach();
}

SkPath layerCornerPath(NXSize size, NXFloat radius, CALayerCornerCurve curve) {
    if (!valid(size)) return SkPath();
    radius = parameters(size, radius, curve).radius;
    struct Entry { NXSize size; NXFloat radius; CALayerCornerCurve curve; SkPath path; };
    // Geometry is shared by fill, border, clip, blur and glass, and by identical
    // controls. Keep a bounded per-render-thread cache rather than rebuilding
    // splines for every static layer on every frame.
    thread_local std::list<Entry> cache;
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it->size == size && it->radius == radius && it->curve == curve) {
            cache.splice(cache.begin(), cache, it);
            return cache.front().path;
        }
    }
    cache.push_front({size, radius, curve, buildCornerPath(size, radius, curve)});
    if (cache.size() > 128) cache.pop_back();
    return cache.front().path;
}

NXFloat layerCornerDistance(NXPoint p, NXSize size, NXFloat radius, CALayerCornerCurve curve) {
    if (!valid(size) || !std::isfinite(p.x) || !std::isfinite(p.y)) return std::numeric_limits<NXFloat>::infinity();
    const auto c = parameters(size, radius, curve);
    const auto dx = std::abs(p.x) - size.width * 0.5f;
    const auto dy = std::abs(p.y) - size.height * 0.5f;
    if (c.radius == 0) return std::max(dx, dy);
    if (curve == CALayerCornerCurve::circular) {
        return std::hypot(std::max(dx + c.radius, 0.0f), std::max(dy + c.radius, 0.0f))
            + std::min(std::max(dx + c.radius, dy + c.radius), 0.0f) - c.radius;
    }
    const auto x = dx + c.x, y = dy + c.y;
    if (x <= 0 || y <= 0) return std::max(dx, dy);
    const auto u = x / c.x, v = y / c.y;
    const auto norm = std::pow(std::pow(u, c.power) + std::pow(v, c.power), 1.0f / c.power);
    const auto gx = std::pow(u / norm, c.power - 1) / c.x;
    const auto gy = std::pow(v / norm, c.power - 1) / c.y;
    return (norm - 1) / std::hypot(gx, gy);
}

}
