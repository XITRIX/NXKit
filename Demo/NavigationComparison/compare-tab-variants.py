"""Compare native controller/explicit-width UITabBar captures and portable tabs.
Usage: compare-tab-variants.py NATIVE_JSON WIDTHS_JSON PORTABLE_JSON OUTPUT_JSON
"""
import json,sys
from pathlib import Path
native,widths,portable=(json.loads(Path(p).read_text()) for p in sys.argv[1:4])
checks=[];failures=[]
for row in portable['records']:
    full=next((r for r in native['records'] if r['viewport'][2]==row['width'] and r['count']==row['count'] and r['dark']==row['dark'] and r['selected']==0),None)
    reference=full or next(r for r in widths['records'] if r['width']==row['width'] and r['count']==row['count'])
    platter=next(v['frame'] for v in reference['views'] if 'ItemPlatterView' in v['class'])
    buttons=[v['frame'] for v in reference['views'] if v['class']=='_UITabButton'][:row['count']]
    lens=buttons[row['selected']]
    # The standalone oracle has an explicit bar at y=0; compare its horizontal
    # geometry and lens padding without claiming a controller-placement sample.
    got_platter=list(row['platter']);got_lens=list(row['lens'])
    if full:
        placement=abs(got_platter[1]-platter[1]);safe=abs(row['childBottom']-full['childSafe'][2])
    else:
        placement=None;safe=None
    lens_error=max(abs(a-b) for a,b in zip([got_lens[0],got_lens[1]-got_platter[1],*got_lens[2:]],[lens[0],lens[1]-platter[1],*lens[2:]]))
    platter_error=max(abs(got_platter[i]-platter[i]) for i in (0,2,3))
    passed=max(lens_error,platter_error,placement or 0,safe or 0)<.34
    name=f"{row['width']}-{row['count']}-{row['selected']}-{'dark' if row['dark'] else 'light'}"
    checks.append({'case':name,'source':'controller' if full else 'explicit-width bar','platterErrorPt':platter_error,'lensErrorPt':lens_error,'placementErrorPt':placement,'safeAreaErrorPt':safe,'passed':passed})
    if not passed:failures.append(name)
result={'nativeOS':native['os'],'checks':checks,'tolerancePt':.34,'failures':failures,'note':'390/402-point viewports use a real native controller in both themes. Other horizontal widths use a native UITabBar with explicit bounds; their controller placement is not a native measurement. Pixel/optical parity is outside these geometry checks.'}
Path(sys.argv[4]).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'cases':len(checks),'maxLensErrorPt':max(r['lensErrorPt'] for r in checks),'failures':failures}))
raise SystemExit(bool(failures))
