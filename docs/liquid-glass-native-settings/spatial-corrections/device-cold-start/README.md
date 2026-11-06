# Physical-device launcher pipeline coverage

A fully cold launch on the iPhone15ProMax, iOS27.2/24B5110a, disabled both bundled and persisted read archives. The runtime exported35 complete Skia-generated MSL sources and27 native pipeline descriptors, with `exportFailed=false`. All35 sources already existed byte-for-byte in the current catalog. Two additional R8 stencil-only curve/winding descriptions were absent; they have no fragment shader.

An earlier incremental capture exported two new sources and two descriptions but did not establish complete launcher coverage. The subsequent complete cold run supersedes that coverage claim. The final catalog retains200 source libraries and151 pipeline descriptions, including every earlier description unchanged.

The framebuffer was observed for15 seconds. The console timeout ends the bounded observer and is not an application-correctness result. Final strict launcher/replay checks are recorded in the parent verification report. This does not establish coverage of every Demo interaction or long-term thermal behavior.

Only this project’s generated shader metadata is retained here. The source bytes remain under `shaders/apple/catalog/sources`; no Apple shader payload is copied.
