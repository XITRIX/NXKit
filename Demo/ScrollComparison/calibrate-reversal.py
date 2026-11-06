"""Calibrate the observed inward-return approximation; no spatial registration.
Usage: calibrate-reversal.py NATIVE_DIRECTORY OUTPUT_JSON
The remaining captures are validation, not calibration inputs.
"""
from pathlib import Path
import json,sys,math
import numpy as np
native=Path(sys.argv[1]);output=Path(sys.argv[2]);rate=-1000*math.log(.998)
def measurement(name):
 d=json.loads((native/(name+'.json')).read_text());edge=d['configuration']['offset'];start=d['events'][-1]['deliveredAt'];p=d['events'][-2]['sample'][2]
 velocity=next(e['velocity'][1]*1000 for e in d['delegates'] if e['name']=='willEnd')
 frames=np.array([f for f in d['frames'] if start+.35<f[0]<start+.85 and f[12]]);t=frames[:,0]-start
 basis=np.stack([np.ones_like(t),np.exp(-rate*t)],axis=1);coeff=np.linalg.lstsq(basis,frames[:,2]-edge,rcond=None)[0]
 return {'case':name,'displacement':p-edge,'releaseVelocity':velocity,'crossingVelocity':float(rate*coeff[0]),'freeFlightFitRMSE':float(np.mean((basis@coeff-(frames[:,2]-edge))**2)**.5)}
def crossing(row,omega,damping):
 decay=damping*.5;frequency=math.sqrt(omega*omega-decay*decay);d=row['displacement'];b=(row['releaseVelocity']+decay*d)/frequency
 angle=math.atan2(-d,b)
 if angle<=0:angle+=math.pi
 return math.exp(-decay*angle/frequency)*frequency*(-d*math.sin(angle)+b*math.cos(angle))
shallow=[measurement('scroll-reverse-'+side+'-outside') for side in ['top','bottom']];best=None
for omega in np.arange(8,24.001,.1):
 for damping in np.arange(10,20.001,.05):
  if damping>=2*omega:continue
  loss=sum((crossing(row,omega,damping)-row['crossingVelocity'])**2 for row in shallow)
  if best is None or loss<best[0]:best=(loss,float(omega),float(damping))
# Hold the rounded shallow coefficients fixed while fitting remaining stretch
# versus incoming momentum. Bottom deep and dense profiles remain validation.
omega=13.2;base=15.5;deep=[measurement('scroll-reverse-top-'+name) for name in ['deep','dense-deep']];ramp=None
for gain in np.arange(3,6.001,.05):
 for knee in np.arange(.5,1.001,.01):
  loss=0
  for row in deep:
   ratio=omega*abs(row['displacement']/row['releaseVelocity']);damping=min(2*omega*.999,base+gain*max(0,ratio-knee))
   loss+=(crossing(row,omega,damping)-row['crossingVelocity'])**2
  if ramp is None or loss<ramp[0]:ramp=(loss,float(gain),float(knee))
result={'measurement':'Public UIKit release state and free-flight samples after crossing. No fitted position shift.','shallowTraining':shallow,'deepTraining':deep,'bestShallowFit':{'omega':best[1],'damping':best[2],'crossingVelocitySquaredError':best[0]},'bestRampFit':{'gain':ramp[1],'knee':ramp[2],'crossingVelocitySquaredError':ramp[0]},'implemented':{'omega':13.2,'baseDamping':15.5,'returnTimeRatioGain':4.25,'returnTimeRatioKnee':.75,'dampingCapRatio':.999},'remainingValidation':'Bottom deep, bottom dense-deep, top/bottom dense-outside, slow and already-crossed controls. This is an observed approximation, not UIKit private implementation code.'}
output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'shallow':result['bestShallowFit'],'ramp':result['bestRampFit']}))
