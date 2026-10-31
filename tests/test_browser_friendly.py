"""Fast offline regression for a browser-friendly, ordinary installer ZIP."""
import hashlib
from pathlib import Path
import tempfile
import zipfile

from scripts.package_browser_friendly import build_package, SETUP_NAME, ZIP_NAME


def run() -> None:
    project = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix="arsonverb-browser-package-") as tmp:
        folder = Path(tmp)
        source = folder / SETUP_NAME
        # Fixture only: arbitrary bytes with PE prefix, never a published binary.
        source.write_bytes(b"MZ" + b"A" * (1024 * 1024))
        checksum = hashlib.sha256(source.read_bytes()).hexdigest()
        output = folder / ZIP_NAME
        actual_zip_sha = build_package(source, output, checksum, project)
        assert hashlib.sha256(output.read_bytes()).hexdigest() == actual_zip_sha
        with zipfile.ZipFile(output) as archive:
            assert archive.testzip() is None
            assert sorted(archive.namelist()) == sorted([
                SETUP_NAME, "LICENSE.txt", "INTER-OFL.txt", "RELEASE_NOTES.md",
                "INSTALL-FIRST.txt", "SHA256-INSTALLER-EXE.txt"
            ])
            assert archive.read(SETUP_NAME) == source.read_bytes()
            assert archive.read("SHA256-INSTALLER-EXE.txt").decode().strip() == (
                f"{checksum}  {SETUP_NAME}"
            )
            assert "AGPL" in archive.read("LICENSE.txt").decode()
            assert "Extract this ZIP" in archive.read("INSTALL-FIRST.txt").decode()
        try:
            build_package(source, folder / "duplicate.zip", "0" * 64, project)
        except ValueError as exc:
            assert "digest mismatch" in str(exc)
        else:
            raise AssertionError("Checksum mismatch was accepted")
        try:
            build_package(source, output, checksum, project)
        except FileExistsError:
            pass
        else:
            raise AssertionError("Existing ZIP was overwritten")
    print("PASS: ordinary ZIP, six files, integrity, provenance, no overwrite")


if __name__ == "__main__":
    run()
