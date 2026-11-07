#!/usr/bin/env python3
"""Read-only, build-specific UIKit gesture arbitration audit; no UIKit execution."""
import gzip,hashlib,json,subprocess,sys
from pathlib import Path
binary=Path(sys.argv[1] if len(sys.argv)>1 else '/private/tmp/nxkit-glass-settings/binaries/UIKitCore')
expected='5cefe8cfb213b0dfcb378feb86ddd9062fda37fb4f331b3ceb782070a20292a4'
digest=hashlib.sha256(binary.read_bytes()).hexdigest()
if digest!=expected:raise SystemExit('This focused audit requires the documented iOS27.0 UIKitCore binary hash')
out=Path(__file__).resolve().parents[2]/'docs/controls/scroll-arbitration/disassembly';out.mkdir(parents=True,exist_ok=True)
methods={
 'switch-failure-policy':'-[UISwitchModernVisualElement gestureRecognizer:shouldRequireFailureOfGestureRecognizer:]',
 'switch-event-admission':'-[UISwitchModernVisualElement _gestureRecognizer:shouldReceiveEvent:]',
 'switch-simultaneous-policy':'-[UISwitchModernVisualElement gestureRecognizer:shouldRecognizeSimultaneouslyWithGestureRecognizer:]',
 'switch-long-press':'-[UISwitchModernVisualElement _handleLongPressWithGestureLocationInBounds:gestureState:]',
 'scroll-delay-began':'-[UIScrollViewDelayedTouchesBeganGestureRecognizer touchesBegan:withEvent:]',
 'scroll-delay-moved':'-[UIScrollViewDelayedTouchesBeganGestureRecognizer touchesMoved:withEvent:]',
 'scroll-delay-ended':'-[UIScrollViewDelayedTouchesBeganGestureRecognizer touchesEnded:withEvent:]',
 'scroll-delay-deliver':'-[UIScrollViewDelayedTouchesBeganGestureRecognizer sendDelayedTouches]',
 'slider-should-begin':'-[UISlider gestureRecognizerShouldBegin:]'
}
cmd=['xcrun','lldb','-b','-o','target create '+str(binary)]
for method in methods.values():cmd+=['-o','disassemble -n "'+method+'"']
cmd+=['-o','memory read -fx -s8 -c1 0x222e8ca30','-o','image lookup -a 0x22e8701d0','-o','quit']
r=subprocess.run(cmd,text=True,capture_output=True,check=True)
if 'error:' in r.stdout+r.stderr:raise SystemExit(r.stdout+r.stderr)
hashes={};previews={}
for name,method in methods.items():
 marker='(lldb) disassemble -n "'+method+'"\n'
 body=r.stdout.split(marker,1)[1].split('(lldb)',1)[0]
 (out/(name+'.txt.gz')).write_bytes(gzip.compress(body.encode(),mtime=0))
 preview='\n'.join(line.rstrip() for line in body.splitlines())+'\n'
 (out/(name+'.txt')).write_text(preview)
 hashes[name]=hashlib.sha256(body.encode()).hexdigest();previews[name]=hashlib.sha256(preview.encode()).hexdigest()
slot=r.stdout.split('(lldb) memory read ',1)[1].split('(lldb) quit',1)[0]
assert '0x000000022e8701d0' in slot and 'UIScrollViewDelayedTouchesBeganGestureRecognizer' in slot
(out/'switch-required-class.txt').write_text(slot)
policy=(out/'switch-failure-policy.txt').read_text()
assert 'objc_msgSend$longPress' in policy and 'objc_msgSend$pan' in policy
assert 'objc_opt_isKindOfClass' in policy and '[x8, #0xa30]' in policy
manifest={'scope':'Static read-only shipped UIKitCore disassembly; methods are not executed',
 'binarySHA256':digest,'methods':methods,'disassemblySHA256':hashes,'normalizedPreviewSHA256':previews,
 'disassemblyEncoding':'Original LLDB bytes are retained in .txt.gz; .txt previews normalize trailing whitespace',
 'observedPolicy':'Switch longPress and pan require failure of recognizers whose class is UIScrollViewDelayedTouchesBeganGestureRecognizer; other own recognizers return false in this method',
 'requiredClassReference':{'slot':'0x222e8ca30','value':'0x22e8701d0','name':'UIScrollViewDelayedTouchesBeganGestureRecognizer'},
 'unknowns':['No assertion about other UIKit builds or complete dependency graph; runtime outcome remains defined by real touch traces']}
(out.parent/'static-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest))
