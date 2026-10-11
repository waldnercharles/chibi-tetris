# Validation record

This record covers the 3D Stillwater revision and the matching engine and packages in [toolchain.json](../toolchain.json). Publication verifies the live release separately from local exports.

## Runtime performance correction

The published `eb4f89e` game with engine `143ceb926` reproduced the reported slowdown: 2.37 FPS, a 425.2 ms median frame and a 433.8 ms 95th percentile in Chromium 154 with a 2560 × 1440 WebGPU canvas. Reducing the canvas to 640 × 360 only reached 2.48 FPS. A CPU profile attributed approximately 73% of sampled time to the script payload collector's address lookup.

The runtime now skips null and foreign roots, uses indexed allocation lookup for retained values, and avoids collecting completed calls whose payload arena is about to be released. Suspended calls and completed calls with outstanding requests still collect. Inline scalar component writes use the existing typed field-write API instead of copying the whole component. The scene, particle counts and visual settings are unchanged.

Each new performance regression failed before its corresponding fix. C/pico_unit cases cover lookup work, interior pointers, freed/reused ranges, values surviving suspension, completed-call cleanup, aliased component references and change notifications. The component-write workload dropped from 2,012 arena backing allocations to 11 on both bytecode and SLJIT.

The same native CPU-only scene profile (five warmup steps, twenty measured steps, 989 entities, SLJIT) improved from 28.697 ms to 5.751 ms per simulation step; the 95th percentile improved from 29.784 ms to 6.302 ms. This excludes rendering and is not native FPS. Allocation churn remains approximately 80,000 xalloc calls and 31.6 MB requested per step; these are allocation traffic, not retained memory.

The combined local browser build measured 55.3 FPS, 16.7 ms median and 25.2 ms 95th percentile over 2.5 seconds at the original 2560 × 1440 resolution after loading and ten warmup frames. Short browser samples include scheduling and rendering and do not establish prolonged-play or cross-device performance. Publication additionally requires matching live asset hashes and warmed frame-time measurements from the public URL; those final observations belong in the release handoff.

The clean `3afbef743` engine was repackaged into the SDK and web player pinned in `toolchain.json`. The refreshed macOS export completed four hidden frames and three fixed simulation steps in 1.642 seconds under a 30-second deadline. This checks packaged startup, not sustained native rendering performance.

## Automated checks

Tests use the engine's production supervisor with one 60-second deadline per admitted command. Configuration and compilation are separate.

| Check | Result |
| --- | --- |
| Game `make test` | 10 C/pico_unit cases, 428 assertions, zero failures; 9.317 seconds. |
| Game `make validate` | 80 assets, zero errors, zero warnings. |
| Engine `make test-native` | 253 cases, 176,375 assertions; 10.094 seconds. |
| Engine `make test-language-all` | 178 cases, 10,510 assertions; 0.967 seconds. |
| Engine `make test-tools` | 91 cases, 22,508 assertions; 21.150 seconds. |

The game dispatcher calls the real authored rules, boards, shell and HUD. It covers bags, SRS, scoring, hold/drop/lock limits, both input slots, top-out/rescue, all modes, completion, chapter selection, saved settings, direct launch, successful-action telemetry, finite visual pools, effect controls, Sprint mesh digits and pause/result depth ordering. [tests/README.md](../tests/README.md) describes the fixtures and their limits. Fresh-runtime save tests inject an exported save snapshot; engine storage tests separately cover the filesystem transport.

The audio checks exercise known PCM samples, frequency separation, onset/release, silence, stale input, stereo cancellation, source switching and callback/device ownership. They do not substitute for listening or real external routing.

Independent reviews covered authored gameplay/presentation, audio lifetimes, native reflection identity, multiplayer input, saves and prepared-artifact compatibility. The performance follow-up separately reviewed collector indexing, completed and suspended activation lifetimes, typed component writes and their interactions. Native and language checks used an unsanitized Release build; the standalone tooling dispatcher used its Makefile's Debug configuration. Formatting and diff checks passed. No project format migration is required for these internal runtime changes.

## Earlier presentation and export checks

The following captures and interaction checks cover the original presentation revision. The performance correction above changes the shared runtime; publication checks the newly packaged exports separately.

Bounded controller captures used Metal with a hidden Cocoa window at 640 × 360. Each simulated 60 CPU frames and rendered two warmup frames plus one saved frame. Journey and Duet completed together in 10.811 seconds under one 60-second deadline. A final Journey grid-legibility capture completed in 5.495 seconds under a 30-second deadline. Captures contained 989 entities and approximately 900 native particles; these are observations, not a frame-rate benchmark.

The [Journey](screenshots/journey.png) and [Duet](screenshots/duet.png) images show the 3D renderer. The scene contains geometry, generated mesh text, procedural shader materials, lights, shadows, bloom, ambient occlusion, particles and trails. It contains no RML game overlay or background images.

`make native` and `make web` produced the macOS application and browser export. A browser check exposed a WebGPU compiler defect in the shared noise helper: its generated loop comparison lost parentheses around a shift. The corrected helper passed a regression through the actual graph-to-WGSL compiler, and the browser rendered the scene after repackaging. The final player observations below use the corrected export. The packaged native application exited successfully after four hidden 1280 × 720 frames and three simulation steps at a fixed 1/60-second step. This is a packaged-startup check, not a native interactive play session.

The final native-startup and browser checks completed together in 18.949 seconds under one 45-second deadline. At 1280 × 720, browser play opened directly, move/hold/drop worked, both Duet boards dropped independently, and the pause panel correctly covered the HUD behind it. Startup and play captures confirmed that streams no longer emit an unpositioned particle cloud inside the well. No shader or JavaScript page errors were observed; the console contained only the engine's font-loading information messages. The browser reported an Apple `metal-3` WebGPU adapter.

The preceding browser interaction check exercised Sprint's elapsed clock and local save/reload. Lowering volume wrote 0.7; reloading and lowering it again wrote 0.6, demonstrating restored preferences. That check completed in 22.543 seconds under its 45-second deadline. Its only subsequent game change was dormant stream initialization, which the final player check covered. The linked screenshots are from the final browser export. These checks do not establish sustained frame rate or platform-wide compatibility.

## Audio capture

M requests external audio. Browser capture uses a user-triggered sharing chooser; native capture uses a recognized loopback device or the exact device selected with `CHIBI_AUDIO_INPUT`. Failure restores the bundled score. No account credentials or music-service APIs are involved. [audio.md](audio.md#music-reaction-and-capture) explains routing and source status.

PCM analysis and host lifecycle checks passed. An earlier browser build reached the capture request after M. A completed external capture, acoustic output, latency, runtime mix balance and human listening review have not been verified. Audio-file measurements and provenance are recorded separately in [audio.md](audio.md).

## Reproduce

Use the engine revision and SDK identified in [toolchain.json](../toolchain.json):

```sh
make authoring ENGINE=/path/to/pinned/chibi
make test-build ENGINE=/path/to/pinned/chibi
make test ENGINE=/path/to/pinned/chibi
make format ENGINE=/path/to/pinned/chibi
make validate ENGINE=/path/to/pinned/chibi
make native
make web
make serve
```

The engine's admitted aggregate commands are `make test-native`, `make test-language-all` and `make test-tools`. Use that checkout's documented build configuration and production deadlines.

Full and quarantined native/render/Studio suites were not run and are not passed. Physical controllers, prolonged play and broad browser/device compatibility remain untested. Task scratch project copies and generated probe binaries are removed after verification; ignored build logs and diagnostic images remain under `build/resonance`.
