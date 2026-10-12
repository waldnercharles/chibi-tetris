# Validation record

## Nine-minute visual polish — October 11, 2026

This pass inspected only 0:00–9:00 of the supplied recording. Nine contact
sheets retain 180 frames at three-second intervals, with a separate clear
detail sheet. [The study](tetris-effect-study.md#nine-minute-polish-pass)
records the observations and adaptations. The downloaded movie and loose
frames were removed; contact sheets, browser captures and logs remain in
ignored `.chibi/reference-polish`.

Hard drops now retain a separate 768-grain wake for 4.8–6 seconds. Accepted
actions brighten and bend the existing background, and gameplay tails freeze
when paused. A longer clear front, finer grid and luminous block borders make
the well easier to read. Original cyan streams and a rim give way to ivory,
amber and teal orbital geometry. New runs and losses share full-size outward
block flight. Automatic Journey chapters briefly hide the board during a
2.6-second camera passage, then restore the same pile and score. Quiet settings
skip the passage and retire departing blocks without movement or shrinking.

Independent review covered transition ownership, opaque block cores, fixed
pools, pause clocks, shader bounds and audio recovery. The selected dispatcher
group (`--loss`, `--attract`, `--consumer`, `--vfx`, `--music-field`, `--stage`,
`--ui`) passed 16 C cases and 821 dispatcher assertions in 45.569 seconds under
one production 60-second deadline. Fixtures also execute authored assertions,
including hidden/restored cell identities and input rejection during passage.
Audio interruption cases supply host-visible connection loss and recover
through the real `AudioReactive` fallback; they do not simulate browser stream
termination. New palette and particle ownership checks failed
against their prior implementations during development.

Production and changed fixture assets are formatted, C formatting and diff
checks pass, and validation reports 144 assets with zero errors or warnings.
Native and web exports use the unchanged pinned engine and SDK. A bounded
640 × 360 Metal preview compiled and rendered the orbital shader. The final
web build visibly rendered the revised wake, block borders, background shapes
and whole-block departure; movement, rotation, hold, both Duet drops and pause
were exercised. No product errors appeared in the browser console.

At normal 2560 × 1440 rendering (1280 × 720 CSS), two-second warmups followed
by 2.5-second samples measured Journey at 58.80 FPS, 16.7 ms median and 24.1 ms
p95, and Duet at 58.82 FPS, 16.7 ms median and 24.9 ms p95. The prior build's
same-size sample measured 58.78 FPS and 24.9 ms p95. These are short local
observations, not sustained or cross-device guarantees. Public revision,
assets and play are verified separately after committing and publishing.

The game-wide aggregate and full/quarantined engine, render and Studio suites
were not run or reconstructed. The engine pin did not change. No temporary
project clones or one-off executables remain from this pass.

## Persistent particle flow and reference-led reactions — October 11, 2026

The engine pinned in `toolchain.json` adds optional persistent state to GLSL particle fields. A GPU update writes position, velocity and age into ping-pong floating-point textures; the instanced mesh draw reads that state. The 131,072 main grains now move through changing force fields instead of prescribed surfaces. Another 16,384 grains provide quiet depth. Stable integer hashes give the main population distinct identities, and independently distributed reset ages prevent a synchronized fade. This is particle advection, not a pressure-based fluid solver or particle collision system.

The supplied Tetris Effect recording was inspected through a low-resolution overview and denser clear/loss frame sequences. [The study](tetris-effect-study.md) distinguishes measured observations from original tuning. Movement, rotation, rejection, soft drop, hard drop and landing now have different exposed-edge paths and timing. Contact and lighting begin immediately; the granular drop wash retires in 90 ms. Small filtered block impulses add lean, depth compression and highlights without moving logical cells. Final loss reuses the original colored blocks for an accelerating upward tumble, keeps Hold/Next fixed, delays the result, and retires geometry after results without advancing gameplay time.

Independent review found and fixed a retired-stack resurrection when returning from the audio chooser. Regression coverage includes that path, original block colors and transforms, frozen scoring, restart, Solo/Duet, rescue priority, winning-clear priority and quiet settings. Narrow-window play now uses a smaller authored camera width; the intro eases from the full chooser composition. Full-width Duet is retained, and the backdrop no longer exposes its edges in tall windows.

| Affected game check | Result |
| --- | --- |
| Loss, restart and audio return | Four C cases / 288 dispatcher assertions; final preview-retention run 13.182 seconds. |
| Action VFX | 1.774 seconds; restoring the former contact delay failed five authored assertions. |
| Action reactions | Five cases; 4.746 seconds. |
| Motion and camera/attract handoff | Five C cases / 43 dispatcher assertions; 5.088 seconds. Previous camera framing failed the new check. |
| Music response, spatial forces and consumer | Three C cases / 25 dispatcher assertions; 3.883 seconds. |

The GPU-state contract passed actual GLSL-to-WGSL/Metal compilation, 14 source/descriptor/prepared cases with 1,280 assertions, and a focused numeric GPU test with eleven hidden 64 × 64 frames. The numeric check covers state retention, reset, unchanged time, capped delta, age, cross-row indexing above 1,024 grains, and shader/count/seed changes. It completed in 1.043 seconds through Cocoa/Metal. The first offscreen-driver attempt could not initialize on this host and is not counted as passed. A separate 96-grain DriftingGrains fixture rendered without Tetris mechanics at 320 × 180 using two warmup frames and one saved frame; it completed in 1.055 seconds.

Browser work exposed two host issues: a zero-sized initial surface reached a null WebGPU canvas, and later CSS-only size changes did not notify SDL. The player now skips presentation while its surface is absent; both web shells observe canvas layout and notify SDL's existing resize handler. The graphics regression failed before the guard and passed afterward in 0.984 seconds. The actual-shell callback regression failed before its fix, then passed 14 cases / 64 assertions in 0.861 seconds. The final reviewed engine passed native checks (299 cases / 178,924 assertions, 11.167 seconds), language checks (178 / 10,518, 0.952 seconds), tooling checks (93 / 22,586, 24.555 seconds), formatting and diff checks.

Production assets and fixtures are formatted; validation reports 144 assets with zero errors or warnings. Clean pinned native and web exports completed. The final macOS application ran four hidden frames and three fixed simulation steps in 1.864 seconds under a 30-second deadline. A browser fixture started with zero CSS canvas area and automatically recovered to 1,591 × 2,350 pixels after layout restoration, with no manual resize notification; the temporary fixture was removed and the clean page reloaded.

Local play confirmed movement, rotation, hold, independent Duet drops, pause, restart and the visible upward whole-block departure. The restored camera framing and full-height backdrop were inspected in the narrow browser panel. At normal 1,589 × 2,350 pixels (662 × 979 CSS), Duet with the full 147,456-grain background measured 58.81 FPS, 16.7 ms median and 24.7 ms p95 after 120 warmup callbacks and a 2.5-second sample. No viewport override was applied. This is a short local observation, not sustained or cross-device performance. Publication and public play are verified separately after committing the clean source.

All test commands retain a 60-second production deadline. The previous game-wide aggregate remains incomplete and was not reconstructed. Full and quarantined native/render/Studio suites remain unexecuted. Direct and hemisphere light response is retained for particles; this change does not add particle shadows or Radiance Cascades. Completed external music capture, service authentication and human listening are not established by these visual checks.

Reference media, extracted frames and build logs remain in ignored `build/particula`; engine evidence remains in `.chibi/particle-state`. Disposable comparison projects and the unused loss-preview clone were removed.

## GLSL particles, music response and compact controls — October 11, 2026

The reviewed engine is `9fff00c8c371a5a09ac01fe455b7f8d23e5893d2`. The four production particle entrypoints now use ordinary GLSL with typed KDL descriptors and shared source includes. They compile directly through the renderer compiler; they do not pass through the graph's 1,024-node IR. Material graphs remain supported. The initial source-authoring contract covers particle shaders, not general surface or compute shaders, HLSL or WGSL input. The engine embeds source and includes in prepared artifacts and preserves the last valid runtime program if replacement compilation fails.

The background retains 147,456 GPU mesh grains. Four formations morph over ten seconds. Measured bass changes compression and depth waves, midrange drives curl, and treble changes color and grain size. Four bounded, world-space impulses carry player actions into the background. The live camera position keeps those impulses aligned with the board during the opening camera move. Clear fragments retain their quick fracture front while dim embers last 3.2–6 seconds according to tempo confidence. Cooling completes independently of that tail. No extra per-row particles were added.

The opening uses compact charcoal panels with neutral mesh text, audio status, mode choices, expandable options and an explicit Play button. The real autoplayer remains beside the controls. Starting clears its occupied rows and retains their particle tails while the camera centers and the HUD fades in.

| Check | Result |
| --- | --- |
| Project formatting and validation | 144 assets, zero errors or warnings. |
| Music response, spatial forces and clear effects | Three C cases / 26 dispatcher assertions, plus 41 music-response, 24 field and 211 VFX authored checks; 3.827 seconds under one deadline. |
| UI, audio onboarding and attract transitions | Affected selections passed together in 46.722 seconds. The separate final consumer check passed in 2.696 seconds. |
| Direct GLSL actual backend compilation | 13 cases / 187 assertions; 0.889 seconds. |
| Source authoring and controller operations | 16 cases / 1,318 assertions; 1.251 seconds. |
| Engine native checks | 294 cases / 178,865 assertions; 12.016 seconds. |
| Engine language checks | 178 cases / 10,510 assertions; 0.970 seconds. |
| Engine tooling checks | 93 cases / 22,586 assertions; 24.983 seconds. |

The prepared-program decoder previously reserved suspension storage as if every pause copied every value. It now charges actual retained vector capacities and owned descriptor storage against the same limit. The regression retains 1,725,425 bytes, succeeds under a 4 MiB budget, rejects a one-byte-short budget, and verifies the resumed result on both execution backends. The per-module decoder limits are unchanged. Separately, the complete game retains about 70 MiB of prepared resources and modules, exceeding the standalone player's former 64 MiB whole-project allowance. The player now selects a named 128 MiB canonical-storage budget; individual file, module and scratch limits remain unchanged. A CPU-only decode of the actual packaged artifact established the size before changing the host allowance: 73,532,564 retained bytes and a 74,445,232-byte peak loading reservation. The generic `test_tools --player-decode` case reproduces rejection at 64 MiB and reaches the pre-publication boundary at 128 MiB, including a one-byte-budget failure before retry. That case passed in 0.347 seconds. Packaged execution separately checks the player host wiring.

Independent review covered the final game shaders and music consumers, engine source ownership and replacement failure paths, prepared codecs and suspension accounting, controller authoring, and headless compiler dependencies. The separate GlslRainField fixture uses 128 lit grains and a GLSL helper loop without Tetris mechanics. It rendered through Metal in a hidden Cocoa window, with four CPU frames, two warmup frames and one saved 640 × 360 frame. The game preview used 60 CPU frames at the same render budget and showed 147,520 particles and 868 entities. Its saved frame cost 17.005 ms. Neither capture is a sustained frame-rate measurement.

Native and web exports use the clean pinned SDK and player. The packaged macOS application completed four hidden frames and three steps in 2.653 seconds. Local browser checks covered the opening controls, full-motion options, shared-clear play transition, movement, rotation, hold, hard drop, independent Duet boards, pause and restart. The background changed formation during play, and no block striping or current product console errors were observed. With 120 warmup frames followed by 2.5-second samples, Journey measured 58.80 FPS at 2047 × 2350 pixels (852.5 × 979.17 CSS, DPR 2.4); Duet measured 58.83 FPS at 3137 × 2350 pixels (1306.67 × 979.17 CSS, DPR 2.4). Both had a 16.7 ms median and 25.0 ms 95th percentile. The browser resized naturally between observations; no viewport override was applied. These short samples do not establish sustained or cross-device performance. Publication and public browser play are verified again after the clean source commit.

Diagnostic logs and images remain in ignored `build/music-response` and engine `.chibi/shader-source`, `.chibi/prepared-suspensions` and `.chibi/package-exports`. Disposable diagnostic project copies and the headless compiler scratch build were removed.

All test commands retain their production 60-second deadline. The earlier game aggregate remains incomplete and was not reconstructed. Full and quarantined native/render/Studio suites remain unexecuted. External service authentication, completed audio capture and human listening remain unverified. Particle motion uses GPU analytic forces, drag and gravity; it is not a stateful fluid solver or Radiance Cascades lighting.

## Attract mode, stable blocks and action trails — October 11, 2026

The opening runs a real autoplayer beside compact audio and mode controls. Explicit Play releases the occupied demo rows through the existing line-clear effect, moves the camera to the board and reveals the HUD. Demo results do not advance player progress. The title, mode heading and chapter heading are removed from gameplay and results. Blocks share a small bevel, retain song-reactive finishes and have lower ordinary emission, rougher highlights and a narrower bloom radius.

The reported dark checkerboard patches were reproduced in a fixed browser scene with the thinnest finish. A near-only A/B changed the camera near clip from 4 to 40 while both point and directional shadows stayed enabled. A sampled yellow face changed from 69 dark pixels out of 320 to zero; its median luminance remained approximately 145. The colored coat and opaque core were too close for the former camera depth precision. The authored motion regression guards a depth separation margin at the smallest preview scale and farthest viewing distance. It also checks the shared bevel, restrained idle emission and music-reactive opacity. No renderer or engine default changed.

Move, rotate, hold and soft-drop reactions now leave longer, dimmer trails. Rotation follows the captured piece center. Hard drop has a compact 160 ms descent, contact fragments on exposed lower edges and a settling layer; the following lock event no longer duplicates the payoff. Ordinary grains receive partial scene lighting. Clear-layer counts and trajectories are unchanged. The [VFX research foundation](vfx-design-principles.md) records primary sources, practical design principles and limits; it is not yet an installed skill.

| Check | Result |
| --- | --- |
| Attract and recovery | Four C cases / 35 dispatcher assertions plus 54 authored assertions; 7.97 seconds. |
| Audio onboarding | Three cases / 43 assertions; 13.02 seconds. |
| Motion and coating depth | One case / eight dispatcher assertions; 4.523 seconds. Eleven authored checks failed before the appearance and camera fixes. |
| VFX and consumer | Two cases; 4.645 seconds under one shared deadline after the final size/opacity tuning. |
| HUD and preview interactions | Six cases / 1,240 assertions; 56.675 seconds. |
| Project validation | 111 assets, zero errors or warnings. |
| Packaged macOS startup | Four hidden frames and three steps; 4.719 seconds under a 30-second deadline. |

Every test command retained the 60-second production deadline. Review caught a same-run demo reset that rewound event serials and stopped subsequent effects; its regression failed before the queue fix. Coverage also includes external source loss during the clear transition, recovery without progress, a single explicit fallback start, connected-source preservation and actual text alpha during the HUD reveal. Authored assets and C sources are formatted. The unchanged pinned engine remains `287ff20417e43ca142d2a43807ce538a8287c098`.

Native and web exports passed. The final browser candidate was exercised through attract play, the shared clear, centered play, rotation, hold, hard drop, independent Duet drops and pause. Frame captures show the drop packet arriving before contact grains and their softer tail. No checkerboard patches were observed, and console output contained only the existing font-loading information messages. At normal display size, 120 warmup frames followed by 2.5-second samples measured Journey at 58.80 FPS (16.7 ms median, 24.9 ms p95) and Duet at 58.80 FPS (16.7 ms median, 25.0 ms p95), with a 3134 × 2350 canvas, 1306 × 979 CSS and DPR 2.4. The earlier attract sample measured 58.80 FPS at 2560 × 1440 with a 16.7 ms median and 25.0 ms p95. These are short observations, not sustained-play guarantees. Publication and public browser play are verified again after the clean source commit.

Diagnostic logs and images remain in ignored `build/piece-flicker` and `build/attract`; disposable diagnostic projects and web exports were removed.

The game-wide aggregate remains incomplete from the previous revision and was not rerun or reconstructed. Full and quarantined native/render/Studio suites remain unexecuted. Completed external audio capture, service authentication, human listening and sustained cross-device performance are not established by these checks.

## Cosmic materials and lit particles — October 11, 2026

Opaque graphite cores now sit beneath the colored block coats. Music changes coat opacity, roughness, metallic response and emission on both active and settled blocks. Four GPU compositions—nebula, accretion disk, cellular bloom and stellar current—morph over ten seconds; particle scale, rotation, opacity and light response vary with the composition. Surrounding solid facets were removed. Clear effects emit 2,656 instances per row across four independently timed layers.

The reviewed engine is `287ff20417e43ca142d2a43807ce538a8287c098`. Particle functions may append Scale, Rotation and Lighting results to the existing three-output contract. Their normals account for nonuniform instance and entity transforms. Direct and hemisphere lights are evaluated per vertex; particles do not receive shadows, occlusion, collisions or fluid simulation. Motion uses authored analytic drag, gravity and curved flow. No project migration is needed for these optional outputs.

| Check | Result |
| --- | --- |
| Affected game consumers, motion, stage and VFX | Four C cases / 35 dispatcher assertions, plus authored reports including 56 motion, 46 stage and 141 VFX checks; 8.316 seconds under one 60-second deadline. |
| Project validation and formatting | 107 assets, zero errors/warnings; production and fixture sources formatted. |
| Engine `make test-native` | 280 cases / 178,388 assertions; 10.412 seconds. |
| Engine `make test-language-all` | 178 cases / 10,510 assertions; 0.934 seconds. |
| Engine `make test-tools` | 92 cases / 22,540 assertions; 21.750 seconds. |
| Packaged macOS startup | Four hidden frames, three simulation steps; 1.721 seconds under a 30-second deadline. |

The new particle preparation and actual WGSL/Metal compilation cases failed against the previous implementation. Restoring the former Board fails two music/material checks; restoring the previous stage director fails three facet/transition checks. Independent review covered shader classification, uniform lifetimes, particle-only light binding, inverse-transpose normals, opaque-core ordering and the cached block-finish updates.

A separate, independently reviewed `--particle-depth` GPU check passed four 64 × 64 pixel comparisons in 2.094 seconds. Both particle depths and both cache orders composite correctly through the existing shared draw queue. The suspected ordering issue was disproved; no extra particle pass was added. The harness used a hidden Cocoa window but does not report its graphics backend or device.

The game-wide `make test` did **not** complete: the dispatcher reached its 58-second internal deadline, including a retry without competing builds. The focused affected checks above are the available current evidence; the earlier 24-case result below belongs to the previous revision. The incomplete aggregate was not reconstructed through partitions. Full and quarantined engine/render/Studio suites remain unexecuted.

A separate generic `LitRainField` rendered 128 lit, stretched and rotated drops with no opaque scene geometry. The hidden Metal game preview used 60 simulation frames, two warmup frames and one saved 640 × 360 frame: 147,784 particles and an 11.696 ms saved frame. A second preview replayed the existing six-piece route against the production scene (only bypassing music choice), clearing two rows and rendering 153,352 particles. Its saved frame took 72.950 ms while packaging work was also running. These individual frame observations are not sustained native FPS. The [clear image](screenshots/cosmic-evolution.png) comes from that actual route.

The final local web export uses the clean pinned SDK and WebGPU player. At 2560 × 1440 (1280 × 720 CSS, DPR 2), 120 warmup callbacks followed by a 2.5-second sample measured 58.81 FPS, 16.7 ms median and 24.9 ms 95th percentile with all 147,456 background particles. Movement, rotation, hold, hard drop, independent Duet drops and pause were exercised. The scene evolved to another composition during play. Console output contained existing font information messages, with no observed product errors. These short observations do not establish prolonged or cross-device performance. External service authentication and completed music capture were not added or verified by this visual revision.

A disposable copy of the production scene emitted three synthetic four-row releases through the real reaction queue at two, four and six seconds. At 2560 × 1440, the eight-second browser sample measured 58.94 FPS, 16.7 ms median, 24.8 ms 95th percentile, 25.1 ms 99th percentile and 25.9 ms maximum. Each burst adds 10,624 instances and four lights to the full background. A separate visual observation confirmed the release layers were on screen; no product errors were observed. The scratch workload and export are removed after use; they are not shipped.

## Layered reactions and evolving scenes — October 11, 2026

The preceding revision replaces oversized action outlines with particles born on the four captured block faces, compact descending packets, delayed landing compression and short local lights. Births follow a logarithmic cumulative distribution; size and alpha have continuous attack, release and retirement envelopes. A saturated pool preserves live effects. Down taps advance one row, held Down repeats at ten cells per second, and faster automatic gravity remains authoritative.

Four seeded scene families change particle formation, block finish and lighting together. Scene transitions follow chapters and measured musical changes, with heuristic phrase and track-boundary detection rather than provider metadata. The opening flow has two actions: Enter Play or M Connect audio. Connection readiness no longer depends on non-silent samples. Reconnection preserves the previous play, pause or result screen.

Independent review caught and corrected an unbounded facet rotation at large seeds, an angular interpolation branch change, short-tap gravity stalling, and a one-frame lighting drift on pause. Regression cases cover each. The original VFX implementation fails twelve new physical/timing checks; restoring the former soft-drop rate or accumulated-clock handling fails the motion regressions. Fixture canonical formatting and project validation are separate from execution.

The game dispatcher passed all 24 cases and 1,201 assertions in 40.151 seconds under one shared 60-second deadline. Project validation reported 107 assets, zero errors and zero warnings; final fixture formatting made no changes. A native Metal preview used a hidden Cocoa window, 60 simulation frames, two warmup frames and one saved 640 × 360 frame. It showed 147,784 particles including action cues. Its 14.918 ms saved-frame timing is a capture observation, not native FPS.

The local browser export was exercised at 2560 × 1440 (1280 × 720 CSS, DPR 2). After 120 warmup frames, short 2.5-second samples measured Journey at 58.80 FPS, 16.7 ms median and 25.0 ms 95th percentile; Duet at 59.03 FPS, 16.7 ms median and 25.1 ms 95th percentile. Movement, rotation, hard drop and independent Duet drops were visible. The console contained only existing font-loading information. These observations are not sustained-play or cross-device guarantees.

M reached the real sharing request. The in-app browser did not expose a usable source picker to this test, so completed external capture, provider playback, acoustic latency and human listening remain unverified. Authored connection-state, quiet-ready, retry and fallback behavior passed the controller tests. Native and web exports passed. The rebuilt packaged macOS application completed four hidden frames and three simulation steps in 2.532 seconds under a 30-second deadline.

Engine `00db04340` adds generic shader exp/log/pow and typed 3D parameter writes. Scalar and vector curve cases round-trip authored graphs, evaluate expected values and compile real WGSL; the new curve cases failed before the additions. The typed parameter regression failed with the old setter and now passes across bytecode and SLJIT, including rejection of incorrect widths and non-finite values. Independent review found no remaining shader/parameter defects. Packaged startup also exposed temporary verifier scratch being charged against retained prepared storage. The fix keeps a separate bounded scratch preflight without raising limits. A constrained arithmetic-module regression fails against the original accounting and passes with exact retained charges and execution on bytecode and SLJIT. These changes require no project migration.

The engine's admitted aggregates passed in Release without sanitizers: native 278 cases and 178,322 assertions in 9.753 seconds; language 178 cases and 10,510 assertions in 0.933 seconds; standalone tooling 92 cases and 22,540 assertions in 20.935 seconds. Each had its own production 60-second deadline. The native group includes the seven CPU shader/WGSL cases; the standalone tooling configuration does not include the graph compiler. Full native/render/Studio suites remain quarantined and unexecuted.

This record covers the 3D Stillwater revision and the matching engine and packages in [toolchain.json](../toolchain.json). Publication verifies the live release separately from local exports.

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
