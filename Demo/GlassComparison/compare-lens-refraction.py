"""Color-managed, fixed-geometry before/after lens comparison.

Usage: compare-lens-refraction.py RUN BEFORE_RENDERER AFTER_RENDERER OUTPUT [--only=id,...] [--validation-only] [--cache=PATH]
RUN contains native/{id}.json, native/{id}.png, sourceAssets and LensReference.app.
Geometry and source asset names are read from the captured native metadata.
"""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile
import numpy as np
from PIL import Image,ImageCms,ImageFilter
from comparison_images import load_srgb,COLOR_MANAGEMENT

run,before,after,out=(Path(s).resolve() for s in sys.argv[1:5]);out.mkdir(parents=True,exist_ok=True)
only=next((set(s.split('=',1)[1].split(',')) for s in sys.argv if s.startswith('--only=')),None)
native=run/'native';assets=out/'images';assets.mkdir(exist_ok=True)
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
yy,xx=np.mgrid[:1120,:1440];xx=(xx+.5)/2;yy=(yy+.5)/2
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def path_asset(value):
    if isinstance(value,dict):value=value.get('path',value.get('filename'))
    p=Path(value)
    for candidate in [p,native/p,run/p,native/'source-assets'/p,native/'content'/p,run/'LensReference.app'/p]:
        if candidate.is_file():return candidate.resolve()
    raise FileNotFoundError(value)
def sdf(frame):
    x,y,w,h=frame;r=min(w,h)/2;qx=abs(xx-x-w/2)-(w/2-r);qy=abs(yy-y-h/2)-(h/2-r)
    return np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-r
def source_alpha(path,frame):
    rgba=Image.open(path).convert('RGBA');alpha=np.asarray(rgba.getchannel('A'))
    sx,sy,sw,sh=frame
    if alpha.shape!=(round(sh*2),round(sw*2)):raise ValueError(('Source dimensions',path,alpha.shape,frame))
    canvas=np.zeros((1120,1440),np.uint8);left,top=round((40+sx)*2),round((250+sy)*2)
    x0,x1=max(0,left),min(1440,left+alpha.shape[1]);y0,y1=max(0,top),min(1120,top+alpha.shape[0]);canvas[y0:y1,x0:x1]=alpha[y0-top:y1-top,x0-left:x1-left]
    return canvas
def edge_masks(frame):
    # Partition by the dominant normalized axis, including curved end caps.
    x,y,w,h=frame;vx=(xx-x-w/2)/(w/2);vy=(yy-y-h/2)/(h/2);vertical=abs(vy)>=abs(vx)
    band=(sdf(frame)<-.5)&(sdf(frame)>-18)
    return {k:band&m for k,m in dict(top=vertical&(vy<0),bottom=vertical&(vy>=0),left=~vertical&(vx<0),right=~vertical&(vx>=0)).items()}
def stats(error,mask):
    values=error[mask]
    return dict(pixels=int(mask.sum()),mae=float(values.mean()) if len(values) else None,p95=float(np.percentile(values,95)) if len(values) else None)
def signature(record):
    # Exact nuisance-state matching for ink-removed optical controls.
    request=record['request']
    defaults=dict(dark=0,background='gray',base='regular',style=1,lifted=True,contentMode=1,warps=True,liftContainer='content',sourceHostAlpha=1)
    return json.dumps(dict(frame=record['frame'],**{k:request.get(k,v) for k,v in defaults.items()}),sort_keys=True)
def category(request):
    value='validation'
    if request.get('contentMode',1)!=1 or not request.get('warps',True) or request.get('liftContainer','content')!='content':value='private configuration diagnostic'
    if request.get('contentSetup','portal').startswith('source-') or request.get('contentSetup')=='none':value='control'
    if request.get('hideLens'):value='private configuration diagnostic'
    if request.get('probePattern','').startswith('scan-'):value='line correspondence profile'
    return value

records=[]
for file in sorted(native.glob('*.json')):
    if file.name in ['series.json']:continue
    m=json.loads(file.read_text())
    if not isinstance(m,dict):continue
    r=m.get('request',{});name=r.get('id',file.stem)
    if r.get('probe')!='edges' or only and name not in only:continue
    manual=next(x for x in m['hierarchy'] if x.get('identifier')=='manual-lens')
    source=m.get('sourceAssets',{});sf=m.get('sourceFrameInBar',[-64,-64,768,204]);frame=manual['frame'];base=r.get('base','regular')
    normal=path_asset(source['normal']);selected=path_asset(source['selected']);background=path_asset(r.get('background','gray')+'.png')
    records.append(dict(name=name,request=r,metadata=m,frame=frame,sourceFrame=sf,normal=normal,selected=selected,background=background,base={'none':0,'regular':1,'clear':2}[base]))
if not records:raise ValueError('No edge probe captures found')
evidence_counts={value:sum(category(q['request'])==value for q in records) for value in sorted(set(category(q['request']) for q in records))}
if '--validation-only' in sys.argv:
    lookup={q['name']:q for q in records};all_controls={signature(q):q for q in records if q['request'].get('contentSetup')=='none'}
    selected=[q for q in records if category(q['request'])=='validation']
    line_checks=['scan-y-12','scan-y-18','scan-y-20','scan-style0-y-22','scan-x-270','scan-style0-x-274']
    selected.extend(lookup[name] for name in line_checks if name in lookup)
    names={q['name'] for q in selected}
    for q in selected:
        control=lookup.get(q['request'].get('controlID')) or all_controls.get(signature(q))
        if control:names.add(control['name'])
        elif q['request'].get('lifted',True):raise ValueError(('Missing matched control',q['name']))
    records=[q for q in records if q['name'] in names]
(out/'selection.json').write_text(json.dumps(dict(ids=[q['name'] for q in records],nativeEvidenceCounts=evidence_counts),indent=2)+'\n')
temp=tempfile.TemporaryDirectory(prefix='nxkit-lens-refraction-');raw=Path(temp.name)
cache=next((Path(s.split('=',1)[1]).resolve() for s in sys.argv if s.startswith('--cache=')),raw/'cache')
rendered={}
for label,renderer in [('before',before),('after',after)]:
    rows=[];pending=[];rendered[label]={};folder=cache/sha(renderer);folder.mkdir(parents=True,exist_ok=True)
    for q in records:
        r=q['request'];fingerprint=hashlib.sha256(json.dumps(dict(request=r,frame=q['frame'],sourceFrame=q['sourceFrame'],inputs=[sha(q[k]) for k in ['background','normal','selected']]),sort_keys=True).encode()).hexdigest()
        cached=folder/(q['name']+'-'+fingerprint[:16]+'.png');rendered[label][q['name']]=cached
        if cached.is_file():continue
        ppm=raw/(q['name']+'-'+label+'.ppm');fields=[str(ppm),str(q['background']),str(q['normal']),str(q['selected'])]
        line=' '.join(json.dumps(s) for s in fields)
        line+=' '+' '.join([str(int(bool(r.get('dark')))),str(r.get('style',1)),str(int(bool(r.get('lifted',True)))),json.dumps(r.get('contentSetup','portal')),str(r.get('contentMode',1)),str(int(bool(r.get('warps',True)))),str(q['base']),str(int(bool(r.get('hideLens',False))))]+[str(v) for v in q['frame']+q['sourceFrame']]+[str(r.get('sourceHostAlpha',1))])
        rows.append(line)
        pending.append((ppm,cached))
    if rows:
        manifest=raw/(label+'.txt');manifest.write_text('\n'.join(rows)+'\n');subprocess.run([str(renderer),str(manifest)],check=True,stdout=subprocess.DEVNULL)
        for ppm,cached in pending:load_srgb(ppm).save(cached,icc_profile=profile);ppm.unlink()
native_images={q['name']:np.asarray(load_srgb(native/(q['name']+'.png'))) for q in records}
controls={signature(q):q for q in records if q['request'].get('contentSetup')=='none'}
by_name={q['name']:q for q in records}
results=[]
for q in records:
    r=q['request'];name=q['name'];a=native_images[name].astype(float);images={label:np.asarray(load_srgb(rendered[label][name]),float) for label in ['before','after']}
    alpha=source_alpha(q['selected'] if r.get('contentSetup','portal')!='underlay' else q['normal'],q['sourceFrame'])
    ink=alpha>=192;support=np.asarray(Image.fromarray(alpha).filter(ImageFilter.MaxFilter(5)))>8
    d=sdf(q['frame']);inside=d<-.5;edges=edge_masks(q['frame']);control=by_name.get(r.get('controlID')) or controls.get(signature(q));copy=np.zeros_like(inside);signal=None
    if control and control['name']!=name:
        signal=np.max(abs(a-native_images[control['name']].astype(float)),axis=2)
        # Native-observed ink absent from source support is the copy target.
        # The threshold is explicit; masks are never derived from NXKit output.
        copy=inside&~support&(signal>12)
    masks={'mainInk':inside&ink,'centerInk':(d<-18)&ink,'edgeCopies':copy,'nonInkInterior':inside&~support}
    masks.update({edge+'MainInk':mask&ink for edge,mask in edges.items()});masks.update({edge+'Copies':mask&copy for edge,mask in edges.items()})
    result=dict(name=name,category=category(r),request=r,frame=q['frame'],sourceFrame=q['sourceFrame'],copyControl=control['name'] if control else None,metrics={},files={})
    # Display only the manual composition; fixed stage y186..390 keeps64ptoverscan.
    crop=(0,372,1440,780)
    def save(label,array):
        file=assets/(name+'-'+label+'.png');Image.fromarray(array.astype('uint8')).crop(crop).save(file,icc_profile=profile);result['files'][label]='images/'+file.name
    save('native',a)
    for label,b in images.items():
        error=abs(a-b);result['metrics'][label]={k:stats(error,mask) for k,mask in masks.items()};save(label,b);save(label+'Difference',error.clip(0,255))
        if control and control['name']!=name:
            native_delta=a-native_images[control['name']].astype(float)
            nx_delta=b-np.asarray(load_srgb(rendered[label][control['name']]),float)
            for key,mask in masks.items():
                if not key.endswith('Copies') or not mask.any():continue
                result['metrics'][label][key]['inkSignalMAE']=float(abs(native_delta-nx_delta)[mask].mean())
                result['metrics'][label][key]['copyEnergyRatio']=float(abs(nx_delta)[mask].sum()/abs(native_delta)[mask].sum())
        for threshold,suffix in [(0,'Exact'),(2,'Within2')]:save(label+suffix,np.concatenate([b,np.where(error.max(2)>threshold,255,0)[...,None]],axis=2))
    if r.get('probePattern')=='markers' and control:
        # Sparse unique markers make source→copy displacement measurable in2D.
        # This is a centroid measurement, never an image-registration transform.
        native_delta=a-native_images[control['name']].astype(float)
        deltas={'native':native_delta,**{label:b-np.asarray(load_srgb(rendered[label][control['name']]),float) for label,b in images.items()}}
        control_images={'native':native_images[control['name']].astype(float),**{label:np.asarray(load_srgb(rendered[label][control['name']]),float) for label in images}}
        measurements=[]
        for i,point in enumerate(q['metadata']['featureGeometry']['markerPoints']):
            px,py=40+point[0],250+point[1]
            roi=inside&~support&(abs(xx-px)<24)&(abs(yy-py)<24)
            # Allocate overlapping neighborhoods by declared source geometry,
            # never by the rendered output or a fitted marker translation.
            distance=(xx-px)**2+(yy-py)**2
            for other in q['metadata']['featureGeometry']['markerPoints']:
                if other==point:continue
                roi&=distance<=(xx-40-other[0])**2+(yy-250-other[1])**2
            row=dict(marker=i,sourceCenter=[px,py],windowHalfExtent=24,centroids={},sourceToCopyDisplacement={},channelCentroids={},channelSourceToCopyDisplacement={})
            for label,delta in deltas.items():
                weight=np.maximum(np.max(abs(delta),axis=2)-12,0)*roi
                total=weight.sum()
                centroid=[float((xx*weight).sum()/total),float((yy*weight).sum()/total)] if total>100 else None
                row['centroids'][label]=centroid
                row['sourceToCopyDisplacement'][label]=[centroid[0]-px,centroid[1]-py] if centroid else None
                row['channelCentroids'][label]={};row['channelSourceToCopyDisplacement'][label]={}
                for channel,index in [('R',0),('G',1),('B',2)]:
                    # Per-channel coverage removes differing source/background
                    # contrast without fitting colors or moving output pixels.
                    denominator=q['metadata']['featureGeometry']['markerRGB'][i][index]-control_images[label][...,index]
                    coverage=np.divide(delta[...,index],denominator,out=np.zeros_like(denominator),where=abs(denominator)>=16)
                    weight=np.maximum(coverage-.1,0)*roi;total=weight.sum()
                    centroid=[float((xx*weight).sum()/total),float((yy*weight).sum()/total)] if total>1 else None
                    row['channelCentroids'][label][channel]=centroid
                    row['channelSourceToCopyDisplacement'][label][channel]=[centroid[0]-px,centroid[1]-py] if centroid else None
            if row['centroids']['native']:
                row['vectorErrorPoints']={label:float(np.linalg.norm(np.asarray(row['centroids'][label])-row['centroids']['native'])) if row['centroids'][label] else None for label in ['before','after']}
            row['channelVectorErrorPoints']={label:{channel:float(np.linalg.norm(np.asarray(row['channelCentroids'][label][channel])-row['channelCentroids']['native'][channel])) if row['channelCentroids'][label][channel] and row['channelCentroids']['native'][channel] else None for channel in ['R','G','B']} for label in ['before','after']}
            measurements.append(row)
        result['markerCopyDisplacements']=measurements
    if r.get('probePattern')=='map':
        # R/G encode source x/y; B is constant128. Remove a common white/black
        # lighting term with that constant channel before inverting the ramps.
        # The resulting apparent sample centroid combines refraction/reflection;
        # it is not claimed to identify separate physical ray paths.
        fields={}
        sx,sy,sw,sh=q['sourceFrame']
        def decode(image):
            blue=image[...,2];white=np.maximum((blue-128)/127,0);black=np.minimum(blue/128,1)
            denom=np.maximum((1-white)*black,.01)
            rg=(image[...,:2]-white[...,None]*255)/denom[...,None]
            return np.stack([40+sx+.25+rg[...,0]/255*(sw-.5)-xx,250+sy+.25+rg[...,1]/255*(sh-.5)-yy],axis=2)
        fields['native']=decode(a)
        fields.update({label:decode(b) for label,b in images.items()})
        result['apparentSampleDisplacement']={'method':'R/G ramp inversion after constant-B common-light correction; combines ray paths; horizontal quantization≈3pt, vertical≈.8pt','edges':{}}
        for edge,mask in edges.items():
            result['apparentSampleDisplacement']['edges'][edge]={label:dict(meanVector=[float(v) for v in field[mask].mean(0)],vectorErrorPoints=float(np.linalg.norm(field-fields['native'],axis=2)[mask].mean()) if label!='native' else 0) for label,field in fields.items()}
    if r.get('probePattern','').startswith('scan-') and control:
        axis=r['probePattern'][-1];fx,fy,fw,fh=q['frame']
        if axis=='y':
            start,end=int(np.floor(fy*2)),int(np.ceil((fy+fh)*2));fixed=round((fx+fw/2)*2);origin=250
            extract=lambda image:image[start:end,fixed]
        else:
            start,end=int(np.floor(fx*2)),int(np.ceil((fx+fw)*2));fixed=round((fy+fh/2)*2);origin=40
            extract=lambda image:image[fixed,start:end]
        signals={'native':extract(a-native_images[control['name']].astype(float)),**{label:extract(b-np.asarray(load_srgb(rendered[label][control['name']]),float)) for label,b in images.items()}}
        correspondence=dict(axis=axis,sourceCoordinate=r['scanCoordinate'],sourceWidthPoints=1,outputPixelCenters=[float(v) for v in (np.arange(start,end)+.5)/2-origin],profiles={})
        for label,signal in signals.items():
            change=abs(signal).mean(1);active=np.flatnonzero(change>10);groups=np.split(active,np.where(np.diff(active)>1)[0]+1);intervals=[]
            for group in groups:
                if len(group):intervals.append([(start+int(group[0]))/2-origin,(start+int(group[-1])+1)/2-origin])
            native_support=abs(signals['native']).mean(1)>10;predicted_support=change>10;union=native_support|predicted_support
            correspondence['profiles'][label]=dict(supportIntervals=intervals,meanRGBChange=[float(v) for v in change],signedRGBChange=signal.tolist(),inkSignalMAE=float(abs(signal-signals['native']).mean()) if label!='native' else 0,supportIoU=float((native_support&predicted_support).sum()/union.sum()) if union.any() else 1,missingSupportPixels=int((native_support&~predicted_support).sum()),extraSupportPixels=int((~native_support&predicted_support).sum()))
        result['lineCorrespondence']=correspondence
    mask_image=np.zeros_like(a);mask_image[masks['mainInk']]=[50,180,255];mask_image[copy]=[255,70,40];save('scoringMasks',mask_image)
    results.append(result)
results.sort(key=lambda r:(r['name']!='pilot-portal',r['category']!='validation',r['name']))
aggregate={}
for label in ['before','after']:
    aggregate[label]={}
    for key in results[0]['metrics'][label]:
        rows=[r['metrics'][label][key] for r in results if r['metrics'][label][key]['pixels'] and r['category']=='validation']
        aggregate[label][key]=dict(sceneMeanMAE=float(np.mean([r['mae'] for r in rows])) if rows else None,scoredScenes=len(rows),pixels=sum(r['pixels'] for r in rows))
        for field in ['inkSignalMAE','copyEnergyRatio']:
            values=[r[field] for r in rows if field in r]
            if values:aggregate[label][key][field]=float(np.mean(values))
summary=dict(runtime='iPadOS27.0build24A434',captureCount=len(results),alignment='Native hierarchy frames at2×; fixed stage crop; no fitted pixel/color transforms',colorManagement=COLOR_MANAGEMENT,copyMask='native ink minus exact no-ink control >12/255, excluding source alpha dilated by1pt; per-edge18ptbands',aggregate=aggregate,states=results)
summary['categoryCounts']={category:sum(r['category']==category for r in results) for category in sorted(set(r['category'] for r in results))}
summary['nativeEvidenceCounts']=evidence_counts
summary['markerCopyDisplacements']={}
for label in ['before','after']:
    rows=[m for r in results if r['category']=='validation' for m in r.get('markerCopyDisplacements',[]) if m['centroids']['native']]
    errors=[m['vectorErrorPoints'][label] for m in rows if m['vectorErrorPoints'][label] is not None]
    summary['markerCopyDisplacements'][label]=dict(nativePositiveMarkers=len(rows),matchedMarkers=len(errors),missingMarkers=len(rows)-len(errors),meanVectorErrorPoints=float(np.mean(errors)) if errors else None)
    channels={}
    for channel in ['R','G','B']:
        channel_rows=[m for r in results if r['category']=='validation' for m in r.get('markerCopyDisplacements',[]) if m['channelCentroids']['native'][channel]]
        values=[m['channelVectorErrorPoints'][label][channel] for m in channel_rows if m['channelVectorErrorPoints'][label][channel] is not None]
        channels[channel]=dict(nativePositiveMarkers=len(channel_rows),matchedMarkers=len(values),missingMarkers=len(channel_rows)-len(values),meanVectorErrorPoints=float(np.mean(values)) if values else None)
    summary['markerCopyDisplacements'][label]['channels']=channels
summary['lineCorrespondence']={}
for label in ['before','after']:
    rows=[r['lineCorrespondence']['profiles'][label] for r in results if 'lineCorrespondence' in r]
    summary['lineCorrespondence'][label]=dict(scenes=len(rows),meanInkSignalMAE=float(np.mean([r['inkSignalMAE'] for r in rows])) if rows else None,meanSupportIoU=float(np.mean([r['supportIoU'] for r in rows])) if rows else None,missingSupportPixels=sum(r['missingSupportPixels'] for r in rows),extraSupportPixels=sum(r['extraSupportPixels'] for r in rows))
summary['groups']={}
selectors={'themes':lambda r:'dark' if r['request'].get('dark') else 'light','styles':lambda r:'regular' if r['request'].get('style',1)==0 else 'selection','backgrounds':lambda r:r['request'].get('background','gray'),'sizes':lambda r:f"{r['frame'][2]}×{r['frame'][3]}"}
for grouping,selector in selectors.items():
    groups={}
    for value in sorted(set(selector(r) for r in results if r['category']=='validation')):
        selected=[r for r in results if r['category']=='validation' and selector(r)==value]
        group=dict(scenes=len(selected),metrics={})
        for label in ['before','after']:
            group['metrics'][label]={}
            for key in ['mainInk','centerInk','edgeCopies','topCopies','bottomCopies','leftCopies','rightCopies','nonInkInterior']:
                values=[r['metrics'][label][key]['mae'] for r in selected if r['metrics'][label][key]['pixels']]
                group['metrics'][label][key]=float(np.mean(values)) if values else None
        groups[value]=group
    summary['groups'][grouping]=groups
(out/'metrics.json').write_text(json.dumps(summary,indent=2)+'\n')
(out/'provenance.json').write_text(json.dumps(dict(renderers={label:json.loads((p.parent/'build-provenance.json').read_text()) for label,p in [('before',before),('after',after)]},captures={q['name']:sha(native/(q['name']+'.png')) for q in records},sources={str(p):sha(p) for q in records for p in [q['normal'],q['selected'],q['background']]},nativeFixtureHashes=sorted(set(q['metadata'].get('nativeFixtureSourceSHA256','unknown') for q in records))),indent=2)+'\n')
html='''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lens edge refraction</title><style>body{font:15px system-ui;background:#181818;color:#eee;margin:24px}p{max-width:900px}label{display:inline-block;margin:6px}select{font:inherit}#stage{position:relative;background:repeating-conic-gradient(#555 0 25%,#444 0 50%)50%/16px 16px}img{position:absolute;inset:0}a{color:#9cf}</style><h1>Lens edge refraction · UIKit / NXKit</h1><p>Original ink and refracted copies are measured separately at all four edges. Native geometry and identical RGBA source assets determine alignment; images are never fitted. Blue masks identify original source ink; red masks identify native copies outside that support.</p><label>Scene <select id="scene" aria-label="Scene"></select></label><label>Revision <select id="revision" aria-label="Revision"><option>after</option><option>before</option></select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay</option><option value="native">UIKit</option><option value="nxkit">NXKit</option><option value="Exact">Unmatched exact</option><option value="Within2">Unmatched within2/255</option><option value="Difference">Absolute difference</option><option value="scoringMasks">Scoring masks</option></select></label><label>Opacity <input id="opacity" aria-label="Overlay opacity" type="range" min="0" max="100" value="50"></label><label>Zoom <select id="zoom" aria-label="Zoom"><option value="closeup">Lens close-up</option><option value="fit">Fit stage</option><option value="pixels">Actual pixels</option></select></label><div id="stage"><img id="base"><img id="top"></div><pre id="scores"></pre><p><a href="metrics.json">Full metrics</a> · <a href="provenance.json">Provenance</a> · <a href="README.md">Method and limits</a></p><script>const cases=DATA;const e=id=>document.getElementById(id),scene=e('scene'),revision=e('revision'),mode=e('mode'),opacity=e('opacity'),zoom=e('zoom'),base=e('base'),topImage=e('top'),stage=e('stage');cases.forEach((r,i)=>{let o=document.createElement('option');o.value=i;o.textContent=r.name;scene.appendChild(o)});function update(){const r=cases[scene.value],v=revision.value,m=mode.value,key=m==='overlay'?'native':m==='nxkit'?v:['native','scoringMasks'].includes(m)?m:v+m;base.src=r.files[key];topImage.src=r.files[v];topImage.style.display=m==='overlay'?'block':'none';topImage.style.opacity=opacity.value/100;let w=zoom.value==='pixels'?1440:Math.min(720,innerWidth-48),scale=w/720,h=204*scale,left=0,top=0;if(zoom.value==='closeup'){const f=r.frame,x0=Math.max(0,Math.floor((f[0]-12)*2)/2),y0=Math.max(0,Math.floor((f[1]-186-12)*2)/2),x1=Math.min(720,Math.ceil((f[0]+f[2]+12)*2)/2),y1=Math.min(204,Math.ceil((f[1]-186+f[3]+12)*2)/2);scale=w/(x1-x0);h=(y1-y0)*scale;left=-x0*scale;top=-y0*scale}stage.style.width=w+'px';stage.style.height=h+'px';stage.style.overflow='hidden';for(const im of[base,topImage]){im.style.inset='auto';im.style.left=left+'px';im.style.top=top+'px';im.style.width=720*scale+'px';im.style.height=204*scale+'px'}e('scores').textContent=JSON.stringify(r.metrics[v],null,2)}for(const el of[scene,revision,mode,zoom])el.onchange=update;opacity.oninput=update;addEventListener('resize',update);update();</script>'''.replace('DATA',json.dumps(results).replace('</','<\\/'))
(out/'comparison.html').write_text(html);print(json.dumps(aggregate,indent=2));temp.cleanup()
