# Lens AIR findings (static shipped artifact, not original source)
Source QuartzCore default.metallib iOS27.0 24A434 SHA256 5d59e4e26569b90f3e7e6331be5cf8931aab96718306ee62a72862e4e5192073.
Extracted with installed Apple metal-objdump --metallib --disassemble; this ignores --disassemble-symbols and emits all modules, split by source_filename.

## glass_foreground_sdf_lpf.ll
SdfFragmentUniforms48 bytes + GlassForegroundUniforms80 bytes, total128.
Glass offsets:0 float4 displacement_mat;16 refraction_amount;20 inv_refraction_height;24 refraction_offset;28 aberration_amount;32 inv_aberration_height;36 aberration_offset;40 float2 aberration_dir;48 edge_start;52 edge_end;56 edge_opacity_start;60 edge_opacity_end;64 float2 refraction_dir;72 padding8.
Function receives separate SDF coordinate and source texture coordinate. SDF output=(distance,normalX,normalY,alpha). Do not assume texture normalY matches screen-positive-down; CPU sdf normal_mat transforms it.
Let M rows=(a,b),(c,d); direction=(u,v); n=(nx,ny).
R(M,dir,n)=(nx*(a*u+b*v)+ny*(b*u-a*v),nx*(c*u+d*v)+ny*(d*u-c*v)).
Refraction direction R(M,refraction_dir,n); aberration direction swapXY(R(M,aberration_dir,n)).
Shoulder(amount,offset,invHeight): t=clamp((-distance-offset)*invHeight,0,1); return amount*(1-sqrt(t*(2-t))). Both refraction and aberration use this independent shoulder.
center=sourceCoord + refractionDirection*refractionShoulder.
spread=aberrationDirection*aberrationShoulder.
Seven source reads: center+spread*t for t=1,2/3,1/3; center-spread*t for t=0,1/3,2/3,1. No independent5tap Gaussian here.
Each sample RGB is unpremultiplied by max(alpha,1e-6). Red accumulates red*t for first3; green accumulates green*(1-t) for all7; blue accumulates blue*t for last4. Normalize color by(.5,1/3,.5). Alpha=mean7 sampled alpha * SDFalpha * coverage; premultiply color by this alpha (and RGB edr_scale).
coverage=clamp(.5-distance/max(fwidth(distance),.0001),0,1). Return zero if SDFalpha<.001.
edgeT=clamp((distance-edge_start)/(edge_end-edge_start),0,1); output *=1-mix(edge_opacity_start,edge_opacity_end,edgeT). Linear edge fade, not smoothstep.

## displacement_map_lpf.ll
Uniform32bytes:float4 mat@0,float2 clamp@16,float2 offset@24.
mask=read(mask_texture,maskCoord); if mask.b<1/255=>transparent.
q=mask.rg-offset; delta=(dot(mat.xy,q),dot(mat.zw,q)); p=min(sourceCoord+delta,clamp).
result=source_texture(p)*mask.b, then RGB*=edr_scale. Mask alpha not used. Matrix/offset are CPU-prepared, so inputAmount alone insufficient to equate RG to vector without preparation evidence.

## SDF generation
sdf_gen_gradients_lpf computes normal from normalize(right-left,up-down) (Y reversed relative texture rowindex). Output=(u.x*distance+u.y,u.z*normal.x+u.w,u.z*normal.y+u.w,1).
SDF displacement FOUND inside generic fixed_frag_lpf_cpf.ll:7202 (UberShader sdf_glass_displacement), dispatched textureFunction66 (:3869,4263). CPU -[CASDFGlassDisplacementEffect configureLayer:transaction:] (:1c3b38270) writes effectType7, (height,clampedCurvature,angle,maskOffset). SDFNode::apply effect7 selects textureFunction66 and packsparams0=(height,curvature,effectOffset,maskOffset);params1.xy=(cos(angle),sin(angle)).
Let d=SDFdistance; z=-(d+effectOffset); w=max(fwidth(z-maskOffset),.0001); t=saturate(z/height). curved=sqrt(1-(1-t)^2); flat=1-(t<1?.292899996:0); strength=1-mix(flat,curved,curvature).
rotatedNormal=(nx*cos-ny*sin,nx*sin+ny*cos).
RG=rotatedNormal*strength*saturate((d+effectOffset+height)/w+.5).
B=saturate((z-maskOffset)/w+.5)*SDFalpha, forced0 if(z-maskOffset)<-5; A=1.
The second AA factor softens transition to zero displacement at the height endpoint. Native CA displacementMap samples these signed RG values (not .5-centered) and usesB as maskcoverage. Since curvature1=>strength=1-sqrt(t(2-t)), independently matches foreground shoulder; both lose all displacement at depth>=height.
CPU evidence in sdf-node-cpu.txt:556–570, foreground-cpu.txt:4–55. Native effect curvature clamped[0,1] beforerender.

## Half shader
lph shares shoulder/edge formula and80byte packing. It delegates7sample filter to aberrate_texture helper. Helper supports optional mip-prefilter (4samples around location), unpremultiply flags, alpha handling, but foreground caller uses false,false (see call site), so no active mip-prefilter in inspected default foreground entrypoint. Half arithmetic can affect bytes slightly.

## Foreground CPU preparation (foreground-cpu.txt)
Refraction: amount fromCAkey428; invHeight=1/getKey432; offset=getKey433 unchanged. Aberration: amount=key328;invHeight=1/getKey330; offset=key331 unchanged; angle=key329. Directions=(cos(angle),sin(angle)); refractionangle key429. No apparent .5/point-to-pixel gain on distance/height in packing; source UV displacement_mat incorporates currenttransform/source surface pixel dimensions and sampling scale.
Edge opacity values independently clamp[0,1] before packing. EdgeStart/End raw distances.
Displacement-map mat still requires CPU packing inspection before treating inputAmount as direct logical-pointgain for alltransforms.

## displacementMap CPU preparation (displacement-cpu.txt)
Actual inherited implementation is CA::OGL::SampleMapFilter::render at1c3991760, established by DisplacementMapFilter vtable.
Reads inputAmount key337 (default1) into d9; optional inputOffset key419 into float2 d10 (default0; scalar duplicates). No .5-centered default.
At1c39919b0–1c39919e4, forms displacement_mat from current2x2transform columns and multiplication inputAmount*sourceScale, divided peraxis by source texture width/height (duplicatedW,W,H,H). This converts logical displacement to normalizedtextureUV. When working directly in samepointspace and identitytransform, delta=amount*RG. Inverse transforms elsewhere map coordinates, do not duplicate scale inpointspace.
Clamp=(source_content_extent-.5)/source_texture_size (1c3991ac4–1c3991ae4). Offset exactly inputOffset packed unchanged.
Note native sourceSDFnormalY may be inverted by sdf normal_mat so preserve geometry-coordinate rules from sharedSDF pipeline.

## Gradient ovalization and normal matrix follow-up
fixed_frag_lpf_cpf.ll:9534 supercircle_sdf_image false variant, :9587 rect_sdf_image, and glass_foreground_sdf_lpf.ll:264 compute_sdf_with_mode establish the same rule:
- geomNormal is shape boundary normal.
- radial=normalize(float2(local.x,halfWidth*local.y/halfHeight)).
- normal=normalize(mix(geomNormal,radial,gradientOvalization)).
- transformedNormal=normal_mat*normal.
Thus gradientOvalization=.5 changes normalDIRECTION, not amplitude or distance. It cannot by itself account for observed half-gain.
CPU emit_sdf_bounds_internal in prior sdf-geometry-cpu.txt:81–89 initializes no-extra-transform normal matrix to(1,0,0,1). Transform path:194–296 divides basis terms bymax(columnLengths), then packs inverse/cofactor-like matrix; no universal.5. Packingproof lines397–415 map toshadernormal_mat. This does not establish the actual normal matrix/source scale of the current lens composition; nested/offscreen source coordinates can still change effective displacement. Need actual end-to-end pixel probes or captured uniforms, not a guessedunitnormal gain.
The earlier statement 'identity point-space delta=inputAmount*RG' concerns the displacement-map pass givenitsRGmask, and is notproof that RGequalsunitnormal in allcompositions.
