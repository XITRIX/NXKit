"""Verify the bounded native activation conclusions from retained frame traces."""
import gzip,json
from pathlib import Path
root=Path(__file__).resolve().parent
identity=[1,0,0,1,0,0]
read=lambda name:json.loads(gzip.decompress((root/(name+'-trace.json.gz')).read_bytes()))
result={}
for name in ['resting-2x','resting-3x']:
    frames=[f for f in read(name)['frames'] if .8<f['t']<3.5]
    maximum=max(abs(v-i) for f in frames for v,i in zip(f['transform'],identity))
    assert maximum==0 and {f['activation'] for f in frames}=={1}
    result[name]={'samples':len(frames),'maxElasticAffineError':maximum,'activation':1}
frames=read('lifted-2x')['frames'];comparisons=[]
for style in [0,1]:
    by_name={name:[f for f in frames if f['style']==style and f['name']==name and .8<f['t']<3.5] for name in dict.fromkeys(f['name'] for f in frames)}
    assert all(f['activation']==3 for rows in by_name.values() for f in rows)
    for own,names in [('lens-position',['parent-position','ancestor-position','parent-transform']),('lens-animation',['ancestor-animation'])]:
        for name in names:
            a,b=by_name[own],by_name[name];assert len(a)==len(b)
            maximum=max(abs(x-y) for f,g in zip(a,b) for x,y in zip(f['transform'],g['transform']))
            assert maximum<1e-8
            comparisons.append({'style':style,'own':own,'ancestor':name,'maxAffineDifference':maximum})
    # Positive bounds-origin displacement moves descendants in the opposite
    # direction: the signed drift mirrors while directionless strain agrees.
    maximum=max(abs(x-(-y if i==4 else y)) for f,g in zip(by_name['lens-position'],by_name['parent-bounds']) for i,(x,y) in enumerate(zip(f['transform'],g['transform'])))
    assert maximum<1e-8
    comparisons.append({'style':style,'own':'lens-position','ancestor':'parent-bounds (mirrored x drift)','maxAffineDifference':maximum})
    for name,rows in by_name.items():
        maximum=max(abs(x-y) for f in rows for x,y in zip(f['transform'],identity))
        assert maximum==0 if name=='stationary' else maximum>0
result['liftedEquivalentMotion']=comparisons
frames=read('lowering-2x')['frames'];transitions=[]
for style in [0,1]:
    rows=[f for f in frames if f['style']==style and f['name']=='parent-position']
    assert any(not f['lifted'] and f['activation']==3 for f in rows)
    assert rows[-1]['activation']==1
    old=None
    for f in rows:
        state=(f['lifted'],f['activation'])
        if state!=old:transitions.append({'style':style,'t':f['t'],'lifted':state[0],'activation':state[1]});old=state
result['loweringTransitions']=transitions
print(json.dumps(result,indent=2))
