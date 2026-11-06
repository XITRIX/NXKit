"""Validate the rounded opaque regular-tint model against native solid controls.

Usage: python analyze-regular-tint.py NATIVE_CONTROLS.json OUTPUT.json
Native controls must already have embedded ICC converted to sRGB. This checks
the analytic transfer at flat body pixels; it is not a raster/optical test.
Yellow is a held-out background, not a held-out tint hue.
"""
import json
from pathlib import Path
import sys
import numpy as np

records = json.loads(Path(sys.argv[1]).read_text())
backgrounds = dict(black=[0,0,0], gray=[128/255]*3, white=[1,1,1],
                   red=[1,0,0], green=[0,1,0], blue=[0,0,1], yellow=[1,1,0])
tints = dict(blue=np.array([0,122,255])/255, coral=np.array([255,69,58])/255)
minimum_sizes = [52,80,120,56,160]

def linear(c):
    return np.where(c <= .04045, c/12.92, ((np.maximum(c,0)+.055)/1.055)**2.4)

def encoded(c):
    return np.where(c <= .0031308, c*12.92, 1.055*np.maximum(c,0)**(1/2.4)-.055)

def transfer(color, source, size, dark, current):
    luma = source @ np.array([.2126,.7152,.0722])
    red = max(color[0]-max(color[1],color[2]),0)
    green = max(color[1]-max(color[0],color[2]),0)
    if dark and not current:
        return color*(1-.02*luma)+(1-color)*.11*luma
    if dark:
        s = np.clip((size-52)/(120-52),0,1); s = s*s*(3-2*s)
        luma = np.clip(luma-.15*np.ptp(source)*4*luma*(1-luma),0,1)
        knee = .54 + (.24-.54)*s
        response = -np.expm1(-luma/knee)/(-np.expm1(-1/knee))*(1+(.61-1)*s)
        body = color-color*(1-color)*.02
        return np.clip(body*(1-.02*response)+(1-body)*(.12+.09*red)*response,0,1)
    gain = .395+(.50 if current else .265)*red-.045*green
    bias = -.014+(-.043 if current else -.025)*red+.022*green
    base = .4-.235*red if current else .4
    if current:
        luma = np.clip(luma-.263*np.ptp(source)*4*luma*(1-luma),0,1)**1.14
    amount = base+(1-base)*luma
    rgb = linear(color); shade = np.maximum(rgb*gain+bias,0)
    return np.clip(encoded(shade*(1-amount)+rgb*amount),0,1)

controls = []
for record in records:
    if record['background'] not in backgrounds or record['tint'] not in tints:
        continue
    for i, rgb in enumerate(record['RGB']):
        source = np.array(backgrounds[record['background']]); color = tints[record['tint']]
        before = transfer(color,source,minimum_sizes[i],record['dark'],False)*255
        after = transfer(color,source,minimum_sizes[i],record['dark'],True)*255
        native = np.array(rgb)
        controls.append(dict(id=record['id'], dark=record['dark'], tint=record['tint'],
            background=record['background'], sizeIndex=i, minDimension=minimum_sizes[i],
            heldOut=record['background']=='yellow', nativeRGB=rgb,
            previousRGB=before.tolist(), currentRGB=after.tolist(),
            previousMAE=float(abs(before-native).mean()), currentMAE=float(abs(after-native).mean())))
summary = []
for dark in [0,1]:
    for heldout in [False,True]:
        selected = [r for r in controls if r['dark']==dark and r['heldOut']==heldout]
        summary.append(dict(dark=dark, heldOutBackground=heldout, samples=len(selected),
            previousMAE=float(np.mean([r['previousMAE'] for r in selected])),
            currentMAE=float(np.mean([r['currentMAE'] for r in selected]))))
report = dict(method='Analytic flat-body transfer against ICC-normalized native controls. Equal weight per RGB channel, material, size and background. No raster fitting.',
    trainingBackgrounds=['black','gray','white','red','green','blue'], heldOutBackground='yellow',
    measuredTintRGB=dict(blue=[0,122,255],coral=[255,69,58]),
    summary=summary, controls=controls)
Path(sys.argv[2]).write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(summary,indent=2))
