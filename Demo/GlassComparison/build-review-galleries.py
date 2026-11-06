"""Rebuild offline review galleries from retained comparison products."""
from pathlib import Path
import json

docs = Path(__file__).resolve().parents[2] / 'docs'

def gallery(folder, title, description, rows, modes):
    for row in rows:
        for _, suffix in modes:
            if not (folder / (row['file'] + suffix)).is_file():
                raise FileNotFoundError(row['file'] + suffix)
    html = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>TITLE</title>
<style>:root{color-scheme:dark}body{font:15px/1.5 system-ui;margin:24px;background:#191a1e;color:#eee}h1{font-size:25px}p{max-width:1000px}a{color:#9cf}label{display:inline-block;margin:6px}select{font:inherit}#stage{width:max-content;max-width:100%;background:repeating-conic-gradient(#444 0 25%,#333 0 50%) 50%/16px 16px}img{display:block;max-width:100%;height:auto}#stage.pixels{max-width:none}#stage.pixels img{max-width:none}</style>
<h1>TITLE</h1><p>DESCRIPTION</p><label>Scene <select id="scene" aria-label="Scene"></select></label><label>View <select id="mode" aria-label="View"></select></label><label>Zoom <select id="zoom" aria-label="Zoom"><option>Fit</option><option>Actual pixels</option></select></label><div id="stage"><img id="image" alt="Aligned comparison"></div><p><a href="metrics.json">Measurements</a> · <a href="provenance.json">Provenance</a> · <a href="README.md">Method and limits</a></p>
<script>const rows=ROWS,modes=MODES;const scene=document.getElementById('scene'),mode=document.getElementById('mode'),zoom=document.getElementById('zoom'),stage=document.getElementById('stage'),im=document.getElementById('image');rows.forEach((r,i)=>scene.add(new Option(r.name,i)));modes.forEach((r,i)=>mode.add(new Option(r[0],i)));function update(){im.src=rows[scene.value].file+modes[mode.value][1];stage.className=zoom.selectedIndex?'pixels':''}scene.onchange=mode.onchange=zoom.onchange=update;update();</script></html>'''
    html = html.replace('TITLE', title).replace('DESCRIPTION', description).replace('ROWS', json.dumps(rows)).replace('MODES', json.dumps(modes))
    (folder / 'comparison.html').write_text(html)

folder = docs / 'liquid-glass-shadows'
records = json.loads((folder / 'metrics.json').read_text())['records']
gallery(folder, 'UIKit / NXKit default shadows · 84 measured scenes',
        '36 retained diagnostic montages cover neutral material and additional size/group/clear controls. The linked metrics include all 84 scenes, including the tint controls whose exterior shadows are identical. Montage columns: native / old / current / absolute exterior difference ×8. Exterior masks exclude glass bodies and joins. Signed residual overlays use red for weak shadows and blue for excessive darkness; alpha is amplified ×32. These are diagnostic amplifications, not raw image error. Both themes and colored backgrounds are retained.',
        [dict(name=r['id'],file='images/'+r['id']) for r in records if (folder/'images'/(r['id']+'.png')).is_file()],
        [('Native / old / current / difference ×8', '.png'), ('Exterior mask', '-exterior-mask.png'), ('Signed residual overlay ×32', '-residual-overlay.png')])
folder = docs / 'liquid-glass-shadows/shared-material'
gallery(folder, 'UIKit / NXKit shared neutral material sizing',
        'Montage columns: native / separate defaults / shared defaults / body difference ×8. Groups share the largest neutral regular member’s material defaults while retaining each item’s optical geometry. White/gray, both themes; the small light-white regression is retained.',
        [dict(name=p.stem,file=p.stem) for p in sorted(folder.glob('*.png'))], [('Native / old / current / body difference ×8','.png')])
folder = docs / 'liquid-glass-interaction/retained-joins/current-combined'
gallery(folder, 'UIKit / NXKit retained joins · 96 checkpoints',
        'Columns: native / matched-2× frozen old / current / amplified absolute difference. Rows: initial / held / pulled / released. Geometry comes from native presentation traces; no registration or colors are fitted. Both insertion orders, both themes, neutral/blue and blue/coral. The final row preserves items that remain merged after release.',
        [dict(name=p.stem,file=p.stem) for p in sorted(folder.glob('join-*.png'))], [('Native / old / current / amplified difference','.png')])
