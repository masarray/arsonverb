# Validation record — 2026-10-24

**Environment:** Linux x86-64 container, GCC 14.2, CMake 3.31.6; Release and Debug/ASan/UBSan builds. Windows Visual Studio/VST3 host testing has **not** yet been performed.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DARVERB_BUILD_SHARED=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
./build/arverb_benchmark

cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-asan -j4
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-asan --output-on-failure
```

## Result

- Release unit suite: **PASS**.
- Debug ASan/UBSan unit suite: **PASS**.
- Unit checks: stereo feedback, processing stability, supported sample rates, in-place output, block segmentation, preset/parameter bounds, reset, C API, and zero observed `operator new`/`new[]` calls during DSP processing.
- Offline benchmark (Release, stereo f32, 48 kHz, 256-frame blocks, 30 seconds of synthesized input): **0.25–0.28 seconds** of processing in repeated runs in this container, approximately **108–120x real-time**. This is a single-machine microbenchmark, not a DAW CPU utilization measurement or universal performance number.

## Not yet validated

- Comparative sound quality against Valhalla or competing studio reverbs.
- Long-duration production audio + real vocals, plugin host automation, Windows x64 DLL ABI, multi-DAW VST3, true plate/chamber/hall modes, GUI or packaging.
- Detailed band-limited RT60, echo density, mono-compatibility listening tests, and perceived metallicity under all settings.
