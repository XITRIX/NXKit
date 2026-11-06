"""Compare native iOS26 and portable scroll-edge snapshots without claiming parity.

Usage: python3 compare-portable.py NATIVE_DIR PORTABLE_DIR OUTPUT_DIR
Requires Pillow. Inputs are matching NAME.png files and states.json[.gz].
Optional profile/{light,dark}-{soft,hidden}.png inputs allow independent optical
fits; actual effect frames are scored only when present in the portable trace.
"""
from __future__ import annotations
import gzip,hashlib,json,math,os,subprocess,sys
from pathlib import Path
from PIL import Image,ImageChops,ImageDraw,ImageStat

ALLOW_PARTIAL='--allow-partial' in sys.argv
EDGES=('top','left','bottom','right')
WIDTH,HEIGHT,SCALE=402,874,2
ROWS=(0,20,40,60,80,100,106,116,126,136,146,156,166,700,720,740,760,780,791,810,840,860)

def states_at(directory):
    for name in ('states.json','states.json.gz'):
        path=directory/name
        if path.exists():
            raw=gzip.decompress(path.read_bytes()) if path.suffix=='.gz' else path.read_bytes()
            try:rows=json.loads(raw)
            except json.JSONDecodeError:
                if not ALLOW_PARTIAL:raise
                text=raw.decode();rows=[];position=text.find('[')+1;decoder=json.JSONDecoder()
                while position>0 and position<len(text):
                    while position<len(text) and text[position] in ' \r\n\t,':position+=1
                    if position>=len(text) or text[position]==']':break
                    try:value,end=decoder.raw_decode(text,position)
                    except json.JSONDecodeError:break
                    rows.append(value);position=end
            if isinstance(rows,dict):rows=rows.get('states',rows.get('cases',[]))
            return {row['name']:row for row in rows},path
    raise FileNotFoundError(f'Missing states.json or states.json.gz in {directory}')

def image_at(directory,name):
    path=directory/f'{name}.png'
    if not path.exists():return None
    image=Image.open(path).convert('RGB')
    if image.size!=(WIDTH*SCALE,HEIGHT*SCALE):
        raise ValueError(f'{path}: expected {(WIDTH*SCALE,HEIGHT*SCALE)}, got {image.size}; do not silently rescale optical metrics')
    return image

def absolute_rgb(a,b,rect):
    box=tuple(round(x*SCALE) for x in rect)
    return sum(ImageStat.Stat(ImageChops.difference(a.crop(box),b.crop(box))).mean)/3

def rect(value):
    if isinstance(value,(list,tuple)) and len(value)==4:return [float(v) for v in value]
    if isinstance(value,dict):
        if all(k in value for k in ('x','y','width','height')):return [float(value[k]) for k in ('x','y','width','height')]
        if 'origin' in value and 'size' in value:return [float(value['origin'][k]) for k in ('x','y')]+[float(value['size'][k]) for k in ('width','height')]
    return None

def edge_for_frame(frame):
    x,y,w,h=frame;epsilon=1
    if abs(w-WIDTH)<epsilon and h<HEIGHT*.45:
        if abs(y)<epsilon:return 'top'
        if abs(y+h-HEIGHT)<epsilon:return 'bottom'
    if abs(h-HEIGHT)<epsilon and w<WIDTH*.45:
        if abs(x)<epsilon:return 'left'
        if abs(x+w-WIDTH)<epsilon:return 'right'
    return None

def native_carriers(state):
    result={}
    for view in state.get('views',[]):
        if 'BackdropView' not in view.get('class','') or view.get('hidden'):continue
        frame=rect(view.get('windowFrame'))
        if frame is None:continue
        edge=edge_for_frame(frame)
        if edge is None:continue
        # Native carrier and its alpha portal repeat a frame; retain the actual
        # carrier rather than treating duplicate representation as extra effects.
        if edge not in result or view.get('alpha',0)>result[edge]['alpha']:
            result[edge]={'frame':frame,'alpha':float(view.get('alpha',0))}
    return result

def portable_carriers(state):
    result={}
    rows=state.get('edges',[])
    if isinstance(rows,dict):rows=[dict(value,edge=key) for key,value in rows.items()]
    for index,value in enumerate(rows):
        edge=value.get('edge',EDGES[index] if index<4 else None)
        if isinstance(edge,int):edge=EDGES[index] if index<4 else None
        if edge not in EDGES:continue
        frame=None
        for key in ('carrierFrame','effectFrame','overlayFrame','windowFrame','frame','bandRect'):
            frame=rect(value.get(key))
            if frame is not None:break
        if frame is not None and frame[2]>0 and frame[3]>0:result[edge]={'frame':frame,'visible':value.get('visible'),'strength':value.get('strength'),'frameProvenance':value.get('bandRectSource') or ('derived band' if value.get('derived') else 'actual trace frame')}
    # A renderer may instead expose an explicit hierarchy record.
    for value in state.get('effectViews',[]):
        frame=rect(value.get('windowFrame',value.get('frame')))
        if frame is not None:
            edge=value.get('edge') or edge_for_frame(frame)
            if edge in EDGES:result[edge]={'frame':frame,'visible':value.get('visible')}
    return result

def geometry_for(native,portable):
    n=native_carriers(native);p=portable_carriers(portable);rows=[]
    for index,edge in enumerate(EDGES):
        row={'edge':edge,'native':n.get(edge),'portable':p.get(edge)}
        if edge in n and edge in p:
            delta=[p[edge]['frame'][i]-n[edge]['frame'][i] for i in range(4)]
            row.update(deltaPoints=delta,rmsPoints=math.sqrt(sum(x*x for x in delta)/4))
        else:row['comparison']='No nonempty portable band or no native carrier; inactive portable bands are not scored against dormant native carrier objects.'
        raw=portable.get('edges',[])
        if isinstance(raw,list) and index<len(raw):row['portableState']=raw[index]
        rows.append(row)
    return rows

def chrome_excluded(x,y):
    # Avoid the native/portable bars' independent symbol, title and glass pixels.
    return (62<=y<=116 and (145<=x<=255 or 325<=x<=395)) or (770<=y<=860 and 95<=x<=307)

def profile_pair(directory,theme):
    a=image_at(directory/'profile',f'{theme}-soft')
    b=image_at(directory/'profile',f'{theme}-hidden')
    if a is not None and b is not None:return a,b,'profile paired soft/hidden'
    if theme=='light':
        a=image_at(directory,'light-soft-middle');b=image_at(directory,'hidden')
        if a is not None and b is not None:return a,b,'main light-soft-middle/hidden'
    return None

def profile_fit(a,b):
    result=[]
    for y in ROWS:
        points=[x for x in range(48,390,48) if not chrome_excluded(x,y)]
        av=[a.getpixel((x*SCALE,y*SCALE)) for x in points];bv=[b.getpixel((x*SCALE,y*SCALE)) for x in points]
        channels=[]
        for channel in range(3):
            observed=[v[channel] for v in av];reference=[v[channel] for v in bv]
            om=sum(observed)/len(observed);rm=sum(reference)/len(reference)
            denominator=sum((x-rm)**2 for x in reference)
            slope=sum((x-rm)*(z-om) for x,z in zip(reference,observed))/denominator if denominator else 0
            channels.append({'slope':slope,'intercept':om-slope*rm,'nativeOrPortableMean':om,'hiddenMean':rm})
        difference=sum(abs(z-x) for a0,b0 in zip(av,bv) for z,x in zip(a0,b0))/(len(points)*3)
        result.append({'y':y,'sampleX':points,'samplePlacement':'48pt-grid cell centers at contentOffset.x=120; chrome samples excluded','meanAbsoluteRGBDifference':difference,'rgb':channels})
    return result

def pair_picture(native,portable,name):
    w,h=WIDTH,HEIGHT
    result=Image.new('RGB',(w*2,h+42),'#dedede');draw=ImageDraw.Draw(result)
    draw.text((6,4),name,fill='black');draw.text((6,22),'Native UIKit iOS 26.0',fill='black');draw.text((w+6,22),'NXKit portable',fill='black')
    result.paste(native.resize((w,h),Image.Resampling.LANCZOS),(0,42));result.paste(portable.resize((w,h),Image.Resampling.LANCZOS),(w,42))
    return result

def main():
    native_dir,portable_dir,out=map(lambda s:Path(s).resolve(),sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
    native,native_trace=states_at(native_dir);portable,portable_trace=states_at(portable_dir)
    done=native_dir/'done.json'
    if done.exists() and not str(json.loads(done.read_text()).get('version','')).startswith('26.'):
        raise ValueError('Native comparison oracle must be iOS26, not27')
    pairs=out/'pairs';pairs.mkdir(exist_ok=True)
    result={'scope':'Static native iOS26/portable snapshot comparison. Descriptive measurements, not an equivalence or pixel-parity test.',
        'limitations':['Whole-image differences include independent glyph rendering, chrome/material optics and semantic colors.','No timing, transition-animation, physical-device or accessibility comparison.','OS shell is excluded from the primary pixel ROI (top62pt, bottom34pt).','Carrier-frame scores require actual frames; viewport and safeExtent metadata are not substituted.','Regression slopes/intercepts combine blur, tint and masking; they are not recovered native alpha or Gaussian sigma.'],
        'nativeCases':len(native),'portableCases':len(portable),'partialInputPermitted':ALLOW_PARTIAL,'missingPortable':sorted(set(native)-set(portable)),
        'extraPortable':sorted(set(portable)-set(native)),'cases':[],'opticalProfiles':{}}
    for name,n in native.items():
        if name not in portable:continue
        a=image_at(native_dir,name);b=image_at(portable_dir,name)
        if a is None or b is None:
            result['cases'].append({'name':name,'error':'Missing PNG from one input'});continue
        p=portable[name];g=geometry_for(n,p)
        row={'name':name,'nativeOffset':n.get('offset'),'portableOffset':p.get('offset'),'nativeSafe':n.get('safe'),'portableSafe':p.get('safe'),
             'primaryROI':[8,62,394,840],'meanAbsoluteRGBDifference':absolute_rgb(a,b,(8,62,394,840)),'geometry':g}
        rois={'top':(8,62,394,171),'bottom':(8,726,394,840)}
        if n.get('sideChrome') or p.get('sideChrome') or 'sides' in name:rois.update(left=(0,116,86,740),right=(316,116,402,740))
        row['edgeStripRGBDifference']={edge:{'roi':box,'meanAbsoluteRGBDifference':absolute_rgb(a,b,box)} for edge,box in rois.items()}
        if n.get('maskProbe') or p.get('maskProbe') not in (None,'','-') or name.startswith('mask-'):
            row['maskProbe']={'nativeException':name=='mask-nil','portableException':p.get('probeException'),'effectROI':[24,220,378,476],'meanAbsoluteRGBDifference':absolute_rgb(a,b,(24,220,378,476))}
        result['cases'].append(row)
        pair_picture(a,b,name).save(pairs/f'{name}.png')
    for theme in ('light','dark'):
        n=profile_pair(native_dir,theme);p=profile_pair(portable_dir,theme)
        if not n or not p:
            result['opticalProfiles'][theme]={'available':False,'reason':'Missing independent soft/hidden pair from one input.'};continue
        nf=profile_fit(n[0],n[1]);pf=profile_fit(p[0],p[1]);deltas=[]
        for nr,pr in zip(nf,pf):
            deltas.append({'y':nr['y'],'rgbSlopeDelta':[pr['rgb'][i]['slope']-nr['rgb'][i]['slope'] for i in range(3)],'rgbInterceptDelta':[pr['rgb'][i]['intercept']-nr['rgb'][i]['intercept'] for i in range(3)]})
        result['opticalProfiles'][theme]={'available':True,'nativeSource':n[2],'portableSource':p[2],'native':nf,'portable':pf,'deltas':deltas}
    for theme in ('light','dark'):
        width,height=402,479;sheet=Image.new('RGB',(width*3,height*3),'#dedede');draw=ImageDraw.Draw(sheet)
        for row,offset in enumerate(('start','middle','end')):
            for col,style in enumerate(('automatic','soft','hard')):
                name=f'{theme}-{style}-{offset}';path=pairs/f'{name}.png'
                if path.exists():
                    image=Image.open(path);image.thumbnail((width,height));sheet.paste(image,(col*width,row*height))
                else:draw.text((col*width+6,row*height+8),f'Missing {name}',fill='black')
        sheet.save(out/f'{theme}-comparison.png')
    if (native_dir/'sparse-profile/light-soft.png').exists() and (portable_dir/'sparse-profile/light-soft.png').exists():
        subprocess.run([sys.executable,str(Path(__file__).with_name('analyze-sparse.py')),str(native_dir/'sparse-profile'),str(out/'sparse'),str(portable_dir/'sparse-profile')],check=True,capture_output=True,text=True)
        sparse=json.loads((out/'sparse/sparse-measurements.json').read_text())
        result['sparseProfiles']={'available':True,'measurementFile':'sparse/sparse-measurements.json','gapMaximumMeanAbsoluteRGBDifference':{renderer:{theme:max(row['meanAbsoluteRGBDifference'] for row in values['transparentGaps']) for theme,values in traits.items()} for renderer,traits in sparse.items() if renderer in ('native','portable')}}
    else:result['sparseProfiles']={'available':False}
    deltas=[value for row in result['cases'] for edge in row.get('geometry',[]) for value in edge.get('deltaPoints',[])]
    result['geometrySummary']={'scoredCarrierFrames':len(deltas)//4,'inactiveEmptyPortableBandsExcluded':True,'portableFrameProvenance':sorted({edge['portable'].get('frameProvenance','unspecified') for row in result['cases'] for edge in row.get('geometry',[]) if edge.get('portable')}),'coordinateRMSPoints':math.sqrt(sum(x*x for x in deltas)/len(deltas)) if deltas else None,'maximumCoordinateDifferencePoints':max(map(abs,deltas)) if deltas else None}
    result['inputTraceSHA256']={'native':hashlib.sha256(native_trace.read_bytes()).hexdigest(),'portable':hashlib.sha256(portable_trace.read_bytes()).hexdigest()}
    result['inputImageSHA256']={key:{str(p.relative_to(directory)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(directory.rglob('*.png')) if p.parent==directory or p.parent.name in ('profile','sparse-profile')} for key,directory in [('native',native_dir),('portable',portable_dir)]}
    result['toolSourceSHA256']={name:hashlib.sha256(Path(__file__).with_name(name).read_bytes()).hexdigest() for name in ['compare-portable.py','analyze-sparse.py']}
    (out/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
    native_link=os.path.relpath(native_dir/'README.md',out)
    geometry=result['geometrySummary']
    report=[
        '# Scroll-edge comparison with iOS 26',
        '',
        f"Compared {len(result['cases'])} matching static cases from {len(native)} native and {len(portable)} portable trace entries. The native oracle is iOS 26.0 release 23A343; see the [native audit]({native_link}).",
        '',
        '[Light comparison](light-comparison.png), [dark comparison](dark-comparison.png), [measurements and trace hashes](comparison.json), [portable state trace](portable-states.json.gz). Native UIKit is on the left of every pair; portable is on the right. Individual pairs are under `pairs/`.',
        '',
        '## Scope and interpretation',
        '',
        'These are measurements of a reproducible fixture, not a claim of pixel or behavioral parity. The primary image region excludes the OS shell; independent glyph rendering, tab/navigation glass and semantic colors remain in broad image differences. Per-row soft/hidden fits exclude title, trailing button and tab-glass samples where practical. They describe combined blur, tint and mask behavior and must not be read as the native alpha mask or Gaussian sigma.',
        '',
        f"Carrier geometry: {geometry['scoredCarrierFrames']} scored bands; coordinate RMS {geometry['coordinateRMSPoints']} points; maximum coordinate difference {geometry['maximumCoordinateDifferencePoints']} points. Portable frame provenance: {', '.join(geometry['portableFrameProvenance']) or 'no actual frames provided'}. Frames derived from renderer snapshot extents are explicitly labelled; geometry scores do not independently verify raster output. Empty inactive portable bands are excluded, since UIKit retains dormant carrier objects that have no corresponding active rendered band.",
        '',
        f"Missing portable trace cases: {', '.join(result['missingPortable']) or 'none'}. Extra portable cases: {', '.join(result['extraPortable']) or 'none'}. Partial-input mode: {ALLOW_PARTIAL}.",
        '',
        '## Optical profiles',
        '',
        'The table records the red-channel slope and intercept of `soft ≈ slope × hidden + intercept` at selected rows. Full RGB fits and per-row differences are in the JSON. A high positive intercept usually indicates a light wash, while a reduced slope with near-zero intercept indicates darkening; blur and color mixing also affect both.',
        '',
        '| Trait | y (pt) | Native red slope / intercept | Portable red slope / intercept |',
        '|---|---:|---:|---:|',
    ]
    for theme,profile in result['opticalProfiles'].items():
        if not profile.get('available'):
            report.append(f'| {theme} | — | — | Independent hidden reference unavailable |');continue
        for nrow,prow in zip(profile['native'],profile['portable']):
            if nrow['y'] not in (20,116,126,156,860):continue
            a=nrow['rgb'][0];b=prow['rgb'][0]
            report.append(f"| {theme} | {nrow['y']} | {a['slope']:.3f} / {a['intercept']:.2f} | {b['slope']:.3f} / {b['intercept']:.2f} |")
    report+=['','## Private variable-blur probes','','| Mask | Mean absolute RGB difference in effect ROI |','|---|---:|']
    for case in result['cases']:
        if case.get('maskProbe'):
            probe=case['maskProbe'];report.append(f"| {case['name']} | {probe['meanAbsoluteRGBDifference']:.3f} |")
    report+=['','Transparent/opaque/ramp masks exercise the real variable-blur renderer; nil is an argument-rejection case. Optical measurements include source raster differences and do not prove a universal private radius/kernel mapping.']
    if result['sparseProfiles'].get('available'):
        report+=['','## Transparent content','','[Sparse light/dark comparison](sparse/sparse-comparison.png), [gap and colored-cell measurements](sparse/sparse-measurements.json). The sparse fixture skips alternate tile fills/text and keeps the drawing background clear; the screen background stays systemBackground.','',f"Maximum per-row mean gap soft-versus-hidden RGB difference: {result['sparseProfiles']['gapMaximumMeanAbsoluteRGBDifference']}. These are gap-preservation checks, not whole-scene equivalence scores."]
    report+=['','## Remaining differences','','The paired sheets still show differences in adaptive bar/title legibility over dense light content, dark bottom/tab optics, and hard-edge material/color mixing. The native coverage-dependent adaptation algorithm is unrecovered; the portable coverage policy and private radius mapping remain approximations. Arbitrarily transformed or frame-animated scroll/chrome geometry is outside this fixture.','','No timing, animation, physical-device, accessibility or alternate-runtime verification is inferred from this static comparison. Nil-mask exception behavior is recorded separately from optical mask images.','','## Reproduction','','```sh',f'python3 Demo/ScrollEdgeComparison/compare-portable.py {native_dir} {portable_dir} {out}','```','']
    (out/'README.md').write_text('\n'.join(report))
    shutil_trace=out/'portable-states.json.gz'
    with gzip.open(shutil_trace,'wb') as f:f.write(json.dumps(list(portable.values()),indent=2).encode())
    print(json.dumps({'compared':len(result['cases']),'nativeCases':len(native),'portableCases':len(portable),'geometry':result['geometrySummary'],'output':str(out)},indent=2))

if __name__=='__main__':main()
