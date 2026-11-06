# Skia upstream consolidation — 2026-11-03

Skia now has one Switch-support commit, `eb958eafdc`, directly on official
`main` revision `c733ad3e37` from <https://skia.googlesource.com/skia.git>.
The local branch is `codex/switch-support`; no remote history was rewritten.
The older `horizon` branch remains available.

The old checkout compared July 2026 sources against the 2024 Horizon branch,
which made thousands of upstream changes appear to be local edits. The exact
original source tree, including untracked source files and filename-case
artifacts, is retained in `codex/backup-skia-20261103-120207` at
`b02ccac313ff64669e0fd7ad9e83aabb2510e383`.

Recovery files, the original Git index, binary patch and reflog are in
`Submodules/skia/out/recovery/20261103-120207/`. The previous native Vulkan
build was moved into that directory as `previous-horizon-vulkan-build`.
Other ignored build outputs remain in place.

The three edited dependency repositories each retain their exact original
source tree on `codex/backup-skia-20261103`:

| Dependency | Recovery commit |
| --- | --- |
| abseil-cpp | `d0041856ad6194167e6334525e3594d48cd19386` |
| dawn | `bb346743c5a671fc14dd61c0ffd6a8f3db0a90da` |
| vulkan-utility-libraries | `2f2975294b09434e1af055e8440786d34aba19ad` |

The recovery directory also contains each dependency's original revision,
status, and binary patch. The original Vulkan-Utility-Libraries deletions are
preserved there; the new build uses the clean revision pinned by upstream.

The port preserves Horizon toolchain/portlib integration, POSIX adaptations,
SkSL and strike-cache pthread storage, and serialized Graphite runtime-effect
initialization. It adapts the executor changes to upstream's new implementation
file, adds the correct libnx software TLS compiler flags, and fixes counted
semaphore signaling. The old Dawn/OpenGL ES compatibility changes remain as
patches regenerated against the new `DEPS` versions. Upstream already removed
the `strnlen` call that required the old Dawn string-helper workaround.

The submodule provides `tools/build-horizon.sh` and `README.horizon.md`.
NXKit's Switch build wrapper delegates to this helper, so rebuilding does not
modify Skia source files. `DEVKITPRO` must point to a complete SDK installation;
verification used `/opt/devkitpro`.

Verification:

- All seven native Vulkan/text/ICU archives build from latest upstream.
- NXKit renderer, layer, text and pipeline-cache headers compile against it.
- The Switch Demo links and packages an NRO with the new archives.
- Regenerated Dawn and Abseil patches pass both clean-application and reverse-application checks against
  their new pinned revisions. The optional legacy Dawn backend was not built.
- Runtime and shader-cache validation on the physical Switch remain pending
  for this upgraded Skia revision; its old cache ABI is invalidated automatically.
- Matching iOS arm64 and macOS arm64 Metal/Graphite, paragraph, shaping, Unicode
  and ICU archives now build from this revision. The iOS Xcode Debug Demo
  (unsigned) and macOS CMake Demo both compile/link. Fresh Core/navigation/tab
  regressions pass 3/3 using the new macOS archives and current headers.
  Apple GN builds set `skia_use_partition_alloc=false` to match NXKit's ordinary
  pointer/allocator configuration; release and trivial-ABI settings match the
  consumers. Previous iOS archives and arguments are preserved in
  `out/recovery/navigation-apple-20261103`. See [apple-build.json](apple-build.json)
  for SDKs, arguments and artifact hashes. Device execution is not claimed.

See the retained build logs and `manifest.json` for exact revisions and results.
