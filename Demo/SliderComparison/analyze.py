"""Summarize retained real-touch UIKit samples without fitting a motion law."""
from pathlib import Path
import hashlib,json,sys
root=Path(sys.argv[1]).resolve()
results=[]
for directory in sorted(root.iterdir()):
    if not (directory/'frames.json').exists() or not (directory/'touches.json').exists():continue
    frames=json.loads((directory/'frames.json').read_text());touches=json.loads((directory/'touches.json').read_text());events=json.loads((directory/'events.json').read_text())
    if not frames or not touches:continue
    starts=[x for x in touches if x['phase']==0];ends=[x for x in touches if x['phase'] in [3,4]]
    if not starts or not ends:continue
    start,end=starts[0],ends[-1]
    active=[x for x in frames if x['t']<end['t'] and x['t']>=start['t'] and x['tracking']]
    actions=[x for x in events if x.get('kind')=='action'];changes=[x for x in actions if x.get('event')==4096]
    native_sizes=[x['lensFrame'][2:] for x in frames if x.get('lensFrame')]
    record={'case':directory.name,'initialValue':frames[0]['value'],'activeValueBeforeEnd':active[-1]['value'] if active else None,'finalValue':frames[-1]['value'],'touchStart':start['point'],'touchEnd':end['point'],'actualDuration':end['timestamp']-start['timestamp'],'beginAccepted':next((x['accepted'] for x in events if x.get('kind')=='begin'),None),'valueChangedCount':len(changes),'finalEvents':[{'event':x['event'],'value':x['value'],'tracking':x['tracking'],'highlighted':x['highlighted']} for x in actions[-3:]],'largestModelSize':[max(s[i] for s in native_sizes) for i in [0,1]],'sourceSHA256':{name:hashlib.sha256((directory/name).read_bytes()).hexdigest() for name in ['frames.json','touches.json','events.json']}}
    moves=[x for x in touches if x['phase']==1]
    record['terminalMoveAndEndSameTimestamp']=bool(moves and moves[-1]['timestamp']==end['timestamp'])
    if directory.name=='noncontinuous':record['measurementWarning']='Event-order-only reference: coarse/delayed touch delivery; final moved and ended share timestamp, with public value refresh after end dispatch. Exclude from velocity/projection fitting.'
    pans=[g for frame in frames if frame['t']<end['t'] for g in frame.get('gestures',[]) if g['class']=='UIPanGestureRecognizer' and g['state']==2]
    if pans:
        velocity=pans[-1]['velocity'][0];record['lastNativePanVelocityX']=velocity
        if active:
            # This oracle declares a 300pt-wide slider. This is measurement
            # normalization, not a production layout or velocity assumption.
            distance=(frames[-1]['value']-active[-1]['value'])*300
            record['releaseProjectionPoints']=distance;record['rate99ProjectionPoints']=velocity*.099
            record['rate99ProjectionErrorPoints']=distance-velocity*.099
    results.append(record)
print(json.dumps({'coverage':'Retained native output analysis only; no live UIKit execution or fitted force law','cases':results},indent=2))
