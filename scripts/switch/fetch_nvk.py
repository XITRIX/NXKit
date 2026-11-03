#!/usr/bin/env python3
"""Install a checksummed, pinned Horizon NVK SDK inside the checkout."""
import hashlib
from pathlib import Path
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
TAG = "nvk-switch-mesa-25.0.7-r1"
ASSET = "nvk-switch-25.0.7.tar.xz"
SHA256 = "4ab244f982dc078aaff8c58640d7cf8a0f3ab43bbf2daadfd00c02fbf352af86"
LIB_SHA256 = "ff65a74b46fe749f6be73eac3c8968d70e312c7646f815f26b69f4aa35737d26"


def main():
    destination = ROOT / "Submodules/nvk-switch"
    destination.mkdir(exist_ok=True)
    library = destination / "lib/libvulkan.a"
    if library.exists() and hashlib.sha256(library.read_bytes()).hexdigest() == LIB_SHA256:
        print(f"NVK {TAG} already installed and verified")
        return
    archive = destination / ASSET
    if not archive.exists() or hashlib.sha256(archive.read_bytes()).hexdigest() != SHA256:
        url = f"https://github.com/jhuz0/nvk-switch/releases/download/{TAG}/{ASSET}"
        with urllib.request.urlopen(url, timeout=120) as response:
            archive.write_bytes(response.read())
    if hashlib.sha256(archive.read_bytes()).hexdigest() != SHA256:
        raise RuntimeError("NVK download checksum mismatch")
    with tarfile.open(archive) as package:
        package.extractall(destination, filter="data")
    if hashlib.sha256(library.read_bytes()).hexdigest() != LIB_SHA256:
        raise RuntimeError("Extracted NVK library checksum mismatch")
    print(f"Installed NVK {TAG}; attribution: {destination / 'NOTICE.md'}")


if __name__ == "__main__":
    main()
