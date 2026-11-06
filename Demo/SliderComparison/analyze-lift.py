"""Extract native stationary lens curves, retaining actual dispatch timestamps."""
from pathlib import Path
import csv,gzip,json,statistics,sys
path=Path(sys.argv[1]);output=Path(sys.argv[2]);output.mkdir(parents=True,exist_ok=True)
trace=json.loads(gzip.decompress(path.read_bytes()) if path.suffix=='.gz' else path.read_text());schema={x['id']:x for x in trace['schema']}
lens=next(x for x in schema.values() if x['delegateClass']=='_UILiquidLensView');white=next((x for x in schema.values() if x['path']==lens['path']+'.0.0'),None)
touches=trace['touches'];begins=[x for x in touches if x['phase']==0];ends=[x for x in touches if x['phase']in [3,4]];begin=begins[0]['dispatchT'];end=ends[0]['dispatchT']
rows=[];opacity_ranges={}
for frame in trace['frames']:
    layers={x['id']:x for x in frame['layers']};l=layers[lens['id']];w=layers.get(white['id']) if white else None
    row={'frameID':frame['frameID'],'t':frame['t'],'fromBegin':frame['t']-begin,'fromEnd':frame['t']-end,'width':l['presentationBounds'][2],'height':l['presentationBounds'][3],'modelWidth':l['modelBounds'][2],'modelHeight':l['modelBounds'][3],'scaleX':l['presentationTransform'][0],'scaleY':l['presentationTransform'][3],'lensOpacity':l['presentationOpacity'],'whiteOpacity':w['presentationOpacity'] if w else None,'whiteModelOpacity':w['modelOpacity'] if w else None,'tracking':frame['tracking'],'highlighted':frame['highlighted'],'touchInside':frame.get('touchInside')}
    rows.append(row)
    for layer in frame['layers']:
        values=opacity_ranges.setdefault(layer['id'],[]);values.append(layer['presentationOpacity'])
with (output/'curve.csv').open('w') as file:
    writer=csv.DictWriter(file,fieldnames=rows[0].keys());writer.writeheader();writer.writerows(rows)
changed=[]
for identity,values in opacity_ranges.items():
    if min(values)!=max(values):changed.append({'schema':schema[identity],'minimum':min(values),'maximum':max(values)})
intervals=[b['t']-a['t'] for a,b in zip(trace['frames'],trace['frames'][1:])]
events=[dict(row,fromBegin=row['t']-begin,fromEnd=row['t']-end) for row in trace['actions']]
summary={'configuration':trace['configuration'],'os':trace['os'],'scale':trace['scale'],'actualTouches':touches,'firstBeginDispatchT':begin,'firstEndDispatchT':end,'lens':lens,'white':white,'sampleCount':len(rows),'medianSampleInterval':statistics.median(intervals),'liftPeak':max((x for x in rows if x['t']<end),key=lambda x:x['width']),'releaseMinimum':min((x for x in rows if x['t']>=end),key=lambda x:x['width']),'varyingOpacityLayers':changed,'events':events,'animations':trace['animations'],'selectedSamples':[]}
for phase,origin,times in [('lift',begin,[0,.0167,.0333,.05,.0667,.0833,.1,.125,.15,.175,.2,.25,.3,.4,.6]),('rest',end,[0,.0167,.0333,.05,.0667,.0833,.1,.15,.2,.25,.3,.4,.5,.6,.8])]:
    for time in times:
        sample=min(rows,key=lambda x:abs(x['t']-(origin+time)));summary['selectedSamples'].append({'phase':phase,'requestedTime':time,**sample})
(output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps({k:summary[k] for k in ['configuration','sampleCount','medianSampleInterval','liftPeak','releaseMinimum','varyingOpacityLayers']},indent=2))
