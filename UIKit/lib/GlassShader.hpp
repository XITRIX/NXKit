#pragma once
#include <BackdropEffect.h>

namespace NXKit::Internal {
BackdropEffect makeGlassShaderEffect(NXFloat maximumSampleRadius, NXFloat backdropBlurRadius);
}
