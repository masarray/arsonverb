"""Offline package format tests: no JUCE builds, no system installs, no DAW."""
import hashlib
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import zipfile

SCRIPT = Path(__file__).resolve().parent.parent / "scripts" / "package_distribution.py"
VERSION = "v1.0.1"


def test(platform: str, temp: Path) -> None:
    base = temp / platform
    tree = base / "build" / "ARStudioReverb_artefacts" / "Release"
    vst3 = tree / "VST3" / "ArSonVerb.vst3" / "Contents"
    vst3.mkdir(parents=True)
    (vst3 / "fake-binary").write_bytes(b"compiled-test-fixture")
    standalone = tree / "Standalone"
    standalone.mkdir()
    if platform == "macos":
        app = standalone / "ArSonVerb.app" / "Contents" / "MacOS"
        app.mkdir(parents=True)
        (app / "ArSonVerb").write_bytes(b"fake-mach-o")
    else:
        exe = standalone / ("ArSonVerb.exe" if platform == "windows" else "ArSonVerb")
        exe.write_bytes(b"fake-native-binary")
        exe.chmod(0o755)
    subprocess.run([sys.executable, str(SCRIPT), "--platform", platform,
                    "--build", str(base / "build"), "--out", str(base / "assets")],
                   cwd=base, check=True)
    if platform == "windows":
        subprocess.run([sys.executable, str(SCRIPT), "--platform", platform,
                        "--out", str(base / "assets"), "--checksums-only"],
                       cwd=base, check=True)
    assets = base / "assets"
    packages = [p for p in assets.iterdir() if p.name.startswith("ArSonVerb-")]
    assert len(packages) == 2, packages
    checksums = (assets / f"SHA256SUMS-{platform}.txt").read_text().splitlines()
    assert len(checksums) == 2, checksums
    for item in packages:
        assert item.stat().st_size > 1024, item
        assert VERSION in item.name, item
        if item.suffix == ".zip":
            with zipfile.ZipFile(item) as archive:
                members = archive.namelist()
                for path in ("LICENSE", "INTER-OFL.txt", "INSTALL.txt", "RELEASE_NOTES.md"):
                    assert any(m.endswith(path) for m in members), (item, path)
                assert any("ArSonVerb.vst3/" in p for p in members) if "-VST3-" in item.name else True
        elif item.name.endswith(".tar.gz"):
            with tarfile.open(item, "r:gz") as archive:
                members = archive.getnames()
                assert "LICENSE" in members
                assert "INTER-OFL.txt" in members
                assert any(p.startswith("ArSonVerb.vst3/") for p in members) if "-VST3-" in item.name else True
        else:
            raise AssertionError(item)
        digest = hashlib.sha256(item.read_bytes()).hexdigest()
        assert f"{digest}  {item.name}" in checksums, item
    print(f"PASS: {platform}, content, format, notices and checksums")


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="arsonverb-dist-test-") as folder:
        for platform in ("windows", "linux", "macos"):
            test(platform, Path(folder))
