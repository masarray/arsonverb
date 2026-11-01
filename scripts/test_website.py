#!/usr/bin/env python3
"""Cheap, offline site contract: builds identically on GitHub and Cloudflare Pages."""
from __future__ import annotations

from html.parser import HTMLParser
from pathlib import Path
import struct
import sys
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parent.parent
SITE = ROOT / "website"
ASSETS = {
    "ArSonVerb-Windows-x64-VST3-v1.0.1.zip",
    "ArSonVerb-Windows-x64-Installer-v1.0.1.zip",
    "ArSonVerb-Windows-x64-Standalone-v1.0.1.zip",
    "ArSonVerb-Linux-x86_64-VST3-v1.0.1.tar.gz",
    "ArSonVerb-Linux-x86_64-Standalone-v1.0.1.tar.gz",
    "ArSonVerb-macOS-universal-VST3-v1.0.1.zip",
    "ArSonVerb-macOS-universal-Standalone-v1.0.1.zip",
}


class Inspector(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.local = []
        self.release_assets = set()
        self.tabs = []
        self.panels = []
        self.images = []

    def handle_starttag(self, tag, attributes) -> None:
        attrs = dict(attributes)
        for attr in ("href", "src"):
            value = attrs.get(attr, "")
            if value.startswith(("http:", "https:")):
                parsed = urlparse(value)
                if parsed.netloc == "github.com" and "/releases/download/v1.0.1/" in parsed.path:
                    self.release_assets.add(parsed.path.rsplit("/", 1)[-1])
            elif value and not value.startswith(("#", "mailto:", "javascript:")):
                self.local.append(value.split("#")[0].split("?")[0])
        if tag == "button" and attrs.get("data-platform"):
            self.tabs.append(attrs["data-platform"])
            assert attrs.get("role") == "tab"
        if attrs.get("data-panel"):
            self.panels.append(attrs["data-panel"])
            assert attrs.get("role") == "tabpanel"
        if tag == "img":
            self.images.append(attrs)


def main() -> None:
    doc = SITE / "index.html"
    assert doc.is_file(), "site needs website/index.html"
    parser = Inspector()
    parser.feed(doc.read_text(encoding="utf-8"))
    assert parser.tabs == ["windows", "macos", "linux"], parser.tabs
    assert parser.panels == ["windows", "macos", "linux"], parser.panels
    assert ASSETS <= parser.release_assets, ASSETS - parser.release_assets
    for filename in parser.local:
        assert not filename.startswith("//"), filename
        path = SITE / filename.removeprefix("./")
        assert path.is_file(), f"Missing relative site resource: {filename}"
    assert (SITE / "app.js").stat().st_size < 8 * 1024
    assert (SITE / "styles.css").stat().st_size < 35 * 1024
    assert len(parser.images) == 1
    assert "Actual ArSonVerb" in parser.images[0].get("alt", "")
    screenshot = SITE / "images" / "arsonverb-v1.0.1.png"
    with screenshot.open("rb") as image:
        assert image.read(8) == b"\x89PNG\r\n\x1a\n"
        header = image.read(16)
        width, height = struct.unpack(">II", header[8:16])
        assert (width, height) == (1180, 545), (width, height)
    assert screenshot.stat().st_size >= 15 * 1024, "Screenshot may be empty"
    html = doc.read_text(encoding="utf-8")
    assert "Real JUCE GUI" in html.upper().title() or "REAL JUCE GUI" in html
    assert not any(s in html for s in ("TODO", "example.com", "YOUR_TOKEN", "PLACEHOLDER"))
    print(f"PASS: responsive static website, 7 real download assets, "
          f"OS tabs, same-origin resources, real screenshot {width}x{height}")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"SITE QA FAILED: {error}", file=sys.stderr)
        raise
