"""Summarize alternating, compilation-free before/after ScrollPocket captures.
Usage: python3 analyze-pocket-performance.py LOG_DIRECTORY OUTPUT.json
The fixture scrolls vertically with four edge-chrome pockets, at 402x874pt.
"""
import csv,hashlib,json,statistics,sys
from pathlib import Path
logs=Path(sys.argv[1]);output=Path(sys.argv[2]);profiles=[];artifacts=[]
for scale,theme in ((3,'dark'),(2,'light')):
    result={'scale':scale,'theme':theme,'versions':{}}
    for version in ('before','after'):
        rows=[]
        for repeat in range(3):
            path=logs/f'ab-{version}-{scale}-{theme}-{repeat}.csv'
            with path.open() as source:current=list(csv.DictReader(source))
            assert len(current)==975 and all(int(r['library_compiles'])==int(r['pipeline_compiles'])==int(r['failures'])==0 for r in current)
            rows += [r for r in current if int(r['pass'])>=1 and r['case'] in ('soft-discovered','automatic-discovered') and r['phase']=='vertical']
            artifacts.append({'file':path.name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
        result['versions'][version]={'frames':len(rows),'cpuMedianMS':statistics.median(float(r['cpu_ms']) for r in rows),'synchronizedTotalMedianMS':statistics.median(float(r['total_ms']) for r in rows)}
    a=result['versions']['before'];b=result['versions']['after'];result['cpuMedianReductionPercent']=100*(1-b['cpuMedianMS']/a['cpuMedianMS']);result['totalMedianReductionPercent']=100*(1-b['synchronizedTotalMedianMS']/a['synchronizedTotalMedianMS']);profiles.append(result)
report={'scope':'Alternating paired local macOS Metal measurements; not phone power/thermal measurements.','fixture':'402x874pt public UIKit scene with all four edge chrome containers; discovered automatic/soft vertical-scroll rows only.','repeats':3,'passesUsed':[1,2],'repeatOrder':['before-after','after-before','before-after'],'compilationPolicy':'Offline bundles only; runtime library/pipeline compilation forbidden in both versions.','profiles':profiles,'csvInputs':artifacts,'limits':['Only two scale/theme combinations are paired.','First pass omitted to focus on repeated rendering.','Medians include real layout, rendering, submission and GPU completion wait; they are not isolated shader timing.','The four-pocket fixture differs from a typical top/bottom-only phone screen.','No physical phone power, temperature, sustained frame pacing or battery measurement.']}
output.write_text(json.dumps(report,indent=2)+'\n')
for p in profiles:print(p['theme'],p['scale'],'CPU reduction',round(p['cpuMedianReductionPercent'],1),'%, total reduction',round(p['totalMedianReductionPercent'],1),'%')
