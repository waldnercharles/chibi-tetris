# Validation record

This record covers the 3D Stillwater revision and the matching engine and packages in [toolchain.json](../toolchain.json). Native and browser exports are local build products. The published website is a separate release.

## Automated checks

Tests use the engine's production supervisor with one 60-second deadline per admitted command. Configuration and compilation are separate.

| Check | Result |
| --- | --- |
| Game `make test` | 10 C/pico_unit cases, 428 assertions, zero failures; 25.294 seconds. |
| Game `make validate` | 80 assets, zero errors, zero warnings. |
| Engine `make test-native` | 253 cases, 176,375 assertions; 9.692 seconds. |
| Engine `make test-language-all` | 173 cases, 5,839 assertions; 0.965 seconds. |
| Engine `make test-tools` | 91 cases, 22,508 assertions; 19.842 seconds. |

The game dispatcher calls the real authored rules, boards, shell and HUD. It covers bags, SRS, scoring, hold/drop/lock limits, both input slots, top-out/rescue, all modes, completion, chapter selection, saved settings, direct launch, successful-action telemetry, finite visual pools, effect controls, Sprint mesh digits and pause/result depth ordering. [tests/README.md](../tests/README.md) describes the fixtures and their limits. Fresh-runtime save tests inject an exported save snapshot; engine storage tests separately cover the filesystem transport.

The audio checks exercise known PCM samples, frequency separation, onset/release, silence, stale input, stereo cancellation, source switching and callback/device ownership. They do not substitute for listening or real external routing.

Independent reviews covered authored gameplay/presentation, audio lifetimes, native reflection identity, multiplayer input, saves and prepared-artifact compatibility. Asset formatting and diff checks passed.

## Rendering and exports

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
