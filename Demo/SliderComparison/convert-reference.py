"""Color-convert native P3 references to sRGB before comparing pixel values."""
from pathlib import Path
from PIL import Image,ImageCms
import hashlib,io,json,sys
native=Path(sys.argv[1]).resolve();output=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
target=ImageCms.createProfile('sRGB');target_bytes=ImageCms.ImageCmsProfile(target).tobytes();rows=[]
files=list((native/'calibration/thumb-snapshots').glob('*.png'))+list((native/'calibration/thumb-snapshots-dark').glob('*.png'))+list((native/'velocity/custom-thumb').glob('*.png'))
for path in sorted(files):
    image=Image.open(path);profile=ImageCms.ImageCmsProfile(io.BytesIO(image.info['icc_profile']))
    rgb=ImageCms.profileToProfile(image.convert('RGB'),profile,target,outputMode='RGB');rgba=rgb.convert('RGBA');rgba.putalpha(image.getchannel('A'))
    destination=output/path.relative_to(native);destination.parent.mkdir(parents=True,exist_ok=True);rgba.save(destination,icc_profile=target_bytes)
    rows.append({'source':str(path.relative_to(native)),'sourceSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),'sourceProfile':ImageCms.getProfileName(profile).strip(),'output':str(destination.relative_to(output)),'outputSHA256':hashlib.sha256(destination.read_bytes()).hexdigest(),'outputProfile':'sRGB','sizePixels':image.size,'alphaPreserved':True})
(output/'manifest.json').write_text(json.dumps(rows,indent=2)+'\n');print(f'Converted {len(rows)} native reference images to sRGB')
