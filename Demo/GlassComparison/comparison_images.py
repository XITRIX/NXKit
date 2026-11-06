"""Color-managed image loading shared by native/raster comparisons.

Original captures are never changed. Embedded native ICC profiles are converted
through LittleCMS to sRGB; untagged raster output is explicitly treated as sRGB.
"""
from functools import lru_cache
from io import BytesIO
from PIL import Image, ImageCms

_SRGB=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB'))
@lru_cache(maxsize=8)
def _source_profile(data):
    return ImageCms.ImageCmsProfile(BytesIO(data))

def load_srgb(path):
    image=Image.open(path)
    profile=image.info.get('icc_profile')
    if profile:
        return ImageCms.profileToProfile(image,_source_profile(profile),_SRGB,outputMode='RGB')
    return image.convert('RGB')

COLOR_MANAGEMENT='Embedded native ICC → sRGB with LittleCMS; untagged raster is sRGB; originals preserved.'
