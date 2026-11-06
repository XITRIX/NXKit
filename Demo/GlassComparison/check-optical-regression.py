"""Validate measured glass regressions against frozen native framebuffer controls.
Usage: check-optical-regression.py MAIN_METRICS HOLDOUT_METRICS PASTEL_METRICS
Generate metrics with compare-optical.py --baseline=BEFORE. No native runtime is
needed when checking the retained reports. Exterior shadows are not scored here.
"""
from pathlib import Path
import json,sys

main,holdouts,pastels=[json.loads(Path(p).read_text()) for p in sys.argv[1:4]]
checks=0

def require(condition,description):
 global checks
 checks+=1
 if not condition:raise AssertionError(description)

fields=['bodyMAE','rimMAE','refractionMAE']
def before(field):return 'before'+field[0].upper()+field[1:]
def mean(states,field):return sum(e[field] for s in states for e in s['elements'])/sum(len(s['elements']) for s in states)

for report,count in [(main,60),(holdouts,4),(pastels,6)]:
 require(len(report['states'])==count,'Incomplete native comparison series')
 require(all(len(s['elements'])==5 for s in report['states']),'Missing native size controls')
 require(len({s['id'] for s in report['states']})==count,'Duplicate scene IDs')
 require(all(s['dark'] in [0,1] for s in report['states']),'Unexpected appearance metadata')
 require({s['dark'] for s in report['states']}=={0,1},'Both appearances are required')
 require(not report['regionDefinition']['exteriorShadowScored'],'Material scores must exclude exterior shadows')

require({(s['background'],s['dark'],s['tint']) for s in main['states']}==
 {(bg,dark,tint) for bg in ['black','white','gray','red','green','blue','yellow','ramp','lines','stripes']
 for dark in [0,1] for tint in ['none','blue','coral']},'Main color/tint/theme coverage is incomplete')
require({(s['background'],s['dark']) for s in holdouts['states']}==
 {(bg,dark) for bg in ['waves60','waves240'] for dark in [0,1]},'Independent frequency coverage is incomplete')

# Uniform controls protect the calibrated body, tint and light in both themes.
# Tolerances accommodate only sub-channel averages from raster quantization.
solid=[s for s in main['states'] if s['background'] not in ['ramp','lines','stripes']]
require(len(solid)==42,'Missing primary/gray/black/white controls')
for states in [solid,pastels['states']]:
 for s in states:
  for e in s['elements']:
   for field in fields:
    require(abs(e[field]-e[before(field)])<.03,'Uniform or pastel response drifted: '+s['id'])

for field in fields:
 require(mean(main['states'],field)<mean(main['states'],before(field)), 'Full matrix did not improve '+field)

# The new colors were captured after choosing the kernel; each frequency and
# appearance must remain useful independently, not merely in an aggregate.
for s in holdouts['states']:
 for field in fields:
  require(mean([s],field)<mean([s],before(field))+.01,'Independent holdout regressed: '+s['id']+' '+field)

for dark in [0,1]:
 stripes=[s for s in main['states'] if s['background']=='stripes' and s['dark']==dark]
 for field in fields:
  require(mean(stripes,field)<mean(stripes,before(field))+.01,'Colored-wave appearance regressed')

print('Passed',checks,'measured optical checks across',sum(len(r['states']) for r in [main,holdouts,pastels]),'scenes')
