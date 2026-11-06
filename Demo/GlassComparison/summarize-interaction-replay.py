#!/usr/bin/env python3
"""Summarize the native/production CSV emitted by GlassInteractionTests."""
import argparse
import csv
import json
import math
from collections import defaultdict
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("csv", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
errors = defaultdict(list)
frames = defaultdict(int)
with args.csv.open() as source:
    for row in csv.DictReader(source):
        errors[row['case']].extend(float(row['nx'+axis])-float(row['native'+axis]) for axis in ['X','Y','W','H'])
        frames[row['case']] += 1
cases = {name: dict(frames=frames[name],rmse=math.sqrt(sum(v*v for v in values)/len(values)),maximum=max(map(abs,values))) for name,values in errors.items()}
result = dict(units="points",comparison="All retained input/presentation samples, with explicitly retained compositor timing alignment.",
    cases=cases,caseCount=len(cases),frameCount=sum(frames.values()),
    worstCaseRMSE=max(case['rmse'] for case in cases.values()),maximum=max(case['maximum'] for case in cases.values()))
args.output.write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k!='cases'},indent=2))
