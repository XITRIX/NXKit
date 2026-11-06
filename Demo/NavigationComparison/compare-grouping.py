"""Compare public UIKit observations with portable grouping regression output.

Usage: compare-grouping.py NATIVE_JSON_GZ PORTABLE_JSON OUTPUT_JSON
Native captures use explicit 20-point regular symbols. SF Symbol raster metrics
can differ between AppKit and UIKit; group decisions are exact, frames allow
1.1 pt. Custom UIButton intrinsic sizing belongs to UIButton's separate audit.
"""
import gzip,json,sys,math
from pathlib import Path
def read(path):
    with (gzip.open(path,'rt') if str(path).endswith('.gz') else open(path)) as f:return json.load(f)
def native_groups(state):
    return sorted([v['frame'] for v in state['views'] if 'UIPlatformGlassInteractionView' in v['class']],key=lambda r:r[0])
native,portable=map(read,sys.argv[1:3]);states={s['name']:s for s in native['states']}
checks=[]
for state in portable['states']:
    name=state['name'];a=native_groups(states[name]);b=sorted([g['frame'] for g in state['groups']],key=lambda r:r[0])
    error=max((abs(x-y) for ar,br in zip(a,b)for x,y in zip(ar,br)),default=0)
    # A generic custom view joins adjacent image items, but comparing UIButton
    # intrinsic sizing here would conflate a separate component implementation.
    frame_checked=not name.endswith('-custom')
    title=state['title'];native_title=next(v['frame']for v in states[name]['views']if v.get('text')=='Groups')
    title_error=max(abs(title[0]+title[2]/2-native_title[0]-native_title[2]/2),abs(title[1]+title[3]/2-native_title[1]-native_title[3]/2),abs(title[2]-native_title[2]))
    checks.append(dict(name=name,groupsMatch=len(a)==len(b),maximumFrameErrorPt=error,titleCenterWidthErrorPt=title_error,frameChecked=frame_checked,passed=len(a)==len(b)and(not frame_checked or error<=1.1)and title_error<=1.1))
transitions=[]
for motion in portable['motions']:
    name=motion['operation'];a=native_groups(states[name]);b=sorted(motion['frames'][-1]['groups'],key=lambda r:r[0])
    error=max((abs(x-y)for ar,br in zip(a,b)for x,y in zip(ar,br)),default=0)
    transitions.append(dict(name=name,groupsMatch=len(a)==len(b),endFrameErrorPt=error,passed=len(a)==len(b)and error<=1.1))
contacts=[]
directory=Path(sys.argv[1]).parent.parent
for name in ['pair','prominent','fixed-0','unshared']:
    trace=read(directory/('hold-'+name+'.json.gz'));frames=trace['frames']
    rest=next(f for f in frames if f['t']>.5)
    groups=lambda f:[v for v in f['views'] if 'UIPlatformGlassInteractionView' in v['class']]
    base=max(groups(rest),key=lambda v:v['frame'][0]);candidates=[(f,v)for f in frames for v in groups(f)if v['id']==base['id']]
    peak_frame,peak=max(candidates,key=lambda pair:pair[1]['presentation'][2])
    error=0;glyphs=0
    for glyph in peak_frame['views']:
        if not glyph['path'].startswith(peak['path']+'/')or 'ModernBarButton' not in glyph['class']:continue
        original=next(v for v in rest['views'] if v['id']==glyph['id'])
        a,b=original['presentation'],glyph['presentation'];r,p=base['presentation'],peak['presentation']
        expected=[p[0]+(a[0]-r[0])*p[2]/r[2],p[1]+(a[1]-r[1])*p[3]/r[3],a[2]*p[2]/r[2],a[3]*p[3]/r[3]]
        error=max(error,max(abs(x-y)for x,y in zip(expected,b)));glyphs+=1
    contacts.append(dict(name=name,actions=trace['actions'],glyphs=glyphs,maximumGlyphAffineErrorPt=error,passed=trace['actions']==1 and glyphs>0 and error<.001))
result=dict(nativeOS=native['os'],configurations=checks,transitions=transitions,nativeContacts=contacts,
    frameTolerancePt=1.1,failures=[x['name']for x in checks+transitions+contacts if not x['passed']],
    note='Exact group decisions; resting/end geometry permits AppKit/UIKit SF Symbol metric differences. Custom UIButton intrinsic frames are reported, not scored. Transition physics and material optics remain approximate; no frame-by-frame animation parity is claimed.')
Path(sys.argv[3]).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(dict(configurations=len(checks),transitions=len(transitions),nativeContacts=len(contacts),failures=result['failures'],maxScoredFrameErrorPt=max(x['maximumFrameErrorPt']for x in checks if x['frameChecked'])),indent=2))
sys.exit(bool(result['failures']))
