"""Compare captured UIKit container geometry with the production raster tree.
Usage: python compare-container.py NATIVE_SERIES RASTER_DIRECTORY OUTPUT_DIRECTORY
Requires numpy and Pillow. Fails on connectivity, default-spacing or silhouette
regressions outside a documented one-point AA threshold; --strict removes
that tolerance. Optical channel errors are reported separately, not called parity.
"""
import json
from pathlib import Path
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont

series, rasters, output = map(Path,sys.argv[1:4])
strict = "--strict" in sys.argv[4:]
output.mkdir(parents=True,exist_ok=True)
variants = ['initial','near','far','large','dark','return']
results = []
images = {}
failures = []
boundary_differences = []
def mask(image):
    rgb = np.asarray(image,dtype=np.int16)
    # Tint detects the dark NXKit body; contrast detects UIKit's neutral
    # reflection joining across its optical seam. The gray elevation shadow
    # stays below 3 levels on this 31/255 backdrop. Neither test alone works
    # across both materials. Channel errors remain a separate optical metric.
    return ((rgb[:,:,0] - np.maximum(rgb[:,:,1],rgb[:,:,2])) > 12) | (np.max(np.abs(rgb - 31), axis=2) > 12)

def connected(binary, start, finish):
    # Four-connected flood fill between the centers of the two glass items.
    if not binary[start[1],start[0]] or not binary[finish[1],finish[0]]: return False
    seen = np.zeros_like(binary)
    todo = [start]
    seen[start[1],start[0]] = True
    while todo:
        x,y = todo.pop()
        if (x,y) == finish: return True
        for xx,yy in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
            if 0 <= xx < binary.shape[1] and 0 <= yy < binary.shape[0] and binary[yy,xx] and not seen[yy,xx]:
                seen[yy,xx] = True
                todo.append((xx,yy))
    return False

for variant in variants:
    meta = json.loads((series/variant/'capture.json').read_text())
    if meta['defaultSpacing'] != 0: failures.append(f'{variant}: UIKit default spacing changed')
    scale = meta['scale']
    x,y = meta['x']*scale,meta['y']*scale
    native = Image.open(series/variant/'screen.png').convert('RGB').crop((round(x),round(y),round(x+720*scale),round(y+760*scale)))
    raster = Image.open(rasters/f'{"initial" if variant=="return" else variant}.ppm').convert('RGB')
    if raster.size != native.size: raise RuntimeError('The reference requires the same 2x scale')
    images[variant] = (native,raster)
    request = meta['request']
    factor, offset = request['sizeScale'], request['gapOffset']
    for row, base_gap in enumerate(meta['gaps']):
        for column, spacing in enumerate(meta['spacings']):
            rect = (column*180*scale,row*150*scale,(column+1)*180*scale,(row+1)*150*scale)
            aa,bb = native.crop(rect),raster.crop(rect)
            a,b = mask(aa),mask(bb)
            size,gap = 60*factor,base_gap+offset
            left = (180-size*2-gap)/2
            start,finish = (round((left+size/2)*scale),75*scale),(round((left+size*1.5+gap)*scale),75*scale)
            ca,cb = connected(a,start,finish),connected(b,start,finish)
            iou = float(np.sum(a&b)/max(1,np.sum(a|b)))
            result = dict(variant=variant,spacing=spacing,gap=gap,size=size,nativeConnected=ca,nxkitConnected=cb,
                          silhouetteIoU=iou,meanAbsoluteChannelError=float(np.mean(np.abs(np.asarray(aa,dtype=float)-np.asarray(bb,dtype=float)))))
            results.append(result)
            if ca != cb:
                # A topology decision at the AA threshold is unstable within
                # one point across the captured UIKit versions. Keep it in the
                # report; --strict makes every such difference a failure.
                boundary = spacing > 0 and abs(gap-spacing*.5) <= 1
                if boundary: boundary_differences.append(result)
                if strict or not boundary: failures.append(f'{variant} spacing={spacing} gap={gap}: connectivity mismatch')
            if iou < .95: failures.append(f'{variant} spacing={spacing} gap={gap}: silhouette IoU {iou:.4f} < .95')
    # Rounded rectangles additionally exercise a broad overlap, where a
    # conventional smooth-min would incorrectly inflate parallel flat edges.
    for column in range(2):
        rect = (column*360*scale,610*scale,(column+1)*360*scale,750*scale)
        aa,bb = native.crop(rect),raster.crop(rect)
        a,b = mask(aa),mask(bb)
        start,finish = (90*scale,70*scale),((260 if column else 170)*scale,70*scale)
        ca,cb = connected(a,start,finish),connected(b,start,finish)
        iou = float(np.sum(a&b)/max(1,np.sum(a|b)))
        result = dict(variant=variant,shape='separatedRectangles' if column else 'overlappingRectangles',
                      spacing=20,gap=50 if column else -40,nativeConnected=ca,nxkitConnected=cb,
                      silhouetteIoU=iou,meanAbsoluteChannelError=float(np.mean(np.abs(np.asarray(aa,dtype=float)-np.asarray(bb,dtype=float)))))
        results.append(result)
        if ca != cb: failures.append(f'{variant}: rectangle connectivity mismatch')
        if iou < .95: failures.append(f'{variant}: rectangle silhouette IoU {iou:.4f} < .95')
# Returning retained native views to their initial geometry should restore the
# same topology and contour. This catches state left behind during resizing.
a,b = (mask(images[name][0]) for name in ['initial','return'])
history_iou = float(np.sum(a&b)/max(1,np.sum(a|b)))
if history_iou < .995: failures.append(f'Native return history changed silhouette: {history_iou}')
report = dict(nativeOS=meta['os'],results=results,nativeReturnIoU=history_iou,boundaryDifferences=boundary_differences,connectivityTolerancePoints=0 if strict else 1,failures=failures)
(output/'metrics.json').write_text(json.dumps(report,indent=2)+'\n')
font = ImageFont.load_default(size=18)
canvas = Image.new('RGB',(1440,830),'#202124')
draw = ImageDraw.Draw(canvas)
for column,(label,image) in enumerate(zip(['UIKit','NXKit'],images['initial'])):
    draw.text((column*720+12,12),label,fill='white',font=font)
    canvas.paste(image.resize((720,760)),(column*720,42))
canvas.save(output/'comparison.png')
for name,(native,raster) in images.items():
    if name in ['initial','large','dark']:
        native.save(output/f'{name}-native.png')
        raster.save(output/f'{name}-nxkit.png')
print(json.dumps(dict(cases=len(results),connectivityMatches=sum(r['nativeConnected']==r['nxkitConnected'] for r in results),
                     minimumSilhouetteIoU=min(r['silhouetteIoU'] for r in results),nativeReturnIoU=history_iou,boundaryDifferences=len(boundary_differences),failures=failures),indent=2))
if failures: sys.exit(1)
