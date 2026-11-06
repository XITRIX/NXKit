#!/usr/bin/env python3
"""Build an offline geometry viewer from the retained Lens motion replay.

The replay supplies native control-owned placement/lift bounds to both models;
this viewer compares the additional elastic response, not tab/slider policy.
"""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("replay", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--metadata", type=Path)
    args = parser.parse_args()
    cases = {}
    required = {"t", "inputCenterX", "inputCenterY", "inputWidth", "inputHeight",
                "nativeX", "nativeY", "nativeW", "nativeH", "nxX", "nxY", "nxW", "nxH",
                "nativeScaleX", "nativeScaleY", "nxScaleX", "nxScaleY"}
    with args.replay.open(newline="") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or not required.issubset(reader.fieldnames):
            raise ValueError(f"Missing replay columns: {sorted(required-set(reader.fieldnames or []))}")
        for row in reader:
            name = row.get("case") or row.get("name")
            if not name:
                raise ValueError("Every replay row needs a case name")
            result = {}
            for key, value in row.items():
                if key in ("case", "name", "style"):
                    continue
                try:
                    number = float(value)
                except (TypeError, ValueError):
                    continue
                if math.isfinite(number):
                    result[key] = number
            if not required.issubset(result):
                raise ValueError(f"Nonfinite or missing geometry in {name}")
            cases.setdefault(name, {"name": name, "style": row.get("style", ""), "rows": []})["rows"].append(result)
    for case in cases.values():
        case["rows"].sort(key=lambda row: row["t"])
        squared = []
        before_squared = []
        for row in case["rows"]:
            for suffix in ("X", "Y", "W", "H"):
                squared.append((row[f"native{suffix}"] - row[f"nx{suffix}"]) ** 2)
            before = (row["inputCenterX"] - row["inputWidth"] / 2,
                      row["inputCenterY"] - row["inputHeight"] / 2,
                      row["inputWidth"], row["inputHeight"])
            for suffix, value in zip(("X", "Y", "W", "H"), before):
                before_squared.append((row[f"native{suffix}"] - value) ** 2)
        case["rmse"] = math.sqrt(sum(squared) / len(squared))
        case["beforeRMSE"] = math.sqrt(sum(before_squared) / len(before_squared))
    if not cases:
        raise ValueError("No replay cases")
    metadata = json.loads(args.metadata.read_text()) if args.metadata else {}
    data = {"cases": list(cases.values()), "metadata": metadata,
            "replaySHA256": hashlib.sha256(args.replay.read_bytes()).hexdigest()}
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "replay.csv").write_bytes(args.replay.read_bytes())
    (args.output / "replay.json").write_text(json.dumps(data, separators=(",", ":")) + "\n")
    encoded = json.dumps(data, separators=(",", ":")).replace("<", "\\u003c")
    (args.output / "comparison.html").write_text(PAGE.replace("__REPLAY__", encoded))
    print(f"Wrote {len(cases)} motion comparisons to {args.output}")


PAGE = r'''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lens liquid motion · UIKit / NXKit</title>
<style>
:root{color-scheme:dark}body{font:16px/1.5 system-ui;margin:28px auto;padding:0 22px;max-width:1100px;background:#17181b;color:#eee}h1{font-size:28px;line-height:1.2}p{color:#bec5d0;max-width:960px}label{display:inline-flex;align-items:center;gap:8px;margin:6px 18px 6px 0}select,button{font:inherit;padding:6px;background:#282b32;color:inherit;border:1px solid #636b78;border-radius:6px}input[type=range]{vertical-align:middle;min-width:220px}canvas{display:block;width:100%;background:#22262d;border:1px solid #454c58;border-radius:8px;box-sizing:border-box;margin-top:12px}#shape{height:230px}.plot{height:155px}.legend{display:flex;flex-wrap:wrap;gap:18px;margin:14px 0}.native{color:#66c9ff}.nx{color:#ff9688}.input{color:#bbc4d3}a{color:#9dceff}#status{font-variant-numeric:tabular-nums}small{color:#b2bac7}h2{font-size:17px;margin:20px 0 0}footer{margin:24px 0}button{cursor:pointer}
</style><h1>Lens liquid motion · UIKit / NXKit</h1>
<p>Recorded UIKit lens placement drives the portable response. Both use the same control-owned position, lift size and ancestor scale; the outlines and curves compare the additional liquid deformation. This isolates lens physics from tab selection and slider layout.</p>
<label>Scene <select id="scene" aria-label="Scene"></select></label>
<label>Revision <select id="revision" aria-label="Revision"><option value="after">After</option><option value="before">Before · no elastic response</option></select></label>
<button id="play" type="button">Play</button>
<label>Time <input id="time" aria-label="Time" type="range" min="0" step="0.001" value="0"></label>
<div id="status" aria-live="off"></div>
<div class="legend"><span class="native">━ UIKit</span><span class="nx">━ NXKit</span><span class="input">┄ Shared position / lift bounds</span></div>
<canvas id="shape" width="1100" height="230" aria-label="Aligned native and NXKit lens geometry"></canvas>
<small>Geometry overlay, enlarged around the shared input center. Measured bounds are drawn as capsule guides, rather than native contours or rendered-glass pixels. Chromatic pixel overlays are <a href="../chromatic/comparison.html">separate</a>.</small>
<h2>Horizontal stretch</h2><canvas class="plot" id="width" width="1100" height="155" aria-label="Horizontal stretch over time"></canvas>
<h2>Vertical stretch</h2><canvas class="plot" id="height" width="1100" height="155" aria-label="Vertical stretch over time"></canvas>
<h2>Horizontal lens-local drift</h2><canvas class="plot" id="driftX" width="1100" height="155" aria-label="Horizontal drift over time"></canvas>
<h2>Vertical lens-local drift</h2><canvas class="plot" id="driftY" width="1100" height="155" aria-label="Vertical drift over time"></canvas>
<p id="summary"></p>
<footer><a href="README.md">Evidence, method and limits</a> · <a href="replay.csv">Replay CSV</a> · <a href="replay.json">Replay and provenance</a></footer>
<script type="application/json" id="data">__REPLAY__</script><script>
'use strict';
const data=JSON.parse(document.getElementById('data').textContent),scene=document.getElementById('scene'),revision=document.getElementById('revision'),time=document.getElementById('time'),play=document.getElementById('play');
for(const item of data.cases){const option=document.createElement('option');option.value=item.name;option.textContent=item.name;scene.append(option)}
scene.value=data.cases.some(c=>c.name==='tabs-slow-fixed')?'tabs-slow-fixed':data.cases[0].name;
let active=data.cases[0],playing=false,last=0;
function frameAt(t){let lo=0,hi=active.rows.length-1;while(lo<hi){const m=Math.floor((lo+hi)/2);if(active.rows[m].t<t)lo=m+1;else hi=m}if(lo>0&&Math.abs(active.rows[lo-1].t-t)<Math.abs(active.rows[lo].t-t))lo--;return active.rows[lo]}
function context(canvas){const dpr=window.devicePixelRatio||1,w=Math.round(canvas.clientWidth*dpr),h=Math.round(canvas.clientHeight*dpr);if(canvas.width!==w||canvas.height!==h){canvas.width=w;canvas.height=h}const ctx=canvas.getContext('2d');ctx.setTransform(dpr,0,0,dpr,0,0);return ctx}
function rounded(ctx,x,y,w,h){const r=Math.max(0,Math.min(w,h)/2);ctx.beginPath();ctx.roundRect(x,y,Math.max(0,w),Math.max(0,h),r);ctx.stroke()}
function shape(row){const canvas=document.getElementById('shape'),ctx=context(canvas),W=canvas.clientWidth,H=canvas.clientHeight;ctx.clearRect(0,0,W,H);const scale=Math.min(6,W/(Math.max(row.inputWidth,row.nativeW,row.nxW)*2.3),H/(Math.max(row.inputHeight,row.nativeH,row.nxH)*1.7));const x=v=>W/2+(v-row.inputCenterX)*scale,y=v=>H/2+(v-row.inputCenterY)*scale;
ctx.strokeStyle='#37404b';ctx.lineWidth=1;ctx.beginPath();for(let dx=-100;dx<=100;dx+=10){ctx.moveTo(x(row.inputCenterX+dx),0);ctx.lineTo(x(row.inputCenterX+dx),H)}for(let dy=-50;dy<=50;dy+=10){ctx.moveTo(0,y(row.inputCenterY+dy));ctx.lineTo(W,y(row.inputCenterY+dy))}ctx.stroke();
ctx.setLineDash([6,5]);ctx.strokeStyle='#bbc4d3';rounded(ctx,x(row.inputCenterX-row.inputWidth/2),y(row.inputCenterY-row.inputHeight/2),row.inputWidth*scale,row.inputHeight*scale);ctx.setLineDash([]);ctx.lineWidth=2.5;ctx.strokeStyle='#66c9ff';rounded(ctx,x(row.nativeX),y(row.nativeY),row.nativeW*scale,row.nativeH*scale);ctx.strokeStyle='#ff9688';const before=revision.value==='before';rounded(ctx,x(before?row.inputCenterX-row.inputWidth/2:row.nxX),y(before?row.inputCenterY-row.inputHeight/2:row.nxY),(before?row.inputWidth:row.nxW)*scale,(before?row.inputHeight:row.nxH)*scale)}
function plot(id,axis,t,drift=false){const canvas=document.getElementById(id),ctx=context(canvas),W=canvas.clientWidth,H=canvas.clientHeight,pad=48,end=active.rows.at(-1).t;ctx.clearRect(0,0,W,H);const sample=(r,prefix)=>drift?(r[prefix+'Drift'+axis]||0):100*(r[prefix+'Scale'+axis]-1);const values=active.rows.flatMap(r=>[sample(r,'native'),sample(r,'nx')]);const range=Math.max(1,...values.map(Math.abs))*1.12;const px=v=>pad+v/Math.max(end,.001)*(W-pad-12),py=v=>H/2-v/range*(H/2-24);ctx.font='12px system-ui';ctx.strokeStyle='#48515e';ctx.fillStyle='#c3ccd8';for(const v of [-range,0,range]){ctx.beginPath();ctx.moveTo(pad,py(v));ctx.lineTo(W,py(v));ctx.stroke();ctx.fillText(v.toFixed(1)+(drift?' pt':'%'),3,py(v)+4)}for(const [prefix,color] of [['native','#66c9ff'],['nx','#ff9688']]){ctx.strokeStyle=color;ctx.lineWidth=2;ctx.beginPath();active.rows.forEach((r,i)=>{const value=prefix==='nx'&&revision.value==='before'?0:sample(r,prefix);if(i)ctx.lineTo(px(r.t),py(value));else ctx.moveTo(px(r.t),py(value))});ctx.stroke()}ctx.strokeStyle='#e6e9ef';ctx.lineWidth=1;ctx.beginPath();ctx.moveTo(px(t),10);ctx.lineTo(px(t),H-12);ctx.stroke();ctx.fillStyle='#bbc4d3';ctx.fillText('0 s',pad,H-3);ctx.fillText(end.toFixed(2)+' s',W-65,H-3)}
function draw(){const t=Number(time.value),row=frameAt(t);shape(row);plot('width','X',t);plot('height','Y',t);plot('driftX','X',t,true);plot('driftY','Y',t,true);document.getElementById('status').textContent=`${t.toFixed(3)} s · ${row.phase===-1?'Before contact':row.phase===3?'Released':row.phase===4?'Cancelled':'Contact'} · UIKit ${row.nativeW.toFixed(2)} × ${row.nativeH.toFixed(2)} pt · NXKit ${(revision.value==='before'?row.inputWidth:row.nxW).toFixed(2)} × ${(revision.value==='before'?row.inputHeight:row.nxH).toFixed(2)} pt`;const rmse=revision.value==='before'?active.beforeRMSE:active.rmse;document.getElementById('summary').textContent=`${active.rows.length} recorded comparison samples. Position/size RMSE: ${rmse.toFixed(3)} pt. Model bounds and control placement are shared inputs; this does not establish full UIKit control or touch-latency parity.`}
function choose(){active=data.cases.find(c=>c.name===scene.value)||data.cases[0];time.max=active.rows.at(-1).t;time.value=0;draw()}
window.addEventListener('resize',draw);scene.addEventListener('change',choose);revision.addEventListener('change',draw);time.addEventListener('input',draw);play.addEventListener('click',()=>{playing=!playing;play.textContent=playing?'Pause':'Play';last=0;if(playing)requestAnimationFrame(tick)});
function tick(now){if(!playing)return;if(last)time.value=String(Number(time.value)+(now-last)/1000);last=now;if(Number(time.value)>=Number(time.max)){time.value=0}draw();requestAnimationFrame(tick)}choose();
</script></html>'''


if __name__ == "__main__":
    main()
