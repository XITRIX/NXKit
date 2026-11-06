# Navigation glass spacing correction — 2026-11-03

The 44-point capsules already have the same 12-point layout gap as the native
reference. The defect was the navigation container's 20-point SDF blend distance:
its influence reached across that gap, pulling facing edges outward and leaving
the split neck connected after UIKit had already separated.

The navigation-only blend distance is now **12 points**. Layout and animation
trajectories are unchanged. The neck still exists during the overlapping part
of the split, but it is clear at the one-third-second checkpoint. At rest the
capsules retain their independent circular silhouettes.

![UIKit / previous 20-point blend / corrected 12-point blend](facing-contours.png)

![UIKit / previous / corrected](split-dark.gif)

[Full split filmstrip](split-dark.png). The previous source and comparison are
retained as `previous-UINavigationBar.cpp.gz`, `previous-source-manifest.json`
and `previous-comparison.json`; the parent motion report contains the refreshed
matrix for all arrangements.

`UIKitNavigationTests` now requires a clear gap at 1/3 second while keeping its
earlier 1/4-second neck assertion. It also compares both resting silhouettes
against the same capsules rendered with merging disabled, allowing at most one
raster pixel of contour difference at 3×. A clear center gap alone would not
have caught the facing-edge deformation.

The refreshed 20-clip matrix improves native connectivity agreement across the
six comparable dark join/split cases from 95.4% to 98.3%. All eight focused
regression suites and the macOS / unsigned iOS Demo builds pass.
