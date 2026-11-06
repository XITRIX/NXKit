#!/usr/bin/env python3
"""Extract native slider video crops and correlate committed marker IDs to traces.

Requires ffmpeg/ffprobe, Pillow and numpy. No Simulator or UIKit process is run.
The input recording is Display P3, not sRGB; zscale performs an explicit gamut
conversion before PNG encoding. These compressed-video crops are visual
evidence, not lossless screenshots for channel-error measurement.
"""
import argparse
import gzip
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

import numpy as np
from PIL import Image, ImageCms


FILTER = (
    "crop=1020:486:180:510,"
    "zscale=primariesin=smpte432:transferin=iec61966-2-1:matrixin=bt709:"
    "rangein=limited:primaries=bt709:transfer=iec61966-2-1:matrix=gbr:range=full,"
    "format=gbrpf32le,format=rgb24"
)
CASES = {
    "stationary": ["stationary-light", "stationary-dark"],
    "interruption": ["tap-light", "tap-dark", "interrupted-light", "interrupted-dark"],
}


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for b in iter(lambda: f.read(1024 * 1024), b""):
            h.update(b)
    return h.hexdigest()


def probe(path, entries):
    return json.loads(subprocess.check_output([
        "ffprobe", "-v", "error", "-select_streams", "v:0", "-show_entries",
        entries, "-of", "json", str(path),
    ]))


def load_trace(root, recording, case):
    path = root / recording / case / "trace.json"
    if path.exists():
        raw = path.read_bytes()
    else:
        path = path.with_suffix(".json.gz")
        raw = gzip.decompress(path.read_bytes())
    return path, raw, json.loads(raw)


def extract(source, output, retain_sources, trace_root=None):
    trace_root = trace_root or source
    output.mkdir(parents=True, exist_ok=True)
    profile = ImageCms.ImageCmsProfile(ImageCms.createProfile("sRGB")).tobytes()
    result = {"schema": 1, "filter": FILTER, "cropPixels": [180, 510, 1020, 330],
              "cropInRootPoints": [60, 170, 340, 110], "scale": 3,
              "markerInRootPoints": [80, 320, 128, 12], "recordings": [],
              "phaseLimitation": "Marker is committed after the sampled display callback. "
              "Trace and visible presentation may differ by about one compositor frame; "
              "ID correlation is not a zero-latency render timestamp.",
              "colorLimitation": "P3 compressed video explicitly converted to sRGB. "
              "Clipping, chroma subsampling and encoder error make these visual-only crops; "
              "do not use them as lossless screenshot pixel-error ground truth."}
    for recording, names in CASES.items():
        video = source / recording / "native-motion.mp4"
        if not video.exists():
            video = source / (recording + ".mp4")
        stream = probe(video, "stream=width,height,color_range,color_space,color_transfer,"
                       "color_primaries,r_frame_rate,avg_frame_rate,duration,nb_frames")["streams"][0]
        assert [stream["width"], stream["height"]] == [1320, 2868], stream
        assert (stream["color_primaries"], stream["color_transfer"], stream["color_space"],
                stream["color_range"]) == ("smpte432", "iec61966-2-1", "bt709", "tv"), stream
        timing = probe(video, "frame=best_effort_timestamp_time")["frames"]
        record = {"id": recording, "sourceSHA256": digest(video), "stream": stream,
                  "cases": [], "decodedFrameCount": 0}
        if retain_sources:
            copy = output / (recording + ".mp4")
            if video.resolve() != copy.resolve():
                shutil.copyfile(video, copy)
            record["source"] = copy.name
        else:
            record["source"] = str(video.resolve())
        # setpts only avoids rawvideo's artificial output-time-base warnings;
        # original per-frame video timestamps come independently from ffprobe.
        cmd = ["ffmpeg", "-v", "error", "-i", str(video), "-vf", FILTER + ",setpts=N",
               "-fps_mode", "passthrough", "-enc_time_base", "1:600", "-f", "rawvideo", "-"]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        current = None
        old_id = 0
        previous = None
        index = 0
        segment = -1
        case_dir = None
        trace_frames = None
        seen = set()
        while True:
            raw = proc.stdout.read(1020 * 486 * 3)
            if not raw:
                break
            if len(raw) != 1020 * 486 * 3:
                raise RuntimeError("Truncated raw frame")
            frame = np.frombuffer(raw, dtype=np.uint8).reshape(486, 1020, 3)
            # Marker bit centers in the decoded crop: x72+24*i, y468.
            levels = [float(frame[466:471, 70 + 24 * i:75 + 24 * i].mean()) for i in range(16)]
            word = sum(int(v > 128) << i for i, v in enumerate(levels))
            # App-launch/dismiss frames can put arbitrary UI under the marker.
            # Only an unambiguous white active bit and binary data are valid.
            active = levels[15] >= 205 and all(v <= 60 or v >= 205 for v in levels)
            frame_id = word & 0x7fff
            if active:
                if current is None or frame_id < old_id:
                    segment += 1
                    if segment >= len(names):
                        raise RuntimeError("Unexpected extra active marker segment")
                    name = names[segment]
                    trace_path, trace_raw, trace = load_trace(trace_root, recording, name)
                    assert trace["configuration"]["id"] == name
                    trace_frames = {f["frameID"]: f for f in trace["frames"] if f["frameID"] > 0}
                    case_dir = output / name
                    case_dir.mkdir(exist_ok=True)
                    current = {"id": name, "traceSHA256": hashlib.sha256(trace_raw).hexdigest(),
                               "retainedTrace": os.path.relpath(trace_path, output),
                               "firstVideoFrame": index, "frames": []}
                    record["cases"].append(current)
                    seen = set()
                    if previous is not None:
                        pre_name = "precontact.png"
                        Image.fromarray(previous[:330]).save(case_dir / pre_name, icc_profile=profile)
                        current["precontact"] = {"image": name + "/" + pre_name,
                                                 "videoFrame": index - 1,
                                                 "videoTime": float(timing[index - 1]["best_effort_timestamp_time"])}
                assert frame_id in trace_frames, (recording, segment, frame_id)
                sampled = trace_frames[frame_id]
                event = {"videoFrame": index, "videoTime": float(timing[index]["best_effort_timestamp_time"]),
                         "markerID": frame_id, "traceTime": sampled["t"],
                         "displayTimestamp": sampled["displayTimestamp"],
                         "targetTimestamp": sampled["targetTimestamp"],
                         "tracking": sampled["tracking"], "highlighted": sampled["highlighted"],
                         "value": sampled["value"],
                         "contactRelativeTime": sampled["t"] - trace["actions"][0]["t"]}
                if frame_id not in seen:
                    filename = f"frame-{frame_id:04d}.png"
                    Image.fromarray(frame[:330]).save(case_dir / filename, icc_profile=profile)
                    seen.add(frame_id)
                event["image"] = name + f"/frame-{frame_id:04d}.png"
                current["frames"].append(event)
                current["lastVideoFrame"] = index
                old_id = frame_id
            else:
                current = None
                old_id = 0
            previous = frame.copy()
            index += 1
        if proc.wait() != 0:
            raise RuntimeError("ffmpeg decode failed")
        assert index == len(timing), (index, len(timing))
        assert len(record["cases"]) == len(names)
        record["decodedFrameCount"] = index
        for case in record["cases"]:
            ids = {f["markerID"] for f in case["frames"]}
            _, _, trace = load_trace(trace_root, recording, case["id"])
            traced = {f["frameID"] for f in trace["frames"] if f["frameID"] > 0}
            case["distinctObservedMarkerIDs"] = len(ids)
            case["missingTraceMarkerIDs"] = sorted(traced - ids)
            case["duplicateEncodedMarkerFrames"] = len(case["frames"]) - len(ids)
        result["recordings"].append(record)
    result["imageSHA256"] = {str(p.relative_to(output)): digest(p) for p in sorted(output.glob("*/*.png"))}
    (output / "manifest.json").write_text(json.dumps(result, indent=2) + "\n")
    for r in result["recordings"]:
        print(r["id"], r["decodedFrameCount"], "decoded frames")
        for c in r["cases"]:
            print(" ", c["id"], c["distinctObservedMarkerIDs"], "unique IDs;",
                  len(c["missingTraceMarkerIDs"]), "missing;",
                  c["duplicateEncodedMarkerFrames"], "duplicate frames")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("source", type=Path, help="Directory containing stationary/ and interruption/")
    p.add_argument("output", type=Path)
    p.add_argument("--retain-sources", action="store_true")
    p.add_argument("--traces", type=Path, help="Separate root for recording/case/trace.json[.gz]")
    a = p.parse_args()
    extract(a.source, a.output, a.retain_sources, a.traces)
