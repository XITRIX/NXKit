# Switch Graphite Vulkan and shader compilation

The Switch backend uses native Skia Graphite Vulkan → Horizon NVK. It no longer
creates a Dawn instance, WebGPU device or EGL presentation surface. SDL
continues to own lifecycle, gamepad/touch input and window size. Vulkan uses
`VK_NN_vi_surface` with libnx's `NWindow` because this SDL Switch backend does
not implement an SDL Vulkan surface hook. Apple Metal backends are unchanged.

## Dependencies

From the repository root:

```sh
python3 scripts/switch/fetch_nvk.py
sh scripts/switch/build_skia.sh
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON
cmake --build Demo/build/switch --target ThorVGApp.nro -j8
```

The driver SDK is the checksummed `nvk-switch-mesa-25.0.7-r1` release from
[jhuz0/nvk-switch](https://github.com/jhuz0/nvk-switch), built from the
Horizon nxvk port. The archive SHA-256 is
`4ab244f982dc078aaff8c58640d7cf8a0f3ab43bbf2daadfd00c02fbf352af86`;
the library SHA-256 is
`ff65a74b46fe749f6be73eac3c8968d70e312c7646f815f26b69f4aa35737d26`.
It is installed only in this checkout, not into system portlibs. Its source
and packaging workflow are linked by the release repository. Required notices
are retained in `licenses/NVK-Switch-NOTICE.md` and copied into the package.
This is an experimental driver, not a claim of Vulkan conformance on Switch.

`build_skia.sh` idempotently applies the checked-in
`skia-builtin-effect-replay.patch` before compiling. It initializes builtin
runtime-effect metadata when a serialized key is decoded in a fresh process,
fixing the pinned Skia revision's b/238759147 TODO. The fix runs inside Skia;
NXKit does not depend on private Graphite object layouts.

Skia is built separately into `out/horizon-vulkan`, preserving the former
`out/horizon` archives for comparison. The CMake dependency paths are
`NXKIT_SWITCH_NVK_ROOT` and `NXKIT_SWITCH_SKIA_OUT`. SDL's Switch support still
links its existing GL/EGL support objects; NXKit does not use them to render.
No Dawn archive or WebGPU headers are required by the new backend.

## Presentation and memory

The renderer selects a graphics queue that also supports the VI surface,
enables only supported Skia-preferred features/extensions, checks the required
swapchain sampling/copy usages and uses FIFO presentation. Graphite waits on
the image-acquire semaphore and transitions the target to
`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` before signaling presentation. Render-finished
semaphores belong to swapchain images. Acquire semaphores are released only
when their recording completes. Submission failures stop the run loop rather
than presenting while waiting on an unsignaled semaphore.

Resize waits for Graphite and presentation before replacing the swapchain;
out-of-date/suboptimal images trigger recreation. Teardown drains recorder
work, waits for the Vulkan device, releases wrapped surfaces and swapchain,
then destroys recorder/context, VMA, device, surface and instance before the
homebrew runtime hands the process back to the launcher.

The pinned NVK build exposes one host-visible/coherent/cached memory type
(`0xe`) without the `DEVICE_LOCAL` bit. Skia's stock allocator rejects this
for GPU-only resources. The platform allocator uses Skia's existing VMA
implementation with locality as a preference and preserves allocation
alignment, mapping, protected-memory requirements and noncoherent
flush/invalidation metadata. Its Skia-derived allocation metadata handling is
attributed under `licenses/Skia-Vulkan-Allocator-LICENSE`.

## Shader compilation target

```sh
cmake -S Demo -B Demo/build/switch -DPLATFORM_SWITCH=ON \
    -DNXKIT_SWITCH_ADDRESS=192.168.1.148
export NXKIT_SWITCH_FTP_USER=your-ftp-user
export NXKIT_SWITCH_FTP_PASSWORD='your-ftp-password'
cmake --build Demo/build/switch --target ThorVGApp.switch-shaders -j8
```

The target builds an NRO, launches it using `--nxkit-shader-bake`, and renders
28 stages through the actual GPU path. These include every gallery entry,
changing glass group joins, clear/regular/tinted materials, circular/continuous
corners, blur/shadow/contour filters, lens variants and lifted/resting states,
the stress screen, Apple tabs, and text/XML content. The fixture exits through
normal teardown. The build retrieves its Vulkan cache and serialized Graphite
pipeline keys over FTP, verifies integrity and ABI, and embeds them in a
second NRO build. That NRO then reruns all stages with
`--nxkit-bundled-cache-only`, ignoring SD caches. Only after both hardware
passes succeed does the target publish the package. A missing completion or
presentation marker, empty native cache or cache-write failure fails the
target. Credentials stay in the environment and are not saved
in the CMake cache or package.

The package is `Demo/build/switch/switch-package/`, containing the NRO,
attribution, and shader manifest. Per-ABI bake data and the log remain under
`Demo/build/switch-shaders/<ABI>/`. Use the ordinary `.nro` target for development
when no Switch is available; it embeds any matching seed already present.

## Runtime behavior and limits

At context creation, Graphite loads a matching SD cache or its bundled seed.
It additionally verifies the Vulkan vendor/device/cache UUID. Startup
precompiles captured serializable Graphite keys plus common shape, image and
text pipelines before interaction begins. During rendering, callbacks collect
keys in bounded RAM only. Native cache and key files are written on orderly
exit using temporary files followed by rename. Since fsdev cannot rename over
an existing SD file, replacement preserves the previous destination until the
new file is installed, rolls back on failure, and reads the preserved backup
if a replacement was interrupted.

The cache envelope validates its kind, 64-character content ABI, size and
checksum. Shader implementation, app source, selected component kits, Skia
archive and driver changes invalidate the seed. Custom application shader
sources outside the scanned `app/` directory can be supplied through
`NXKIT_SWITCH_SHADER_SOURCES` in CMake. Invalid/truncated/stale files are ignored
and rebuilt; file size and key count limits bound memory use.

This is ahead-of-interaction compilation on the target GPU, not an assertion
that every possible Skia pipeline is host-compiled. Runtime effects with
unregistered/dynamic combinations cannot all be represented as serialized
Graphite keys. Their native NVK machine code can still be cached when the bake
exercises them, but Skia may still generate SkSL/SPIR-V and create a pipeline
object on later runs. Application-provided effects and unexercised group sizes,
formats, filters or drawing combinations can miss the cache. Extend the bake
fixture when adding such combinations. Zero stutter is not guaranteed.

## Verification

- Native Vulkan Skia archives and the complete Switch Demo build/link/package.
- Link command has NVK and no Dawn archive.
- Host cache-format regression checks round trips, stale ABIs, wrong kinds,
  corruption, truncation, trailing data and overflowing size declarations.
- Host SD-file replacement regression covers rename-without-overwrite, POSIX
  replacement, failed install/rollback, preserved recovery and stale backup.
- The C/C++ TLS configure/reconfigure regression passes for both languages.
- The display-link regression reproduces a heap use-after-free with the former
  implementation and passes with the fix under AddressSanitizer/UBSan.
- The iOS arm64 Debug Demo builds with signing disabled after the shared-loop
  changes.
- Hardware Vulkan initialization, frame presentation, all 28 bake stages,
  orderly exit and export of a 2,453,320-byte native cache pass. The retained
  `bake-cold.log` records this run.
- Fresh-process serialized-key replay exposed Skia's uninitialized builtin
  runtime-effect snippets (`ShaderCodeDictionary::getEntry`, b/238759147).
  A null snippet name throws `std::logic_error`; the abort returns through
  libnx and Sphaira then crashes during handoff, obscuring the original stack.
  `bundled-replay-crash-symbolicated.txt` recovers the retained NXKit frames.
  The checked-in Skia patch now initializes builtin snippets during lookup.
  An initial application-side workaround used an inline private accessor with
  a mismatched `GPU_TEST_UTILS` layout and froze at key 1; that workaround was
  removed. The corrected library replays all 51 keys and renders all 28 stages.
  This exposed fsdev rename-overwrite failures during cache persistence; the
  replacement transaction now has rollback/recovery regression coverage and
  passes the final cold and bundled-only hardware runs, including replacement
  of existing native-cache and key files. `bake-bundled.log` records all 51 key
  replays, first presentation, 28 stages, successful save and normal exit.

The manually renamed caches for ABI
`2db5c460dd748bc7e651c407658fd44f08b9ff062b902dee34be61ed9ad159e6`
were restored to `pipeline.cache` and `pipeline.keys` on SD. Bundled-only
validation now ignores SD caches without renaming files. The earlier unverified
package remains separately under `switch-package-unverified-2db5c460`; the
normal package path now contains the verified result of both hardware passes.
The manifest records its ABI, NRO SHA-256 and bundled-cache validation.

The verified NRO is installed at `/switch/ThorVGApp.nro`. FTP readback matches
SHA-256 `dcb7d5bc6bae5f5e36fc5da80853619b72035c449c62e373a1b4495f0892a0f0`.
The canonical SD file already matched the verified build, so no duplicate
backup was created. `deployment.json` records the check. A subsequent normal
Demo launch loaded 51 saved keys and presented at least 600 frames without a
reported rendering error; see `normal-launch.log`.

On 2026-11-02, the user confirmed that the normal Demo UI is visible and
responds to controls on the physical Switch with the verified Vulkan/NVK build
and baked cache. This confirms the smoke test; it is not a performance benchmark.
