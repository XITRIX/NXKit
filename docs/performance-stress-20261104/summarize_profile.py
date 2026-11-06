#!/usr/bin/env python3
"""Resolve Instruments XML references and count inclusive stacks once per sample."""
import argparse, collections, json, xml.etree.ElementTree as ET
p=argparse.ArgumentParser();p.add_argument('xml');p.add_argument('--start',type=float,default=4);p.add_argument('--end',type=float,default=float('inf'));a=p.parse_args()
r=ET.parse(a.xml).getroot();ids={e.attrib['id']:e for e in r.iter() if 'id' in e.attrib}
def resolve(e):
    while e is not None and 'ref' in e.attrib:e=ids[e.attrib['ref']]
    return e
inclusive=collections.Counter();leaf=collections.Counter();total=collections.Counter();groups=collections.Counter()
patterns={'layout':'NXKit::UIView::layoutTreeIfNeeded','display':'NXKit::UIView::displayTreeIfNeeded','tint':'NXKit::UIView::tintColor','corners':'NXKit::UIView::updateCornerConfigurationTree','render':'NXKit::CALayer::skiaRender','glass':'NXKit::CABackdropEffectLayer::draw','submit':'NXKit::SkiaCtx::flushAndSubmit','pacing':'NXKit::SkiaCtx_sdlBase::sleepForNextFrame'}
for row in r.iter('row'):
    time=resolve(row.find('sample-time'))
    if time is None or not a.start<=int(time.text)/1e9<=a.end:continue
    weight=resolve(row.find('weight'));weight=int(weight.text)/1e6 if weight is not None else 1
    thread=resolve(row.find('thread'));main='Main Thread' in thread.get('fmt','') if thread is not None else False
    total['main' if main else 'other']+=weight
    stack=resolve(row.find('tagged-backtrace'))
    if stack is None:continue
    names=[resolve(f).get('name','') for f in stack]
    if main:
        inclusive.update({n:weight for n in set(names)})
        for group,pattern in patterns.items():
            if any(pattern in n for n in names):groups[group]+=weight
        if names:leaf[names[0]]+=weight
print(json.dumps({'startSeconds':a.start,'sampleWeightMs':total,'inclusiveMainGroupsMs':groups,'inclusiveMainMs':inclusive.most_common(60),'leafMainMs':leaf.most_common(40)},indent=2))
