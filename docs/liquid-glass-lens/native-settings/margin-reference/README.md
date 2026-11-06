# Native foreground capture-margin controls

Six native27.0 /24A434,2× captures vary only foreground
`CABackdropLayer.marginWidth` between0,50 and200. All use the same116×56-point
lifted selection lens, blue axial markers, gray32 and a matched empty-source
control. Default margin100 comes from the separate unit reference.

| Margin | Top G/B | Left G/B | Left blue inward from green |
| --- | ---: | ---: | ---: |
|0 |2.4414 |.6338 |.4430 |
|50 |1.7370 |.9248 |.2683 |
|100 baseline |1.5487 |1.0369 |.2257 |
|200 |1.3948 |1.1567 |.1877 |

[Metrics and provenance](metrics.json), requests, stage PNGs and compressed
metadata are retained here. Both direct and captured layer readbacks verify
the requested margin. Unchanged filter inputs were checked. The research helper
copies/reinstalls the foreground filter to invalidate the rendered node.

The angular response depends on the source capture context, consistent with
AIR swapping displacement components after source UV normalization. These
captures do not directly reveal source allocation dimensions, padding or an
exact native allocator rule. Any portable virtual extent remains an inference.

Run `unit-reference/check-holdouts.py` from this document's parent directory
through the repository command shown in the wide/phone reference reports.
