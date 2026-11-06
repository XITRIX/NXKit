# Attribution

The `libvulkan.a` published here is a build of third-party open-source software.
It is not an original work; the packaging scripts are.

## Mesa 3D — NVK Vulkan driver and runtime

Copyright © Mesa contributors.
Licensed under the MIT License and other permissive licenses; see the
`docs/license.rst` and per-file headers in the Mesa source tree.

https://www.mesa3d.org/ — https://gitlab.freedesktop.org/mesa/mesa

The bulk of the archive is Mesa: NVK, NAK, NIL, NIR, SPIR-V/vtn, the Vulkan
runtime, `mesa_util`, BLAKE3 and `xmlconfig`.

## nxvk — Horizon port of NVK

Copyright © 2026 PalindromicBreadLoaf (palindromicbreadloaf@tuta.com)
SPDX-License-Identifier: MIT

https://github.com/PalindromicBreadLoaf/nxvk

Provides the `nvkmd/nvgpu` backend that talks to Horizon's nvhost/nvmap instead
of Linux DRM, the meson crossfiles, and the newlib gap fills (`nvk_compat.c`)
bundled into the archive.

## Expat

Copyright © 1998-2000 Thai Open Source Software Center Ltd and Clark Cooper
Copyright © 2001-2025 Expat maintainers
SPDX-License-Identifier: MIT

https://libexpat.github.io/

Bundled because Mesa's `xmlconfig` parses driconf with it. Taken from the
devkitPro `switch-libexpat` portlib.

## devkitPro / libnx

The toolchain, libnx and portlibs used to build this are devkitPro's.
libnx is licensed under the ISC license.

https://devkitpro.org/ — https://github.com/switchbrew/libnx

## Packaging scripts in this repository

Copyright © 2026 jhuz0
SPDX-License-Identifier: MIT

The workflow, the loaderless shim and the documentation. See LICENSE.

---

If you redistribute the binary, carry this file with it.
