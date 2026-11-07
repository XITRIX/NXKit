"""Compact observed results for REA stdout; timestamps relative to UIWindow delivery."""
from pathlib import Path
import json,sys

def summarize(directory):
 rows=[]
 for path in sorted(Path(directory).glob('*/trace.json')):
  d=json.loads(path.read_text());es=d['events'];down=next((e for e in es if e['kind']=='window-before' and e['phase']==0),None)
  if not down:continue
  t0=down['t'];fs=[f for f in d['frames'] if f['t']>=t0]
  if not fs:continue
  ms=lambda t:round((t-t0)*1000,2)
  first=lambda predicate:next((ms(e['t']) for e in es if predicate(e)),None)
  lift=next((ms(f['t']) for f in fs if any(l.get('lifted') for l in f['lenses'])),None)
  rows.append({'id':d['config']['id'],'touchCount':sum(e['kind']=='window-before' for e in es),'durationMS':ms(fs[-1]['t']),'initialValue':fs[0]['value'],'finalValue':fs[-1]['value'],'finalOffset':fs[-1]['scrollOffset'],'firstLiftMS':lift,'scrollShouldBeginMS':first(lambda e:e['kind']=='scroll-touchesShouldBegin'),'controlTouchesBeganMS':first(lambda e:e['kind']=='control-touchesBegan'),'scrollDragMS':first(lambda e:e['kind']=='scroll-willBeginDragging'),'controlPanActionBeganMS':first(lambda e:e['kind']=='recognizer-action' and e['class']=='UIPanGestureRecognizer' and e['state']==1),'controlLongPressActionBeganMS':first(lambda e:e['kind']=='recognizer-action' and e['class']=='UILongPressGestureRecognizer' and e['state']==1),'valueChangedMS':first(lambda e:e['kind']=='action' and e['event']==4096),'publicScrollCallbacks':[{**e,'dtMS':ms(e['t'])} for e in es if e['kind']in['scroll-touchesShouldBegin','scroll-touchesShouldCancel']],'completePhysicalTouch':any(e['kind']=='window-before' and e['phase']==3 for e in es)})
 return rows
if __name__=='__main__':print(json.dumps(summarize(sys.argv[1]),indent=2))
