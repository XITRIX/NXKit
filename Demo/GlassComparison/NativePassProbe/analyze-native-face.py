"""Derive observed native chroma slopes without fitting a production equation."""
from pathlib import Path
import argparse,json,math
root=Path(__file__).resolve().parents[3];folder=root/'docs/liquid-glass-native-settings/color-transfer'
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');a=p.parse_args()
d=json.loads((folder/'centers.json').read_text());colors=[(255,255,0),(200,160,128),(0,255,0),(255,0,0),(128,128,128),(255,255,255)];records=[]
for size,index,maxLuma in [(52,0,.6),(120,1,.35)]:
 for row,color in enumerate(colors):
  luma=sum(x*y for x,y in zip(color,[.2126,.7152,.0722]))/255
  chroma=[x-255*luma for x in color];intercept=d['dark-saturation0'][row][index]
  numerator=denominator=0;individual={};components=[]
  for sat in [1,2]:
   observed=d['dark-saturation'+str(sat)][row][index];n=w=0
   for channel in range(3):
    # Clamped outputs do not reveal a chromatic slope.
    if 0<observed[channel]<255 and abs(chroma[channel])>1e-5:
     x=sat*chroma[channel];y=observed[channel]-intercept[channel]
     n+=x*y;w+=x*x;numerator+=x*y;denominator+=x*x;components.append([sat,channel])
   individual[str(sat)]=n/w if w else None
  records.append({'size':size,'sourceRGB':color,'rec709Luma':luma,'maxLumaSDR':maxLuma,
                  'saturation0RGB':intercept,'saturation1RGB':d['dark-saturation1'][row][index],
                  'saturation2RGB':d['dark-saturation2'][row][index],
                  'chromaFactorLeastSquares':numerator/denominator if denominator else None,
                  'chromaFactorsPerSaturation':individual,'unclampedComponentsUsed':components,
                  'oldRec709Factor':1+(maxLuma-1)*luma})
# Independently validate the equation recovered from shipped AIR, not a fit.
errors=[]
for label,black,white,saturation in [('dark-no-bleed',.125,1.125,1.3),('dark-black0',0,1.125,1.3),('dark-black025',.25,1.125,1.3),('dark-saturation0',.125,1.125,0),('dark-saturation0-white05',.125,.5,0),('dark-saturation1',.125,1.125,1),('dark-saturation2',.125,1.125,2)]:
 for size,index,maxLuma in [(52,0,.6),(120,1,.35)]:
  for row,color in enumerate(colors):
   rgb=[x/255 for x in color];luma=sum(x*y for x,y in zip(rgb,[.2126,.7152,.0722]));factor=min(max(1+(maxLuma-1)*luma,0),1)
   chromaGain=saturation*factor*(1+.3*(1-factor))
   output=[black+(white-black)*luma*factor+(x-luma)*chromaGain for x in rgb]
   predicted=[round(min(max(x,0),1)*255)for x in output]
   errors.extend(abs(x-y)for x,y in zip(predicted,d[label][row][index]))
assert len(errors)==252 and max(errors)<=1,'Shipped AIR face equation disagrees with independent native controls'
validation={'components':len(errors),'meanAbsoluteError':sum(errors)/len(errors),'maximumError':max(errors),'equationSource':'Shipped QuartzCore glass_background_minimal_lpf AIR; native-air-face.json provenance. No fitting against controls.'}
text=json.dumps({'derivedAIRFaceValidation':validation,'method':'Native saturation0 intercept; least-squares common multiplier on saturation*(sourceRGB-Rec709Luma), using only unclamped output components. No model fitting across colors or sizes.','observations':records},indent=2)+'\n';target=folder/'native-face-slopes.json'
if a.check:
 assert target.read_text()==text,'Derived native face observations are stale'
else:target.write_text(text)
for r in records:
 print(r['size'],r['sourceRGB'],'L',round(r['rec709Luma'],6),'nativeF',round(r['chromaFactorLeastSquares'],6)if r['chromaFactorLeastSquares']is not None else'undefined(gray)','oldF',round(r['oldRec709Factor'],6),'S1/S2',r['chromaFactorsPerSaturation'])
