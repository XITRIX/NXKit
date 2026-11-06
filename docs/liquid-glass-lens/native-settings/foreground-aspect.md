# Foreground spectral displacement: matrix/swap order

Shipped QuartzCore AIR `glass_foreground_sdf_lpf.ll` (and texture-backed
`glass_foreground_lpf.ll`) forms rows of the packed float4M before swapping:

```text
M = [[a,b],[c,d]]
dir = (u,v)
normal = (nx,ny)
R(M,dir,normal) = (
  nx*(a*u+b*v) + ny*(b*u-a*v),
  nx*(c*u+d*v) + ny*(d*u-c*v))
refractionUV = R(M,refractionDir,normal)
aberrationUV = swapXY(R(M,aberrationDir,normal))
```

AIR values%79/%81 compute first/second rowdotnormal, then%80 inserts first
result into lane1 and%82 inserts second result into lane0. `swapXY` therefore
occurs after M's component-dependent UV normalization, not on the point-space
normal before M.

CPU `CA::OGL::GlassForegroundFilter::render`0x1c3995200–523c loads bound input
surfaceX/Y allocation dimensions atx20+0x30, converts integerW/H to float,
replicates(W,W,H,H), constructs current2x2transform in row order, multiplies
sourceScale, divides componentwise by(W,W,H,H), and retains the matrix unchanged
for the glass shader. There is no subsequent observed precompensation of the
aberration amount/direction for aspect ratio in this render function.

LetD=diag(W,H),T=current2x2transform,s=sourceScale. Then:

```text
M = D^-1 * (s*T)
A_uv = swap * D^-1 * s * R(T,dir,normal)
```

For an axis-aligned unitT, untransformed logical coordinates and identical
X/Y source scale, converting the resulting UV delta back to logical points gives:

```text
A_points = (W/H * Q.y, H/W * Q.x), Q=R(I,dir,normal)
```

Source/view Y orientation may additionally reverse signs; it does not cancel
these aspect factors. With square allocation the factors are identity. Source
allocation dimensions must not be replaced by the116x56 lens bounds without
runtime evidence. Backdrop capture margins, texture allocation padding, source
surface resolution and transforms can all make those dimensions different.
The source UV base coordinates are independent shader inputs; they do not
change the above delta algebra. End-to-end native allocated dimensions remain
unmeasured by this static audit.
