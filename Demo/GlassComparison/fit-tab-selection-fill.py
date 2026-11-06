"""Native tab selection fill, isolated from the foreground tint transfer.
Usage: fit-tab-selection-fill.py NATIVE_DIRECTORY OUTPUT HEADER
Green is held out. Input/output are neighboring native material pixels.
"""
from pathlib import Path
import json,sys
import numpy as np
from PIL import Image
native,out,header=map(Path,sys.argv[1:4]);rows=[];profiles=[]
for dark in [0,1]:
 group=[]
 for f in sorted(native.glob(f'tabs-d{dark}-*.json')):
  d=json.loads(f.read_text());a=np.array(Image.open(f.with_suffix('.png')),float)/255;x,y,w,h=d['tabs'][0]['images'][1];scale=d['scale'];base=a[round((y+h/2)*scale),round(75*scale),:3];selected=a[round((y+h/2)*scale),round((x-5)*scale),:3];group.append(dict(case=f.stem,dark=bool(dark),input=base.tolist(),output=selected.tolist(),validation='green' in f.stem))
 train=[r for r in group if not r['validation']];matrix=np.linalg.lstsq(np.array([[*r['input'],1] for r in train]),np.array([r['output'] for r in train]),rcond=None)[0].T;profiles.append(matrix.tolist())
 for r in group:r['predicted']=np.clip(matrix@np.r_[r['input'],1],0,1).tolist();r['MAE255']=float(abs(np.array(r['predicted'])-r['output']).mean()*255)
 rows+=group
(out/'selection-fill.json').write_text(json.dumps(dict(method=__doc__,profiles=profiles,samples=rows),indent=2)+'\n')
text='''#pragma once
// Generated from public UIKit tab-bar pixels; see selection-fill.json.
// A Demo role-specific resting surface, not LensView's caller-owned default.
#include <BackdropEffectView.h>
namespace {
class TabSelectionBackground final:public NXKit::BackdropEffectView {
public:
    TabSelectionBackground():BackdropEffectView(material(false)) {setUserInteractionEnabled(false);refresh();}
    void traitCollectionDidChange(std::shared_ptr<NXKit::UITraitCollection> previous) override {
        BackdropEffectView::traitCollectionDidChange(std::move(previous));
        refresh();
    }
private:
    void refresh() {
        const bool dark=traitCollection()&&traitCollection()->userInterfaceStyle()==NXKit::UIUserInterfaceStyle::dark;
        if(dark==_dark)return;_dark=dark;setEffect(material(dark));
    }
    bool _dark=false;
    static NXKit::BackdropEffect material(bool dark) {
        NXKit::BackdropEffect effect(R"sksl(
uniform shader content;
uniform float filterScale;
uniform float2 resolution;
uniform float4 redRow,greenRow,blueRow;
half4 main(float2 xy) {
    half4 pixel=content.eval(clamp(xy,float2(0),resolution*max(filterScale,1.0)));
    float4 inputColor=float4(pixel.rgb/max(pixel.a,.0001),1);
    half3 color=half3(clamp(float3(dot(redRow,inputColor),dot(greenRow,inputColor),dot(blueRow,inputColor)),0.0,1.0));
    return half4(color*pixel.a,pixel.a);
})sksl");
'''
for i,name in enumerate(['redRow','greenRow','blueRow']):
 v=lambda row:'std::array<NXKit::NXFloat,4>{'+','.join(f'{x:.9f}f' for x in row)+'}'
 text+=f'        effect.setUniform("{name}",dark?'+v(profiles[1][i])+':'+v(profiles[0][i])+');\n'
text+='''        return effect;
    }
};
}
''';header.write_text(text);print([(r['case'],r['MAE255']) for r in rows if r['validation']])
