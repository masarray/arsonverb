#!/usr/bin/env python3
"""Create a transparent, ordinary Windows installer ZIP for GitHub Releases.

The original published EXE is never modified. SHA-256 matching is required;
the packaging intentionally uses regular ZIP, without encryption, scripts,
obfuscation, executables renamed to non-executable extensions, or AV bypass.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import zipfile

PRODUCT_VERSION = "v1.0.1"
SETUP_NAME = "ArSonVerb-Windows-x64-Setup-v1.0.1.exe"
ZIP_NAME = "ArSonVerb-Windows-x64-Installer-v1.0.1.zip"


def digest(path: Path) -> str:
    sha = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            sha.update(block)
    return sha.hexdigest()


def build_package(source: Path, destination: Path, expected_sha256: str,
                  project_root: Path) -> str:
    if source.name != SETUP_NAME or not source.is_file():
        raise ValueError(f"Expected official {SETUP_NAME}: {source}")
    if len(expected_sha256) != 64 or not all(c in "0123456789abcdef" for c in expected_sha256.lower()):
        raise ValueError("Expected SHA-256 must be a 64-character hex string")
    actual = digest(source)
    if actual != expected_sha256.lower():
        raise ValueError(f"Official EXE digest mismatch: expected {expected_sha256}, got {actual}")
    if source.stat().st_size < 1024 * 1024:
        raise ValueError("Installer EXE is unexpectedly small")
    with source.open("rb") as handle:
        if handle.read(2) != b"MZ":
            raise ValueError("Installer EXE does not have a Windows PE header")

    files = {
        SETUP_NAME: source,
        "LICENSE.txt": project_root / "LICENSE",
        "INTER-OFL.txt": project_root / "assets/fonts/OFL.txt",
        "RELEASE_NOTES.md": project_root / "docs/RELEASE_V1.0.1.md",
    }
    for name, path in files.items():
        if not path.is_file() or path.stat().st_size == 0:
            raise ValueError(f"Missing required package file {name}: {path}")

    instructions = (
        "ArSonVerb v1.0.1 — Windows x64 installation\n"
        "==================================================\n\n"
        "1. Extract this ZIP normally (do not run from inside the archive).\n"
        f"2. Double-click {SETUP_NAME}.\n"
        "3. Choose VST3 for your DAW, Standalone, or both.\n"
        "4. Installation is per-user and needs no administrator rights.\n"
        "5. Restart your DAW and rescan its VST3 plug-ins if needed.\n\n"
        "Prefer no installer? Download the smaller VST3-only ZIP:\n"
        "https://github.com/masarray/arsonverb/releases/download/v1.0.1/"
        "ArSonVerb-Windows-x64-VST3-v1.0.1.zip\n\n"
        "Official release and corresponding AGPL-3.0 source:\n"
        "https://github.com/masarray/arsonverb/releases/tag/v1.0.1\n"
        "Installer EXE SHA-256 is in SHA256-INSTALLER-EXE.txt.\n"
        "The archive is standard ZIP (not encrypted) and is not a security bypass.\n"
    )
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        raise FileExistsError(f"Refuse to replace existing archive: {destination}")
    with zipfile.ZipFile(destination, "w", compression=zipfile.ZIP_DEFLATED,
                         compresslevel=9, allowZip64=True) as archive:
        for name, path in sorted(files.items()):
            archive.write(path, arcname=name)
        archive.writestr("INSTALL-FIRST.txt", instructions)
        archive.writestr("SHA256-INSTALLER-EXE.txt", f"{actual}  {SETUP_NAME}\n")
    with zipfile.ZipFile(destination) as archive:
        if archive.testzip() is not None:
            raise ValueError("Archive integrity check failed")
        assert archive.namelist().count(SETUP_NAME) == 1
        assert len(archive.namelist()) == 6
    return digest(destination)


def main() -> None:
    parser = argparse.ArgumentParser(description="Repack an already validated ArSonVerb Windows installer")
    parser.add_argument("--installer", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--expected-sha256", required=True)
    parser.add_argument("--project-root", type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    actual = build_package(args.installer, args.out, args.expected_sha256, args.project_root)
    print(f"PACKAGED {args.out}: sha256 {actual}")


if __name__ == "__main__":
    main()
