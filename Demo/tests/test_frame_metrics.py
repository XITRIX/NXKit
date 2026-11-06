import importlib.util
from pathlib import Path
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('frame_metrics',Path(__file__).parents[1]/'scripts/analyze_frame_metrics.py')
metrics=importlib.util.module_from_spec(spec)
spec.loader.exec_module(metrics)


class FrameMetricsTests(unittest.TestCase):
    def test_uses_presented_time_instead_of_cpu_throughput_and_resolves_async_rows(self):
        # Tiny CPU work must not be reported as 1000 FPS. GPU and presentation
        # callbacks can arrive out of order relative to their CPU frame rows.
        content='''# max_hz=120
kind,frame,time_s,phase,total_ms,layout_ms,render_ms,submit_ms,acquire_ms,gpu_ms,presented,thermal,layers
gpu,2,1.008,,,,,,,4,1,,
cpu,1,1,scroll,1,.1,.3,.4,.2,,1,0,10
cpu,2,1.008,scroll,1,.1,.3,.4,.2,,1,0,10
cpu,3,1.016,scroll,1,.1,.3,.4,.2,,1,0,10
present,3,1.016666667,,,,,,,0,1,,
present,1,1.000000000,,,,,,,0,1,,
present,2,1.008333333,,,,,,,0,1,,
present,4,0,,,,,,,0,0,,
cpu,4,2,idle,.1,.1,0,0,0,,0,0,0
'''
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'frames.csv'
            path.write_text(content)
            result=metrics.analyze(path)
        scroll=result['phases']['scroll']
        self.assertAlmostEqual(scroll['presented_fps'],120,places=4)
        self.assertEqual(scroll['presented_frames'],3)
        self.assertEqual(scroll['gpu_command_buffer_ms']['p50'],4)
        self.assertEqual(scroll['intervals_over_8_4_ms'],0)
        self.assertEqual(scroll['total_ms']['p50'],1)
        self.assertIsNone(result['phases']['idle']['presented_fps'])
        self.assertEqual(result['phases']['idle']['rendered_frames'],0)

    def test_sustained_windows_keep_thermal_and_presentation_samples_together(self):
        content='kind,frame,time_s,phase,total_ms,layout_ms,render_ms,submit_ms,acquire_ms,gpu_ms,presented,thermal,layers\n'
        for frame in range(60):
            content+=f'cpu,{frame},{100+frame},scroll-second,1,0,1,0,0,,1,{0 if frame<30 else 2},1\n'
            content+=f'present,{frame},{100+frame},,,,,,,0,1,,\n'
        content+='# gpu-pass,120,0,1290,200,0.1,1.2\n'
        content+='# pipeline_failures=0\n# pipeline_failures=3\n'
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'frames.csv';path.write_text(content)
            result=metrics.analyze(path);windows=result['sustained_windows_30s']
        self.assertEqual(result['pipeline_failures'],3)
        self.assertEqual(len(windows),2)
        self.assertEqual([w['presented_fps'] for w in windows],[1,1])
        self.assertEqual(windows[0]['thermal_counts'],{'0':30})
        self.assertEqual(windows[1]['thermal_counts'],{'2':30})


if __name__=='__main__':
    unittest.main()
