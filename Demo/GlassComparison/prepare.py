"""Prepare one identical, point-aligned background for both renderers (Pillow)."""
from pathlib import Path
import sys
from PIL import Image

output = Path(sys.argv[1])
output.mkdir(parents=True, exist_ok=True)
artwork = Path(__file__).resolve().parents[1] / "resources/img/glass_demo_background.png"
scene = Image.open(artwork).convert("RGB")
factor = max(720 / scene.width, 440 / scene.height)
scene = scene.resize((round(scene.width * factor), round(scene.height * factor)), Image.Resampling.BICUBIC)
left, top = (scene.width - 720) // 2, (scene.height - 440) // 2
scene = scene.crop((left, top, left + 720, top + 440))
scene.save(output / "scene.png")
(output / "scene.rgb").write_bytes(scene.tobytes())
