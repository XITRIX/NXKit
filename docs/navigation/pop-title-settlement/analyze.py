"""Check retained UIKit outgoing-title motion and current release-frame traces.
Run: python3 analyze.py REPOSITORY FRAME_CSV OUTPUT_DIRECTORY
Native measurements use retained iOS27.0 captures, not a fresh native run.
"""
import csv,gzip,hashlib,json,math,statistics,sys
from pathlib import Path
root=Path(sys.argv[1]);frames=Path(sys.argv[2]);out=Path(sys.argv[3]);out.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
rows=list(csv.DictReader(frames.open()));assert len(rows)==7200
cases={tuple(r[k] for k in ('dark','scale','custom_titles','progress','velocity')) for r in rows};assert len(cases)==120
source=[float(r['source_title'])-float(r['source_content']) for r in rows];target=[float(r['target_title'])-float(r['target_content']) for r in rows];mask=[float(r['reveal_width'])-(.3*402+.7*float(r['source_content'])) for r in rows]
for values in (source,target,mask):assert max(map(abs,values))<.02
native_file=root/'docs/navigation/interactive/native-traces.json.gz';native=json.loads(gzip.decompress(native_file.read_bytes()));titles=['Shared','Accent','Separated','Three items','Text','Leading','Unshared','Hidden glass'];native_cases=[]
for name,clip in sorted(native.items()):
 if not name.endswith('finish'):continue
 anchor=None;started=False;errors=[]
 for f in clip['frames']:
  if f['state'] in (1,2):started=True
  labels=[v for v in f['views'] if v['class']=='UILabel' and v.get('text')==titles[clip['source']]]
  if not labels or not f['sourceContent']:continue
  relative=labels[0]['frame'][0]-f['sourceContent'][0]
  if anchor is None:anchor=relative
  if started and f['state']==0 and abs(f['sourceContent'][0])<401:errors.append(relative-anchor)
 assert errors
 native_cases.append({'name':name,'os':clip['os'],'releaseSamples':len(errors),'maximumTitleToContentOffsetChangePoints':max(map(abs,errors))})
assert len(native_cases)==16
report={'nativeScope':'Retained iOS27.0 ordinary inline titles, eight arrangements, both themes, committed gesture release only. Existing captures; no new UIKit run. This checks outgoing-title attachment, not absolute spring timing or incoming-title optical parity.','nativeSourceSHA256':sha(native_file),'nativeCases':native_cases,'portableFrames':len(rows),'portableCases':len(cases),'frameCSV_SHA256':sha(frames),'maximumSourceTitleToContentDeltaPoints':max(map(abs,source)),'maximumTargetTitleToContentDeltaPoints':max(map(abs,target)),'maximumRevealWidthDeltaPoints':max(map(abs,mask)),'portableScope':'120 real routed edge-gesture cases, 60 samples per release at 120Hz; light/dark, 1x/2x/3x, ordinary/custom source/destination titles, paused commit/cancel, forward flick and backwards cancellation. Geometry relationships and carrier continuity are asserted before completion; lifecycle/outcome/hierarchy checked afterward. CSV values round to six significant digits; production assertions use full precision with 0.02pt tolerance.'}
(out/'motion.json').write_text(json.dumps(report,indent=2)+'\n');(out/'release-frames.csv.gz').write_bytes(gzip.compress(frames.read_bytes(),mtime=0));print(json.dumps({k:v for k,v in report.items() if k.startswith('maximum') or k.startswith('portableF') or k.startswith('portableC')}))
