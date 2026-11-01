# ArSonVerb — official landing page

## Homepage downloads and owner-provided platform icons

The hero has exactly three **direct VST3-only** downloads: Windows x64 ZIP, Linux x86_64 tar.gz and macOS universal ZIP. User-supplied Windows/Linux/macOS logo images are cropped into small 80×80 transparent PNG silhouettes in `website/images/*-download-icon.png` and tinted through CSS masks, so all three download buttons remain consistent with the satin bronze/gold visual design.

The download links target the official v1.0.1 GitHub Release; the section below retains installer and standalone choices. CSS download controls use **stationary hover geometry** (no `translateY` transforms), avoiding cursor-boundary hover flicker. On mobile, the three primary buttons stack neatly. Browser-side platform selection remains keyboard accessible.

Website QA `scripts/test_website.py` verifies the three exact hero URLs and ordering, PNG magic and alpha, CSS icon bindings and absence of moving download hitboxes.

One static site, two hosting providers, **no Node, bundler, framework, paid service, API token or backend**.

## Canonical source

`website/index.html`, `website/styles.css`, `website/app.js`, `website/favicon.svg`, `website/images/arsonverb-v1.0.1.png`.

The public screenshot is captured at **1180×545 from the real JUCE v1.0.1 editor** using `arverb_plugin_host_tests` with `ARVERB_GUI_SCREENSHOT_PNG`. This is not a generated UI concept or an old mockup. The image is checked into the repository once from the pinned actual native renderer, so **both providers serve the same screenshot** without rebuilding C++ every website deployment.

The website is independent of the DSP/plug-in build. Every actual download points directly to the immutable [v1.0.1 GitHub Release](https://github.com/masarray/arsonverb/releases/tag/v1.0.1), including preferred VST3 ZIP, installer ZIP and Standalone ZIP.

## GitHub Pages (GitHub Actions source)

User has enabled **Settings → Pages → Build and deployment → GitHub Actions**.

`.github/workflows/deploy-pages.yml` triggers on changes to `website/**` or its website QA script on `main`. It runs `python3 scripts/test_website.py`, uploads `website/` as-is via `actions/upload-pages-artifact@v4`, then deploys with `actions/deploy-pages@v4`. No CMake and no third-party secrets.

Expected GitHub project Pages address (verify once deployed): **https://masarray.github.io/arsonverb/**. This is a project site, so all local resource URLs start with `./`; no hardcoded root `/assets` paths to break under `/arsonverb/`.

## Cloudflare Pages (Git integration)

Use **Workers & Pages → Create → Pages → Connect to Git** and connect `masarray/arsonverb`.

| Cloudflare setting | Value |
|---|---|
| Framework preset | None |
| Repository | `masarray/arsonverb` |
| Production branch | `main` |
| Root directory | `/` (repository root) |
| Build command | `exit 0` (no compilation) |
| Build output directory | `website` |
| Environment variables | None |
| Output routing | Static HTML, no functions |

The Cloudflare Pages project gives its own `*.pages.dev` address after connection. No Cloudflare API token, Wrangler config, or other Cloudflare-specific fork is required. GitHub Pages and Cloudflare Pages both serve the same `website/` directory. Choose your desired custom domain in the hosting provider dashboard; don't commit a `CNAME` file that would disrupt the independent provider.

## Screenshot update for future plug-in versions

Use the manual screenshot workflow or source branch workflow `.github/workflows/capture-website-screenshot.yml` to rebuild the JUCE host on Linux + Xvfb, capture the real GUI, and commit the resulting PNG **once**. Do not hand-edit the screenshot or substitute a fake UI. Then update download asset paths/version and the `scripts/test_website.py` contract. Normal website changes trigger only the cheap static GitHub Pages workflow.

## Security and quality

- Strictly local/static site with no tracking scripts or storage.
- Keyboard accessible OS tabs, semantic landmarks and visible focus.
- Respects reduced-motion preferences.
- Downloads always go to GitHub Releases directly; the hosting provider does not proxy EXE, ZIP or other binaries.
- All 7 published platform assets remain available. Site does not modify security metadata or executable files.
- No claims of universal zero audio round-trip latency.
- Test: `python3 scripts/test_website.py`; requires real captured PNG checked in.
