#!/usr/bin/env python3
"""Report UIKit inflation, centered pull, and fitted damped spring parameters.

Requires numpy. The per-transition latency is reported separately from spring
parameters: UIKit event dispatch and compositor sampling are not instantaneous.
No parameter in this report is an Apple API contract.
"""
import argparse
import json
from pathlib import Path
import numpy as np

def decay(t, omega, zeta):
    frequency = omega * np.sqrt(1-zeta*zeta)
    return np.exp(-omega*zeta*t)*(np.cos(frequency*t)+omega*zeta/frequency*np.sin(frequency*t))

def fit(times, values, rising):
    def evaluate(p):
        response = decay(np.maximum(0, times-p[2]), p[0], p[1])
        return 1-response if rising else response
    best = None
    for guess in [(24,.625,.02),(18,.6,.02),(16,.375,.02)]:
        p = np.array(guess)
        for _ in range(60):
            error = evaluate(p)-values
            jacobian = np.column_stack([(evaluate(p+np.eye(3)[i]*1e-5)-evaluate(p-np.eye(3)[i]*1e-5))/2e-5 for i in range(3)])
            step = np.linalg.lstsq(jacobian,error,rcond=None)[0]
            old = np.mean(error**2)
            for scale in [1,.5,.25,.125,.0625]:
                trial = np.clip(p-step*scale,[5,.1,-.02],[100,.99,.3])
                if np.mean((evaluate(trial)-values)**2)<old:
                    p=trial;break
            else: break
        score=float(np.sqrt(np.mean((evaluate(p)-values)**2)))
        if best is None or score<best[0]: best=(score,p)
    score,p=best
    return dict(omega=float(p[0]),zeta=float(p[1]),duration=float(2*np.pi/p[0]),latency=float(p[2]),normalizedRMSE=score)

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("directories",type=Path,nargs="+")
parser.add_argument("--output",type=Path)
args=parser.parse_args()
results={}
for directory in args.directories:
    for path in sorted(directory.glob("*/trace.json")):
        trace=json.loads(path.read_text()); config=trace['configuration']
        target='glass1' if config.get('pair') else 'glass0'
        identifier=next(s['id'] for s in trace['schema'] if s['name']==target)
        events=trace['events']; end=next(e.get('dispatch',e['t']) for e in events if e['phase'] in [3,4])
        begin=events[0].get('dispatch',events[0]['t'])
        moving=next((e.get('dispatch',e['t']) for e in events if e['phase']==1),end)
        width,height=config['width'],config['height']; longest=max(width,height)
        geometry=np.array([[f['t'],*next(r for r in f['layers'] if r[0]==identifier)[1:5]] for f in trace['frames']])
        t=geometry[:,0]; inflation=((geometry[:,3]/width+geometry[:,4]/height)/2-1)*longest
        # Opposing relative stretches cancel even on rectangles. Sampling the
        # held tail avoids assuming that every busy simulator delivered a
        # display-link callback in a particular early time window.
        steady=float(np.median(inflation[(t>end-.25)&(t<end-.08)]))
        pressed=(t>=begin)&(t<min(moving,begin+.5))
        released=(t>end-.02)&(t<end+.7)
        held=(t>end-.25)&(t<end-.08)
        pull=float(np.median(geometry[held,3])-width*(1+steady/longest))
        press=fit(t[pressed]-begin,inflation[pressed]/steady,True)
        if np.count_nonzero(pressed)<8:
            # A stalled acquisition cannot identify three spring parameters.
            # Align against independently measured same-size-policy springs,
            # retain the sparse curve and explicitly report its limitation.
            weight=np.clip((longest-44)/116,0,1) if longest>=120 else 0
            duration=(2*np.pi/24)*(1-weight)+(2*np.pi/20)*weight
            omega=2*np.pi/duration
            candidates=np.linspace(0,.3,3001)
            errors=[np.mean((1-decay(np.maximum(0,t[pressed]-begin-lag),omega,.625)-inflation[pressed]/steady)**2) for lag in candidates]
            latency=float(candidates[np.argmin(errors)])
            press=dict(omega=float(omega),zeta=.625,duration=float(duration),latency=latency,
                normalizedRMSE=float(np.sqrt(min(errors))),undersampled=True)
        results[config['id']]=dict(width=width,height=height,growth=steady,pullWidth=pull,
            press=press,release=fit(t[released]-end,inflation[released]/steady,False))
for name,r in results.items():
    print(f"{name:20} growth {r['growth']:9.5f} pull {r['pullWidth']:8.5f} "
          f"press {r['press']['duration']:.4f}/{r['press']['zeta']:.4f} "
          f"release {r['release']['duration']:.4f}/{r['release']['zeta']:.4f} "
          f"lag {r['press']['latency']:.4f}/{r['release']['latency']:.4f}")
if args.output:
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(results,indent=2)+'\n')
