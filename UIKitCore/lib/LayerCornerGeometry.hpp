#pragma once

#include <CALayerCornerCurve.h>
#include <Geometry.h>
#include "include/core/SkPath.h"

namespace NXKit::Internal {

// A portable continuous-curvature approximation, calibrated with the native
// solid-layer fixture. UIKit's exact, private spline is not exposed by its API.
SkPath layerCornerPath(NXSize size, NXFloat radius, CALayerCornerCurve curve);
NXFloat layerCornerDistance(NXPoint centeredPoint, NXSize size, NXFloat radius, CALayerCornerCurve curve);

}
