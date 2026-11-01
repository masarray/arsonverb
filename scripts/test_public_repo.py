#!/usr/bin/env python3
"""Fast offline user-facing repository contract. Python stdlib only."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent.parent
REQUIRED = [
    "README.md", "CHANGELOG.md", "CONTRIBUTING.md",
    "SECURITY.md", "SUPPORT.md", "CODE_OF_CONDUCT.md",
    "LICENSE", "LICENSE-STATUS.md", "docs/QUALITY_STATUS.md",
    "docs/RELEASE_V1.0.1.md", "docs/WEBSITE.md",
    "docs/ARSONMIX_INTEGRATION.md",
    ".github/ISSUE_TEMPLATE/config.yml",
    ".github/ISSUE_TEMPLATE/bug_report.yml",
    ".github/ISSUE_TEMPLATE/feature_request.yml",
    ".github/ISSUE_TEMPLATE/audio_validation.yml",
    "website/index.html", "website/images/arsonverb-v1.0.1.png"
]

def validate_links(path: Path) -> int:
    text = path.read_text(encoding="utf-8")
    checked = 0
    for target in re.findall(r"!?(?:\[[^\]]*\])\(([^)]+)\)", text):
        url = target.strip().split("#", 1)[0].split("?", 1)[0]
        if not url or url.startswith(("https://", "http://", "mailto:", "#")):
            continue
        assert not url.startswith("/"), f"Unsafe root-absolute link in {path}: {url}"
        candidate = (path.parent / url).resolve()
        assert candidate.is_relative_to(ROOT.resolve()), f"Path escapes repo: {path}: {url}"
        assert candidate.is_file(), f"Broken local link in {path}: {url}"
        checked += 1
    return checked

def main() -> None:
    for required in REQUIRED:
        path = ROOT / required
        assert path.is_file() and path.stat().st_size > 0, required
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    assert len(readme) < 11000, "Top-level README should be product-facing and concise"
    for phrase in (
        "ArSonVerb", "v1.0.1", "Windows", "macOS", "Linux",
        "AGPL-3.0", "Studio", "Download", "QUALITY_STATUS.md",
        "CONTRIBUTING.md", "SECURITY.md", "SUPPORT.md",
        "CODE_OF_CONDUCT.md", "CHANGELOG.md",
        "website/images/arsonverb-v1.0.1.png",
    ):
        assert phrase.lower() in readme.lower(), f"Missing product/quality detail: {phrase}"
    assert "no distribution license" not in readme.lower()
    assert "current p8 v0.7.0" not in readme.lower()
    assert "public stable v1.0.0" not in readme.lower()
    for name in ("bug_report", "feature_request", "audio_validation"):
        content = (ROOT / f".github/ISSUE_TEMPLATE/{name}.yml").read_text()
        assert "description:" in content and "body:" in content
        assert "validations:" in content and "required: true" in content
    quality = (ROOT / "docs/QUALITY_STATUS.md").read_text()
    assert "Still requiring human/real-host validation" in quality
    assert "12/12" in quality
    assert "ArSonMix" in quality
    notes = (ROOT / "LICENSE-STATUS.md").read_text()
    assert "AGPL-3.0-only" in notes and "v1.0.1" in notes
    total = 0
    for path in ("README.md", "CHANGELOG.md", "CONTRIBUTING.md", "SUPPORT.md",
                 "SECURITY.md", "CODE_OF_CONDUCT.md", "LICENSE-STATUS.md",
                 "docs/QUALITY_STATUS.md", "docs/ARSONMIX_INTEGRATION.md"):
        total += validate_links(ROOT / path)
    assert total >= 20, f"Unexpectedly few local Markdown references: {total}"
    print(f"PASS: 18 public files, concise README, four docs classes, three issue forms, {total} valid local links")

if __name__ == "__main__":
    main()
