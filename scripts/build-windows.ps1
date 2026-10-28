# Build and test AR Studio Reverb VST3 on Windows x64 with Visual Studio 2022.
# Usage: pwsh -File scripts/build-windows.ps1
# Or: pwsh -File scripts/build-windows.ps1 -JuceSource C:\dev\JUCE
# The script deliberately does not install or sign the VST3.
param(
    [string]$JuceSource = "",
    [string]$BuildDir = "build-vst3",
    [ValidateSet("Release", "Debug")][string]$Configuration = "Release"
)
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildPath = Join-Path $Root $BuildDir
$Configure = @(
    "-S", $Root, "-B", $BuildPath,
    "-G", "Visual Studio 17 2022", "-A", "x64",
    "-DARVERB_BUILD_PLUGIN=ON",
    "-DARVERB_BUILD_TESTS=ON",
    "-DARVERB_BUILD_SHARED=ON"
)
if ($JuceSource) { $Configure += "-DARVERB_JUCE_SOURCE_DIR=$JuceSource" }
Write-Host "[1/3] Configure Windows plugin and core"
& cmake @Configure
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
Write-Host "[2/3] Build plugin, C API and all test targets"
& cmake --build $BuildPath --config $Configuration --parallel 2
if ($LASTEXITCODE -ne 0) { throw "MSVC build failed ($LASTEXITCODE)" }
Write-Host "[3/3] Run CTest incl. JUCE AudioProcessor host contract"
& ctest --test-dir $BuildPath -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "CTest failed ($LASTEXITCODE)" }
Write-Host "Built artifacts (unsigned, DAW validation still required):"
Get-ChildItem (Join-Path $BuildPath "ARStudioReverb_artefacts") -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -like "*.vst3" -or $_.Name -like "*.exe" } |
    ForEach-Object { Write-Host $_.FullName }
