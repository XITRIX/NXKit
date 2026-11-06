#!/usr/bin/env python3
"""Export actual native Lens motion traces for the portable response replay."""
import json,csv,math,hashlib,argparse
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('native',type=Path,help='Directory containing motion-*/trace.json')
parser.add_argument('output',type=Path)
args=parser.parse_args()
root=args.native;out=args.output;out.mkdir(parents=True,exist_ok=True)
columns=['case','style','t','phase','contactX','contactY','velocityX','velocityY','inputCenterX','inputCenterY','inputWidth','inputHeight','nativeX','nativeY','nativeW','nativeH','nxX','nxY','nxW','nxH','nativeScaleX','nativeScaleY','nxScaleX','nxScaleY','nativeRotation','nxRotation','nativeDriftX','nativeDriftY','nxDriftX','nxDriftY','localWidth','localHeight','ancestorScaleX','ancestorScaleY']
meta={'comparison':'Retained native reference with rigid NX baseline columns; LensMotionTests replaces those columns using the production response. Caller geometry and native ancestor basis are inputs.','clock':'All frame and dispatch times relative to the first actual UITouch.timestamp, without fitted time offset. Raw hardware touch timestamps also retained in original traces.','phase':'-1 before dispatch; UIKit UITouchPhase 0 began,1 moved,3 ended,4 cancelled','cases':[]};rows=[]
for directory in sorted(root.glob('motion-*')):
 path=directory/'trace.json'
 if not path.exists():continue
 j=json.loads(path.read_text());name=j['configuration']['id'];
 if name not in ['slider-slow','slider-left-dark','slider-fast-right','slider-reversal','switch-slow','tabs-slow-fixed','tabs-reversal','slider-cancel']:continue
 schema=j['schema'];lenses=[s for s in schema if s['viewClass']=='_UILiquidLensView']
 def samples(s):return [(f['t'],r) for f in j['frames'] for r in f['layers'] if r[0]==s['id']]
 def variation(s):
  r=[r for _,r in samples(s)];return max(x[9] for x in r)-min(x[9] for x in r)+max(x[12] for x in r)-min(x[12] for x in r)
 lens=max(lenses,key=variation);frames=samples(lens);events=sorted(j['touchEvents'],key=lambda e:e['dispatchT']);idx=0;event=None;vel=(0,0);last=None
 for time,r in frames:
  while idx<len(events) and events[idx]['dispatchT']<=time:
   event=events[idx];idx+=1
   if last and event['phase']==1:
    dt=max((event['t']-last['t']),1/240);vel=tuple((event['point'][i]-last['point'][i])/dt for i in range(2))
   else:vel=(0,0)
   last=event
  ax=r[3]/(r[7]*r[9]);ay=r[4]/(r[8]*r[12]);cx=r[1]+r[3]*.5-r[13]*ax;cy=r[2]+r[4]*.5-r[14]*ay;w=r[7]*ax;h=r[8]*ay
  row=[name,'selection' if j['configuration'].get('target')=='tabs' else 'regular',time,event['phase'] if event else -1,*(event['point'] if event else ('','')),*vel,cx,cy,w,h,*r[1:5],cx-w*.5,cy-h*.5,w,h,r[9],r[12],1,1,math.atan2(r[10],r[9]),0,r[13],r[14],0,0,r[7],r[8],ax,ay];rows.append(row)
 meta['cases'].append({'case':name,'trace':str(path),'traceSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),'lensID':lens['id'],'path':lens['path'],'frames':len(frames),'events':len(events),'os':j['os'],'osBuild':j['osBuild'],'inputGeometry':'presentation local bounds, ancestor presentation scale, and stage center with the lens affine drift removed; caller owns these inputs','alignmentSeconds':0})
with (out/'native-reference.csv').open('w') as f:
 writer=csv.writer(f,lineterminator="\n");writer.writerow(columns);writer.writerows(rows)
(out/'native-reference-metadata.json').write_text(json.dumps(meta,indent=2));print(out,len(rows),'rows',len(meta['cases']),'cases')
