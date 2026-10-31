#!/usr/bin/env python3
"""Cross-platform, source-available ArSonVerb packaging. Python stdlib only.

Never alters binaries, clears OS quarantine, bypasses security mechanisms or
re-signs executable code. Fails closed if any VST3/Standalone product is missing.
"""
from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tarfile
import tempfile
import zipfile

VERSION = "1.0.1"
ROOT = Path(__file__).resolve().parent.parent
PLATFORM_LABEL = {"windows": "Windows-x64", "linux": "Linux-x86_64",
                  "macos": "macOS-universal"}


def locate(root: Path, pattern: str, kind: str, parent: str) -> Path:
    candidates = [path for path in root.rglob(pattern)
                  if (path.is_dir() if kind == "dir" else path.is_file())
                  and parent in path.parts]
    if len(candidates) != 1:
        raise RuntimeError(f"Expected exactly one {pattern} in {parent}, got: {candidates}")
    return candidates[0]


def add_file_or_tree(src: Path, dest: Path) -> None:
    if src.is_dir():
        shutil.copytree(src, dest, symlinks=True)
    else:
        shutil.copy2(src, dest)


def package_notice(stage: Path, platform: str, flavor: str) -> None:
    shutil.copy2(ROOT / "LICENSE", stage / "LICENSE")
    shutil.copy2(ROOT / "assets/fonts/OFL.txt", stage / "INTER-OFL.txt")
    shutil.copy2(ROOT / "docs/RELEASE_V1.0.1.md", stage / "RELEASE_NOTES.md")
    instructions = {
        "windows": ("VST3: copy ArSonVerb.vst3 to "
                    "%LOCALAPPDATA%\\Programs\\Common\\VST3\\. "
                    "Standalone: run ArSonVerb.exe. "
                    "The optional Inno Setup installer can install both without admin rights."),
        "linux": ("VST3: mkdir -p ~/.vst3 && cp -a ArSonVerb.vst3 ~/.vst3/ . "
                  "Standalone: run ./ArSonVerb (requires supported system audio and GUI libraries)."),
        "macos": ("VST3: copy ArSonVerb.vst3 to "
                  "~/Library/Audio/Plug-Ins/VST3/ . "
                  "Standalone: move ArSonVerb.app to ~/Applications/ . "
                  "Unsigned/not notarized: macOS may require explicit consent under "
                  "System Settings > Privacy & Security; never disable Gatekeeper."),
    }
    (stage / "INSTALL.txt").write_text(
        f"ArSonVerb {VERSION} ({platform}, {flavor})\n"
        "AGPL-3.0-only; free/open source. Official source:\n"
        "https://github.com/masarray/arsonverb/releases/tag/v1.0.1\n\n"
        + instructions[platform] + "\n"
        "Verify SHA256SUMS.txt from the official GitHub Release before installation.\n"
        "Do not run downloaded software unless you trust the publisher and source.\n",
        encoding="utf-8",
    )


def zip_tree(root: Path, destination: Path) -> None:
    if os.name == "posix" and shutil.which("ditto") and str(destination).endswith(".zip"):
        subprocess.run(["ditto", "-c", "-k", "--sequesterRsrc",
                        str(root), str(destination)], check=True)
        return
    with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED, compresslevel=8) as archive:
        for path in sorted(root.rglob("*")):
            if path.is_file():
                archive.write(path, path.relative_to(root))


def tar_tree(root: Path, destination: Path) -> None:
    with tarfile.open(destination, "w:gz", format=tarfile.PAX_FORMAT) as archive:
        for path in sorted(root.iterdir()):
            archive.add(path, arcname=path.name, recursive=True)


def checksum(out: Path, platform: str) -> None:
    filenames = sorted(p for p in out.iterdir() if p.is_file()
                       and not p.name.startswith("SHA256SUMS"))
    if not filenames:
        raise RuntimeError("No build artifacts to checksum")
    with (out / f"SHA256SUMS-{platform}.txt").open("w", encoding="ascii") as f:
        for path in filenames:
            with path.open("rb") as source:
                digest = hashlib.file_digest(source, "sha256").hexdigest()
            f.write(f"{digest}  {path.name}\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", choices=PLATFORM_LABEL, required=True)
    parser.add_argument("--build", type=Path, default=Path("build-distribution"))
    parser.add_argument("--out", type=Path, default=Path("release-assets"))
    parser.add_argument("--checksums-only", action="store_true")
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    if args.checksums_only:
        checksum(out, args.platform)
        return
    root = (args.build / "ARStudioReverb_artefacts").resolve()
    plugin = locate(root, "ArSonVerb.vst3", "dir", "VST3")
    standalone_name = {"windows": "ArSonVerb.exe",
                       "linux": "ArSonVerb", "macos": "ArSonVerb.app"}[args.platform]
    standalone = locate(root, standalone_name,
                        "dir" if args.platform == "macos" else "file", "Standalone")
    if args.platform == "linux" and not os.access(standalone, os.X_OK):
        raise RuntimeError(f"Standalone is not executable: {standalone}")
    label = PLATFORM_LABEL[args.platform]
    suffix = ".tar.gz" if args.platform == "linux" else ".zip"
    for flavor, binary in (("VST3", plugin), ("Standalone", standalone)):
        with tempfile.TemporaryDirectory(prefix="arsonverb-pack-") as folder:
            stage = Path(folder)
            add_file_or_tree(binary, stage / binary.name)
            package_notice(stage, args.platform, flavor)
            destination = out / f"ArSonVerb-{label}-{flavor}-v{VERSION}{suffix}"
            if args.platform == "linux":
                tar_tree(stage, destination)
            else:
                zip_tree(stage, destination)
            if not destination.is_file() or destination.stat().st_size < 1024:
                raise RuntimeError(f"Empty package {destination}")
            print(f"PACKAGED: {destination} ({destination.stat().st_size} bytes)")
    if args.platform == "windows":
        stage = Path("package-stage")
        stage.mkdir(parents=True, exist_ok=True)
        add_file_or_tree(plugin, stage / plugin.name)
        add_file_or_tree(standalone, stage / standalone.name)
    else:
        checksum(out, args.platform)


if __name__ == "__main__":
    main()
