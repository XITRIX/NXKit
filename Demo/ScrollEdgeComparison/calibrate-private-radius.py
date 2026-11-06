"""Bounded empirical Gaussian fit to the native radius24 opaque-mask probe.
Usage: python3 calibrate-private-radius.py NATIVE_DIRECTORY
Pillow's Gaussian parameter is a fitting instrument, not Apple's API definition.
"""
from pathlib import Path
import json,sys
from PIL import Image,ImageFilter,ImageChops,ImageStat
root=Path(sys.argv[1]).resolve();reference=Image.open(root/'mask-one.png').convert('RGB');source=Image.open(root/'mask-zero.png').convert('RGB')
roi=(108,500,696,892);measurements=[]
for sigma in [12,16,20,24,28,32,36,40,42,44,46,48,56,64]:
 filtered=source.filter(ImageFilter.GaussianBlur(sigma*2));error=sum(ImageStat.Stat(ImageChops.difference(reference,filtered).crop(roi)).mean)/3
 measurements.append({'gaussianSigmaPoints':sigma,'gaussianSigmaPixels':sigma*2,'meanAbsoluteRGBDifference':error})
result={'scope':'One-radius optical fit, not a recovered native radius mapping or kernel guarantee.','nativeFactoryRadius':24,'captureScale':2,'source':'mask-zero.png','reference':'mask-one.png','roiPixels':roi,'roiPoints':[value/2 for value in roi],'measurements':measurements,'bestOfMeasured':min(measurements,key=lambda d:d['meanAbsoluteRGBDifference'])}
(root/'private-radius-calibration.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result['bestOfMeasured'],indent=2))
