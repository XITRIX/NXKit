#!/usr/bin/env python3
"""Recover complete numbered MSL diagnostics; reject interleaved/truncated blocks.

Skia src/utils/SkShaderUtils.cpp BuildShaderErrorMessage uses strict newline
splitting and preserves the final empty line. Joining those lines restores the
original source bytes. This does not recover native pipeline descriptors.
"""
import argparse, collections, hashlib, json, pathlib, re
p=argparse.ArgumentParser();p.add_argument('console',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True);counts=collections.Counter();invalid=0
for block in a.console.read_text().split('Shader compilation error\n')[1:]:
 lines=[];valid=True;complete=False
 for line in block.splitlines():
  if line=='Errors:':complete=True;break
  match=re.fullmatch(r'\s*(\d+)\t(.*)',line)
  if match:
   if int(match[1])!=len(lines)+1:valid=False;break
   lines.append(match[2])
 if not valid or not complete or not lines:invalid+=1;continue
 data='\n'.join(lines).encode();key=hashlib.sha256(data).hexdigest();counts[key]+=1
 (a.output/(key+'.metal')).write_bytes(data)
print(json.dumps({'consoleSHA256':hashlib.sha256(a.console.read_bytes()).hexdigest(),'completeSourceOccurrences':counts,'rejectedBlocks':invalid},indent=2))
