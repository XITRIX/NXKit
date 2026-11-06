"""Capture real simulator framebuffer, cropped only from observed scene geometry.
Usage: python capture-shadows.py DEVICE OUTPUT CASES_JSON
"""
from pathlib import Path
import json,subprocess,sys,time
from PIL import Image,ImageCms
import io
import shutil
import hashlib

device,out=sys.argv[1],Path(sys.argv[2]).resolve();native=out/'native';native.mkdir(exist_ok=True)
def sim(*args):return subprocess.check_output(['xcrun','simctl',*args],text=True).strip()
bundle='org.nxkit.ShadowReference'
sim('install',device,str(out/'ShadowReference.app'));sim('launch','--terminate-running-process',device,bundle)
documents=Path(sim('get_app_container',device,bundle,'data'))/'Documents'
cases=json.loads(Path(sys.argv[3]).read_text())
records=[]
executableHash=hashlib.sha256((out/'ShadowReference.app/ShadowReference').read_bytes()).hexdigest()
sourceHash=hashlib.sha256((Path(__file__).parent/'ShadowReference.m').read_bytes()).hexdigest()
for request in cases:
   name=request['id']; background=request['background']
   staged=documents/'request.tmp';staged.write_text(json.dumps(request));staged.replace(documents/'request.json')
   deadline=time.monotonic()+20
   while True:
    try:m=json.loads((documents/'capture.json').read_text())
    except (FileNotFoundError,json.JSONDecodeError):m={}
    if m.get('request')==request:break
    if time.monotonic()>deadline:raise RuntimeError(name+' timed out')
    time.sleep(.03)
   # The acknowledgement is emitted after 1.1s. One further compositor interval avoids stale frames.
   time.sleep(.06)
   screen=native/(name+'-screen.png');sim('io',device,'screenshot',str(screen))
   image=Image.open(screen);scale=m['scale'];x,y=m['x']*scale,m['y']*scale
   if m['width']!=720 or m['height']!=440 or scale!=2:raise RuntimeError('Unexpected scene dimensions')
   crop=image.crop((x,y,x+1440,y+880)).convert('RGB')
   profile=image.info.get('icc_profile')
   if not profile and image.info.get('srgb') is None:raise RuntimeError('Framebuffer PNG has no explicit color-space declaration')
   raw=native/'raw';raw.mkdir(exist_ok=True)
   shutil.copy2(screen,raw/(name+'-framebuffer.png'))
   srgbProfile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
   crop.save(raw/(name+'.png'),icc_profile=profile or srgbProfile)
   srgb=ImageCms.profileToProfile(crop,ImageCms.ImageCmsProfile(io.BytesIO(profile)),ImageCms.createProfile('sRGB'),outputMode='RGB') if profile else crop
   srgb.save(native/(name+'.png'),icc_profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes())
   m['captureColorSpace']='Embedded ICC converted to sRGB' if profile else 'Framebuffer PNG sRGB chunk (rendering intent '+str(image.info['srgb'])+')'
   m['framebufferColorMetadata']=dict(hasICC=bool(profile),srgbChunk=image.info.get('srgb'));screen.unlink()
   m['nativeExecutableSHA256']=executableHash;m['nativeFixtureSourceSHA256']=sourceHash;m['backgroundAssetSHA256']=hashlib.sha256((out/'ShadowReference.app'/(background+'.png')).read_bytes()).hexdigest()
   (native/(name+'.json')).write_text(json.dumps(m,indent=2)+'\n');records.append(m)
   (native/'series.json').write_text(json.dumps(records,indent=2)+'\n')
   print(name,flush=True)
