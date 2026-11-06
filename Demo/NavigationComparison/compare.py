"""Compare independently recorded UIKit/portable navigation results.
Usage: compare.py NATIVE_JSON PORTABLE_JSON OUTPUT_JSON
"""
import json,sys,gzip,math,bisect
from pathlib import Path

def read(path):
    p=Path(path)
    return json.loads(gzip.decompress(p.read_bytes()) if p.suffix=='.gz' else p.read_text())
native,portable=map(read,sys.argv[1:3]);failures=[];checks=[]
for case in portable['states']:
    ref=next(x for x in native['states'] if x['name']==case['name'])
    errors={key:max(abs(a-b) for a,b in zip(ref[key],case[key])) for key in ['bar','safe']}
    semantics=case['stack']==ref['stack'] and case['top']==ref['top']
    passed=semantics and max(errors.values())<.01
    checks.append({'case':case['name'],'stackMatches':semantics,'barFrameMaxErrorPt':errors['bar'],'safeAreaMaxErrorPt':errors['safe'],'passed':passed})
    if not passed:failures.append(case['name'])
sequence=native['events']==portable['events']
if not sequence:failures.append('lifecycle/delegate sequence')
# Read-only observed glass carrier rectangles; class names are data, not API calls.
# Compare settled phases, separate from claims about private optics/physics.
phases={'root':'launch','pushed':'push','editor':'push-prominent','popped':'pop','root-restored':'pop-root'}
geometry=[]
for case,op in phases.items():
    frames=[x for x in native['frames'] if x['operation']==op]
    frame=frames[-1]
    ref=sorted([v['barPresentation'] for v in frame['views'] if 'GlassInteractionView' in v['class']],key=lambda r:r[0])
    got=sorted(next(x['groups'] for x in portable['states'] if x['name']==case),key=lambda r:r[0])
    error=max((abs(a-b) for r,s in zip(ref,got) for a,b in zip(r,s)),default=0)
    passed=len(ref)==len(got) and error<=3
    geometry.append({'case':case,'nativeGroups':ref,'portableGroups':got,'maxErrorPt':error,'tolerancePt':3,'passed':passed})
    if not passed:failures.append(case+' glass geometry')
motion_report={}
for operation,sample_key,from_key,target in [('push','motion','motionFrom','pushed'),('pop-root','popMotion','popMotionFrom','root-restored')]:
    if not portable.get(sample_key):continue
    series=[]
    for frame in native['frames']:
        if frame['operation']!=operation:continue
        groups=sorted([v['barPresentation'] for v in frame['views'] if 'GlassInteractionView' in v['class']],key=lambda r:r[0])
        if len(groups)==2:series.append((frame['t'],groups))
    samples=portable[sample_key];times=[row['t'] for row in samples]
    native_from,native_to=series[0][1],series[-1][1]
    portable_from=portable[from_key];portable_to=next(x['groups'] for x in portable['states'] if x['name']==target)
    def value(rect,kind):return rect[2] if kind=='width' else rect[0]+rect[2]/2
    def interpolate(t,side,kind):
        if t<=0:return value(portable_from[side],kind)
        i=bisect.bisect_right(times,t)
        if i>=len(samples):return value(samples[-1]['groups'][side],kind)
        a,b=samples[i-1],samples[i];q=(t-a['t'])/(b['t']-a['t'])
        return value(a['groups'][side],kind)*(1-q)+value(b['groups'][side],kind)*q
    best=None
    for step in range(81):
        offset=step*.0025;errors={'width':[],'center':[]}
        for t,groups in series:
            for side in range(2):
                for kind in errors:
                    a,b=value(native_from[side],kind),value(native_to[side],kind)
                    c,d=value(portable_from[side],kind),value(portable_to[side],kind)
                    progress=(interpolate(t-series[0][0]-offset,side,kind)-c)/(d-c)
                    errors[kind].append(a+progress*(b-a)-value(groups[side],kind))
        score=sum(e*e for es in errors.values() for e in es)
        if best is None or score<best[0]:best=(score,offset,errors)
    _,offset,errors=best
    metrics={kind:{'rmsErrorPt':math.sqrt(sum(e*e for e in values)/len(values)),'maxErrorPt':max(abs(e) for e in values)} for kind,values in errors.items()}
    passed=metrics['width']['rmsErrorPt']<3 and metrics['center']['rmsErrorPt']<.8
    motion_report[operation]={'samples':len(series),'alignmentSeconds':offset,'metrics':metrics,'passed':passed,
        'note':'Horizontal carrier width/center are normalized to their own endpoints and aligned once at transition onset; alignment is not an input-latency measurement. Vertical strain and exact private optics remain approximate.'}
    if not passed:failures.append(operation+' horizontal capsule motion')
report={'nativeOS':native['os'],'stateChecks':checks,'lifecycleAndDelegateSequenceMatches':sequence,'eventCount':len(native['events']),'glassGeometry':geometry,'horizontalCapsuleMotion':motion_report,'failures':failures,
        'limitations':['Raster/Simulator comparison does not establish physical GPU parity.','Shared background motion is approximate; private optical and spring implementation is not reproduced.','Large-title/search/appearance configuration and interactive pop are not implemented.']}
Path(sys.argv[3]).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'states':len(checks),'events':len(native['events']),'failures':failures}))
raise SystemExit(bool(failures))
