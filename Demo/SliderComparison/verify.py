"""Verify retained native artifacts, not live UIKit or the NXKit implementation."""
from pathlib import Path
import hashlib,json,sys
repo=Path(__file__).resolve().parents[2];native=repo/'docs/slider/native';checks=[]
for theme in ['light','dark']:
    capture=json.loads((native/'layer-geometry'/theme/'geometry.json').read_text());cases={row['id']:row for row in capture['cases']}
    for width in [100,200,300,440]:
        for height in [20,31,50,80]:
            case=cases[f'plain-{width}x{height}'];assert case['track']==[0,(height-6)/2,width,6];assert case['thumbAt0'][2:]==[37,24]
    assert cases['disabled']['hitTestClass']=='nil';assert cases['default']['hitTestSelf']
    assert cases['min-above-max']['minimum']==cases['min-above-max']['maximum']==cases['min-above-max']['value']==2
    assert cases['max-below-min']['minimum']==cases['max-below-min']['maximum']==cases['max-below-min']['value']==-2
    default=cases['default']['tree']['children'][0]['children'];background=default[0]['children'][0]['backgroundColor']
    assert background==([0,0,0,.1] if theme=='light' else [1,1,1,.1])
    white=default[1]['children'][0]['children'][0];assert abs(white['layer']['shadowOpacity']-.14)<1e-6;assert white['layer']['shadowRadius']==5;assert white['layer']['shadowOffset']==[0,1]
    checks.append(f'{theme}: 16 bounds, endpoint/range/disabled semantics, track alpha and resting shadow')
control_record=json.loads((native/'rea-control-api-evidence.json').read_text());api=json.loads(''.join(frame['data'] for frame in control_record['result']['frames']))
assert api['disabledState']=={'highlighted':True,'enabled':False,'state':7,'selected':True,'hitTestSelf':0}
assert api['disabledPrimaryActionCalls']==1;assert api['thumbImageNormalIdentity']==api['thumbImageDisabledIdentity']==api['thumbImageHighlightedIdentity']==1
checks.append('native UIControl selected/highlighted flags, disabled manual primary action and image fallback')
projection=[]
for group in ['velocity','low-velocity']:
    for case in json.loads((native/group/'summary.json').read_text())['cases']:
        if case['case']=='custom-thumb':assert case['largestModelSize']==[30,40]
        else:assert case['largestModelSize']==[57.35,37.2]
        assert abs(case['activeValueBeforeEnd']-(.25+(80-10)/300))<1e-6
        if 'lastNativePanVelocityX' in case:
            velocity=case['lastNativePanVelocityX'];distance=case['releaseProjectionPoints'];projection.append({'case':case['case'],'velocity':velocity,'distance':distance,'rate99Error':case['rate99ProjectionErrorPoints']})
            if abs(velocity)>100:assert abs(case['rate99ProjectionErrorPoints'])<.0002
            else:assert abs(distance)<1e-7
        for filename,digest in case['sourceSHA256'].items():assert hashlib.sha256((native/group/case['case']/filename).read_bytes()).hexdigest()==digest
checks.append('actual 80pt touch inputs: 10pt deadband/full-width mapping, custom image unchanged size, native release projections')
for row in json.loads((native/'srgb/manifest.json').read_text()):
    assert hashlib.sha256((native/row['source']).read_bytes()).hexdigest()==row['sourceSHA256']
    assert hashlib.sha256((native/'srgb'/row['output']).read_bytes()).hexdigest()==row['outputSHA256']
checks.append('nine P3→sRGB reference conversions retain source/output hashes')
result={'coverage':'Retained artifact verification only. Does not execute or observe native UIKit. Original XCTest logs/hashes retained separately.','checks':checks,'releaseProjection':projection,'unknowns':['Exact native pan velocity estimator','Projection cutoff only bracketed above60 through122pt/s','Private flex solver','Exact optical/pixel parity for all custom controls']}
text=json.dumps(result,indent=2)+'\n';print(text,end='')
if len(sys.argv)>1:Path(sys.argv[1]).write_text(text)
