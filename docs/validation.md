# Validation record

This record covers the 3D Stillwater revision and the matching engine and packages in [toolchain.json](../toolchain.json). Publication verifies the live release separately from local exports.

## Combined engine migration

The combined engine's Release controller upgraded the game from project format 22 to 23 in 0.415 seconds. Formatting completed in 9.905 seconds. Independent review confirmed that the migration changed only the project version; all 3D assets and canonical component names were preserved.

The full game dispatcher passed 22 C/pico_unit cases and 1,265 assertions in 34.408 seconds. Project validation passed for 104 assets with zero errors or warnings in 5.599 seconds. Each command used the production 60-second supervisor, with dispatcher compilation separate. These initial checks used controller SHA-256 `b8ed1d3dd7e3da37a379b7725da43362a359663f83bf888a01f3f65cf950311a` from the engine integration based on `c159b6cd6`, before its final source commit and packaging.

### Clean package checks

The final pin is engine `c98f548a36da0b1e6c6852e1d1d0ff5da89f8089` on `main`, with clean source snapshot `7bf8571dc84cbf2684b199f68b65f3c3220fd7a0928819227ec1e7e3b296a19d`. The SDK build identity, manifest hash and five web-player file hashes are recorded in `toolchain.json`. The project uses format 23 and player-prepared format 12.

Against that engine, the full dispatcher passed all 22 C/pico_unit cases and 1,265 assertions in 30.194 seconds. Formatting and validation passed together in 10.845 seconds, with no formatting changes and 104 assets, zero errors and zero warnings. The controller SHA-256 was `c03042af3d418b219d282643ffc4f853df5cc9f5236a102ba4fc7d394e8577a5`. The C dispatcher passed clang-format 22, and diff checks were clean. Each test command retained the production 60-second deadline; compilation was separate.

Both native and web exports succeeded with the pinned packages. The packaged macOS application completed four hidden frames and three simulation steps at fixed dt 0.0166667 in 7.910 seconds under the same supervisor. This checks packaged startup, not sustained native performance. Public revision, asset hashes and browser play are verified after clean source publication and reported in the release handoff.

### Provisional browser checks before the pacing fix

The local WebGPU export passed game-score selection; Solo movement, rotation, hold, hard drop, pause and resume; and independent movement, rotation and hard drops on both Duet boards. The imported WebGPU device was active. Console inspection found no runtime errors; RmlUi emitted its existing informational messages through stderr. This verifies the score-selection flow, not acoustic output or external audio capture.

At the normal 1280 × 720 CSS viewport with device pixel ratio 2, the canvas was 2560 × 1440. Each mode used 120 warmup frames followed by a 2.5-second sample:

| Mode | Observed FPS | Mean frame interval | 95th percentile interval | Mean synchronous callback | 95th percentile callback |
| --- | --- | --- | --- | --- | --- |
| Solo | 58.8353 | 16.9966 ms | 18.4 ms | 16.7088 ms | 18.0 ms |
| Duet | 58.8306 | 16.9980 ms | 18.3 ms | 16.7270 ms | 18.0 ms |

These are provisional measurements from before the web-player pacing correction. The player still called `cf_set_target_framerate(60)` while the browser also scheduled frames. Synchronous callback elapsed time includes that pacing and CPU work; it excludes asynchronous GPU completion. Frame intervals measure browser presentation scheduling separately. Measurements after the correction follow below; public-release verification follows publication. These short samples do not establish sustained performance or a rate above the display's refresh cadence.

### Browser pacing correction

After removing the player's extra pacing on the web, the local browser export was measured again at 2560 × 1440 pixels in the same normal 1280 × 720 CSS viewport with device pixel ratio 2. Each sample followed 120 warmup frames and covered 2.5 seconds, with approximately 300 frame intervals:

| Mode | Observed FPS | Mean synchronous callback | 95th percentile callback |
| --- | --- | --- | --- |
| Solo | 120.048 | 3.0787 ms | 4.5 ms |
| Duet | 119.995 | 3.46445 ms | 5.0 ms |

WebGPU remained active. Independent hard drops on both Duet boards updated each score to 38. Console inspection found no runtime errors; the remaining messages were font information. The preceding interaction checks covered score selection, Solo movement, rotation, hold, hard drop, pause and resume, and independent Duet controls.

The frame-rate measurements reflect the observed browser animation cadence. Synchronous callback elapsed time covers the main-loop work and excludes asynchronous GPU completion. These bounded samples do not establish sustained performance or a rate that another display can present. The final clean package identities are recorded above; public-release verification follows publication.

## Retired channel preferences

The master-only volume controls now use the authored music and sound levels. Old saved channel values no longer override them or get rewritten when master volume changes. This prevents a previously muted channel from staying silent after its separate control was removed.

The new C/pico_unit `--audio-preferences` case first failed against the unchanged game with engine `e9d23b04e`: after selecting the game score and raising master volume, the snapshot reported master 0.5, music 0 and sound 0. The failure completed in 1.459 seconds. After removing the retired preference reads and writes, the case passed 26 assertions in 1.968 seconds. It also checks that fresh profiles do not save hidden channel settings. Independent review found that an inherited reaction-only environment flag could broaden an explicit selector. After making explicit selectors take precedence, the same case passed alone with that flag set in 2.027 seconds. All runs used the production 60-second supervisor; compilation was separate. Asset formatting with the same controller completed in 8.538 seconds, and the C dispatcher was formatted with clang-format 22. These are simulation and saved-state checks, not a listening test or validation of the new engine package.

## Music choice and fluid-motion revision

Filled Geist meshes replace the stroke lettering. Controls move into pause, results retain their scores and times, and translucent board and HUD materials fade toward their edges. The material tints the background rather than sampling a scene blur. Background particles retain their identities and brightness; smoothed music controls and integrated input impulses move their trajectories. Hard drops produce five curved strands centered on the occupied piece, followed by a spreading ripple.

Music selection now precedes play. YouTube, Spotify and TIDAL choices request their own web players. A separate audio-sharing action supplies measured samples; service launch is not account sign-in. Gameplay waits for a non-silent external signal or an explicit choice of the game score. Permission failures keep the chooser open. Reconnecting preserves the board, elapsed time and previous pause or result screen.

The game dispatcher passed 21 C/pico_unit cases and 1,239 assertions in 35.342 seconds under one 60-second production deadline. The startup gate failed against the prior published game before implementation. Cases cover pointer choices, retry and fallback, preserved runs, clean play/pause/result HUDs, active and frozen Sprint digits, captured drop widths and centers, complete particle-shape freezing and smooth resumption. A separate regression failed on the prior Solo and Duet pause previews, then passed when hold and next-piece meshes were hidden outside play and restored on resume. Project validation reports 104 assets, zero errors and zero warnings. Changed authored sources and fixtures were formatted with the controller.

The glyph generator verifies finite geometry, front-face winding, normals and tangents, encoded mesh round trips, and sampled fill coverage against the font contours. Generation passed in 0.452 seconds; its separate outline proof was visually inspected. The fonts and the build-time tessellator retain their licenses and provenance in [LICENSES.md](../LICENSES.md).

A bounded native preview simulated 60 CPU frames and rendered two warmup frames plus one saved 640 × 360 frame. It completed in 1.588 seconds using Metal in a hidden Cocoa window, with 842 entities and 150,144 particles including drop feedback. The saved frame took 8.723 ms; this is a single capture observation, not sustained native FPS. The [Journey image](screenshots/style-journey.png) shows the new font, backing and dense braided drop wake. Independent review found no remaining authored-state, pool ownership or shader-input issues.

Native and web exports passed. The packaged macOS application completed four hidden frames and three simulation steps in 1.927 seconds under a 30-second deadline. Local browser checks covered the music gate, provider launch, the real sharing request, explicit score selection, reconnecting to an existing run, movement, rotation, hold, both Duet drops, and clean pause/resume previews. The final 2560 × 1440 browser export measured 58.80 FPS over 2.5 seconds after warmup, with a 16.7 ms median and 17.6 ms 95th percentile. The console contained no product errors. These short samples do not establish sustained or cross-device performance. Completed external audio capture and human listening remain unverified; opening a provider does not establish a connection to its audio.

Engine `e9d23b04e` includes reusable host URL actions and world-space pointer input. Independent review covered request ownership, browser activation and cancellation, input recording/replay, scenario/controller parsing, native source bindings and package exports. A quick-input timing issue was fixed before packaging. Packaging then exposed a missing ownership entry; a C regression failed on the omission and passed after the fix. Native authoring and test builds passed in Release without sanitizers. Final admitted checks passed: all 39 native selectors in 11.232 seconds, including feature coverage; language 178 cases and 10,510 assertions in 0.996 seconds; tooling 92 cases and 22,540 assertions in 21.402 seconds. Each ran under its own production 60-second deadline. The focused native viewport GPU check passed in 0.692 seconds. These additions require no project format migration. Quarantined full render and Studio suites remain unrun.

## Cosmic particle revision

The underwater scenery is replaced by two instanced 3D mesh fields. The full setting starts with 131,072 vortex particles and 16,384 stars; action feedback uses 96 reusable fields and eight lights. The scene has 842 entities. Particle positions are computed on the GPU from authored shader-library functions. The CPU updates field controls, not individual particles.

The game dispatcher now passes 17 C/pico_unit cases and 909 assertions under one 60-second production deadline (21.046 seconds). New cases cover accepted and rejected inputs, original clear-row positions, simultaneous Duet records, restarts, overflow accounting, quiet visual controls, and immediate audio/settings HUD updates. The producer and consumer regressions each failed against their previous implementation. These checks exercise the real authored behaviors; they do not establish visual quality or frame rate.

The matching engine's admitted checks passed on the reviewed tree, committed as `f198458a0`:

| Check | Result |
| --- | --- |
| `make test-native` | 265 cases, 177,624 assertions; 9.932 seconds. |
| `make test-language-all` | 178 cases, 10,510 assertions; 0.957 seconds. |
| `make test-tools` | 91 cases, 22,508 assertions; 22.746 seconds. |

Each group had its own production 60-second deadline. Configuration and build time were separate. C sources passed clang-format 22, feature fixtures passed canonical formatting, and diff checks were clean. Full and quarantined GPU/Studio suites remain unexecuted.

The shared renderer was independently reviewed for resource lifetimes, prepared shader publication, library edits and renames, and SDK packaging. A separate `NativeRainField` fixture uses falling 3D octahedra and an opaque occluder without game mechanics. Its hidden Metal render verified per-instance geometry, authored parameter overrides and depth. Generated particle shaders also passed the production GLSL-to-WGSL and Metal compiler paths with the particle index both used and unused.

The final game preview simulated 60 CPU frames, then rendered two warmup frames and one saved 640 × 360 frame in a hidden Cocoa window. It showed 148,096 mesh particles including active drop cues. Project validation and this capture completed together in 4.134 seconds under a 30-second deadline; validation reported 105 assets and no errors or warnings. The saved output frame cost 8.122 ms; this is a capture observation, not sustained native FPS. The [current Journey image](screenshots/cosmic-journey.png) includes the new dark score backing.

The local WebGPU export rendered at 2560 × 1440 (1280 × 720 CSS pixels). After 120 warmup frames, a 2.5-second sample with 73,728 background particles measured 58.8 FPS, 16.7 ms median and 24.4 ms 95th percentile. Doubling to the shipped 147,456 particles measured 58.8 FPS, 16.7 ms median and 17.7 ms 95th percentile. These short samples do not establish a prolonged-play guarantee.

Later in the same browser session, animation callbacks settled at 30 FPS. Effects off, suspending audio, and pausing the game's main loop did not change that ceiling: an independent browser animation callback still measured 30.05 FPS with the game stopped. The game loop uses one callback per browser frame. A separate 75-frame measurement of game work averaged 4.999 ms, with a 4.5 ms median and 8.0 ms 95th percentile; this includes CPU work and submission, not asynchronous GPU completion. Diagnostic hooks were removed and the page reloaded before further play checks. The browser/display limit is recorded rather than attributed to the game or hidden behind the initial faster sample.

Browser play verified direct launch, movement, rotation, hold, hard drop, pause, and independent Duet drops (different board scores and placements). M reached the real audio request and displayed `CHOOSE AUDIO`; source selection could not be completed through the in-app browser. The request was cancelled by returning to the score. No external music capture or listening result is claimed. Product console output contained only the engine's existing font information messages; a temporary profiling-hook error was confined to the diagnostic probe and cleared by reload.

## Responsive camera follow-up

The first public cosmic build exposed cropped HUD text in a tall browser panel. Engine `1f699e26a` now expands an explicit camera's vertical field of view when a narrower viewport needs more height to preserve the authored width. Camera position, stored lens settings and clip planes remain unchanged; rendering and pointer picking share the fitted lens.

The new projection regression failed before the fix. All six admitted `--viewport-resize` cases passed in 1.198 seconds using the native Metal path. They cover portrait and wide viewports, restored aspect, default and custom field of view, projected corner containment, picking rays and a finite near-180-degree projection. Independent review found no remaining issues.

The game dispatcher also passed all 17 cases and 909 assertions in 23.121 seconds with the camera fix.

The final engine aggregates passed on the exact reviewed tree: native 265 cases / 177,624 assertions in 10.978 seconds; language 178 / 10,510 in 0.918 seconds; tools 91 / 22,508 in 22.839 seconds. Each retained its own 60-second production deadline. Formatting and diff checks passed; quarantined suites remain unexecuted.

Before this camera follow-up, the public cosmic build was verified against its deployment manifest and live asset hashes. At its normal 2748 × 2350 canvas (1145 × 979 CSS), it measured 58.8 FPS after warmup, with a 16.7 ms median and 17.7 ms 95th percentile. A later sample after movement, rotation, hold and drop measured 58.81 FPS, 16.7 ms median and 24.8 ms 95th percentile. These are short observed samples, not a sustained-play guarantee.

The repackaged local browser export showed complete Journey and Duet framing at a narrower 1836 × 2350 canvas (765 × 979 CSS). Independent drops produced scores of 30 and 28 with distinct placements; the pause panel remained visible and the console contained no product errors. A warmed 2.5-second Duet sample measured 59.00 FPS, 16.7 ms median and 24.1 ms 95th percentile. Final publication and public browser play are checked again after the clean source commit.

## Earlier runtime performance correction

The published `eb4f89e` game with engine `143ceb926` reproduced the reported slowdown: 2.37 FPS, a 425.2 ms median frame and a 433.8 ms 95th percentile in Chromium 154 with a 2560 × 1440 WebGPU canvas. Reducing the canvas to 640 × 360 only reached 2.48 FPS. A CPU profile attributed approximately 73% of sampled time to the script payload collector's address lookup.

The runtime now skips null and foreign roots, uses indexed allocation lookup for retained values, and avoids collecting completed calls whose payload arena is about to be released. Suspended calls and completed calls with outstanding requests still collect. Inline scalar component writes use the existing typed field-write API instead of copying the whole component. The scene, particle counts and visual settings are unchanged.

Each new performance regression failed before its corresponding fix. C/pico_unit cases cover lookup work, interior pointers, freed/reused ranges, values surviving suspension, completed-call cleanup, aliased component references and change notifications. The component-write workload dropped from 2,012 arena backing allocations to 11 on both bytecode and SLJIT.

The same native CPU-only scene profile (five warmup steps, twenty measured steps, 989 entities, SLJIT) improved from 28.697 ms to 5.751 ms per simulation step; the 95th percentile improved from 29.784 ms to 6.302 ms. This excludes rendering and is not native FPS. Allocation churn remains approximately 80,000 xalloc calls and 31.6 MB requested per step; these are allocation traffic, not retained memory.

The combined local browser build measured 55.3 FPS, 16.7 ms median and 25.2 ms 95th percentile over 2.5 seconds at the original 2560 × 1440 resolution after loading and ten warmup frames. Short browser samples include scheduling and rendering and do not establish prolonged-play or cross-device performance. Publication additionally requires matching live asset hashes and warmed frame-time measurements from the public URL; those final observations belong in the release handoff.

The clean `3afbef743` engine was repackaged into the SDK and web player pinned in `toolchain.json`. The refreshed macOS export completed four hidden frames and three fixed simulation steps in 1.642 seconds under a 30-second deadline. This checks packaged startup, not sustained native rendering performance.

## Earlier automated checks

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

## Earlier audio capture checks

The earlier revision used M to request external audio and restored the score after a failure. The current revision instead retains the music chooser until an explicit choice succeeds. Browser capture uses a user-triggered sharing chooser; native capture uses a recognized loopback device or the exact device selected with `CHIBI_AUDIO_INPUT`. No account credentials or music-service APIs are involved. [audio.md](audio.md#music-reaction-and-capture) explains routing and source status.

PCM analysis and host lifecycle checks passed. An earlier browser build reached the capture request after M. A completed external capture, acoustic output, latency, runtime mix balance and human listening review have not been verified. Audio-file measurements and provenance are recorded separately in [audio.md](audio.md).

## Earlier native component-name migration

The separate 2D revision `42f6736`, merged in [PR #1](https://github.com/waldnercharles/chibi-tetris/pull/1), migrated from project format 21 through 22 to 23 using engine `dd20172b7f093d41d8b4cc23ce68ca479828fca1`. Its assets and fixtures adopted native component names without the `GAME.` and `Game` prefixes, and the format-22 step removed obsolete HRC lab settings.

That candidate passed project validation for 38 assets with zero errors or warnings in 13.403 seconds, Release asset formatting in 6.509 seconds, eight C/pico_unit gameplay cases with 95 assertions in 7.741 seconds, and installed-SDK compilation to a 28,300,107-byte prepared artifact in 2.016 seconds. Each check used the production 60-second supervisor; compilation of the test dispatcher was separate. The initial sanitizer-build formatter was stopped before its deadline, then the Release formatter completed. Native and browser exports were not rebuilt or played for that candidate.

Those results apply to the earlier 2D revision. Validation of the combined 3D game and current engine is recorded separately when completed.

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
