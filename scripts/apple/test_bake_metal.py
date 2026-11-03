import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec=importlib.util.spec_from_file_location('bake',Path(__file__).with_name('bake_metal.py'))
bake=importlib.util.module_from_spec(spec);spec.loader.exec_module(bake)

class CatalogValidation(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory();self.root=Path(self.temporary.name)
        (self.root/'sources').mkdir()
        source=b'vertex float4 vertexMain(){return float4(0);}'
        self.key=hashlib.sha256(source).hexdigest()
        (self.root/'sources'/f'{self.key}.metal').write_bytes(source)
        self.manifest={'abi':'a'*64,'languageVersion':'metal2.3'}
        self.script={'libraries':{'paths':[{'label':self.key,'path':'ignored-build-path'}]},
                     'pipelines':{'render_pipelines':[{'vertex_function':f'alias:{self.key}#vertexMain'}]},'exportFailed':False}
        self.write()
    def tearDown(self):self.temporary.cleanup()
    def write(self):
        (self.root/'manifest.json').write_text(json.dumps(self.manifest))
        (self.root/'pipelines.mtlp-json').write_text(json.dumps(self.script))
    def test_stencil_only_catalog_is_accepted(self):
        self.assertEqual(len(bake.inputs(self.root)[1]),1)
    def test_source_tampering_rejected(self):
        next((self.root/'sources').glob('*.metal')).write_text('changed source')
        with self.assertRaisesRegex(ValueError,'hash mismatch'):bake.inputs(self.root)
    def test_unknown_function_alias_rejected(self):
        self.script['pipelines']['render_pipelines'][0]['vertex_function']='alias:'+'b'*64+'#vertexMain';self.write()
        with self.assertRaisesRegex(ValueError,'function alias'):bake.inputs(self.root)
    def test_incomplete_harvest_rejected(self):
        self.script['exportFailed']=True;self.write()
        with self.assertRaisesRegex(ValueError,'incomplete'):bake.inputs(self.root)
    def test_stale_abi_rejected_before_tools_run(self):
        with patch.object(bake,'tool',side_effect=AssertionError('compiler must not run')):
            with self.assertRaisesRegex(ValueError,'ABI differs'):
                bake.bake(self.root,self.root/'out','macosx','air64-apple-macos13.0','apple7','b'*64)

class BundleEmbedding(unittest.TestCase):
    def test_xcode_app_destination_ignores_utility_products_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);source=root/'baked';source.mkdir()
            (source/'manifest.json').write_text('{}')
            destination=root/'Debug${EFFECTIVE_PLATFORM_NAME}'/'App.app'/'NXKitMetalShaders'
            destination_file=root/'destination.txt';destination_file.write_text(str(destination)+'\n')
            environment=dict(os.environ,EFFECTIVE_PLATFORM_NAME='-iphoneos',BUILT_PRODUCTS_DIR=str(root/'wrong-utility-directory'))
            subprocess.run(['/bin/sh',str(Path(__file__).with_name('embed_metal.sh')),str(source),str(destination_file)],env=environment,check=True)
            self.assertTrue((root/'Debug-iphoneos'/'App.app'/'NXKitMetalShaders'/'manifest.json').is_file())
            self.assertFalse(destination.exists())

if __name__=='__main__':unittest.main()
