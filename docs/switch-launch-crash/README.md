# Switch launch crash, 2026-11-02

The latest Atmosphère report was retrieved from the Switch at `192.168.1.148`
over FTP. The original report, screenshot, deployed NRO header and symbolicated
stack are retained in this directory. The existing working tree had unrelated
tab/controller changes; those were preserved.

## Original failure: incorrect compiler TLS ABI

`01793642405_03db12780bd84000.log` identifies build
`0ab28aa7c1b1ce8d56aae7aac2a79b343d796927`, matching both the deployed NRO
header and the local ELF. The PC is `ThorVGApp + 0x12d3c0`, at the
`renderApplicationFrame()` reentrancy guard in `UIApplicationMain.cpp:16`.
The instruction reads the guard through `TPIDR_EL0`, which is zero:

```asm
mrs  x19, tpidr_el0
ldrb w0, [x19, x0]  // x0 = 0xf8
```

`Submodules/cmake/toolchain.cmake` set normal `CMAKE_C_FLAGS` and
`CMAKE_CXX_FLAGS` variables before `project()`. On reconfiguration these hid
devkitPro's cached platform defaults, including `-mtp=soft` and
`-ftls-model=local-exec`. Ten linked TLS accesses used the wrong register,
including rendering, corner geometry, and glass code.

The fix leaves compiler flags to devkitPro and supplies libnx/portlibs headers
through the standard system include-directory settings. These settings are
cached so first configuration and reconfiguration both retain the headers.
The rebuilt renderer calls `__aarch64_read_tp`, whose implementation reads
libnx's software thread pointer from the thread-local region. No direct
`TPIDR_EL0` accesses remain in the linked executable.

## Follow-up failure: invalid generated WGSL

The first hardware rerun passed the TLS guard but failed when compiling the
glass rim shader. `after-tls-nxlink.log` records a function argument named
`distance` while its generated body refers to undeclared `_0_distance`.
Dawn rejected the shader. Skia then dereferenced a null pipeline while trying
to report command submission failure (`QueueManager.cpp:206`). The follow-up
Atmosphère report and symbolicated stack are retained here; build ID
`e2039d604f07e5a9c41b503bd7508ad8ff5b1221` matches that rerun.

The glass shader parameter is now `rimDistance`, avoiding the builtin-name
collision without changing the shader arithmetic. This works around the
current WGSL generator issue; Skia's generic null-pipeline failure-handling
bug remains an upstream limitation. No Skia submodule files were changed.

## Reproduction and verification

```sh
python3 Demo/tests/SwitchToolchainTests.py
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build Demo/build/switch --target ThorVGApp.nro -j8
nxlink -a 192.168.1.148 -r 2 -s Demo/build/switch/ThorVGApp.nro
```

The regression harness creates a minimal project using the real toolchain,
configures and compiles it twice, and checks generated C and C++ TLS assembly
uses `__aarch64_read_tp` rather than `TPIDR_EL0`. Both stages pass. All 347
Demo C/C++ compilation commands contain the required TLS flags. The complete
Switch Demo builds and packages successfully with the existing Graphite/Dawn
archives.

The NRO with both fixes has build ID
`58d4676aba6454f095b8705f8bcbbe5a48f6fd35`, size 37,390,783 bytes and SHA-256
`794f00407e234df0ec2f5a95194bda4ded21d0161a2c27df62f55ad103fae8a9`.
It was uploaded and launched through nxlink; the hardware's stdout connection
remained established for several minutes. Visual confirmation, interaction
coverage and an exit/relaunch check are pending. An active nxlink connection
alone does not establish rendering fidelity or a completed frame count.

## Vulkan/NVK follow-up

The current backend is Graphite → Dawn/WebGPU → OpenGL ES → Mesa, not WebGL.
Native Graphite Vulkan APIs exist in this Skia checkout. A future optional
Graphite → Vulkan → NVK backend could avoid the WGSL path, but needs a Vulkan
Skia build, a Horizon driver, presentation integration, synchronization and
memory allocation. No backend migration was made as part of this crash fix.
The [mesa-switch port](https://github.com/danfromtico/mesa-switch) documents
loaderless static Vulkan support on Horizon. Hardware comparison is necessary
before deciding whether to replace the current backend.
