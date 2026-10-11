# Validation record

This record covers deterministic simulation, observed player behavior, and remaining coverage limits. It applies to the Stillwater implementation and SDK recorded by [toolchain.json](../toolchain.json). Native and browser exports were built locally from the authored project. See the [README](../README.md) for current browser access and publishing steps.

## Automated checks

All test commands used the engine's production deadline supervisor. Configuration and compilation were timed separately. The normal test ceiling is 60 seconds; the observed native play session used a stricter 55-second ceiling.

| Check | Completed evidence |
| --- | --- |
| Game `make test` | Eight C/pico_unit cases passed, zero failed, 95 C assertions; 11.265 seconds. |
| Game `make validate` | `validate_project` returned `valid: true`: 38 assets, zero errors, zero warnings. |
| Engine `make test-native` | The admitted native selector group completed successfully in 9.290 seconds. This includes source/native integration and the feature-coverage scenes. |
| Engine `make test-language-all` | The admitted language selector group completed successfully in 0.998 seconds. |
| Engine `make test-tools` | 90 cases passed, zero failed, 22,506 assertions; 20.996 seconds. |

These timings are measured results from this run, not performance requirements. The engine groups include the renderer repair. The focused GPU selector was rebuilt and rerun after the final regression-case adjustment.

The game dispatcher is [tests/main.c](../tests/main.c). It clones the project into a temporary fixture, invokes the real controller, and checks authored runtime results. Its eight cases cover:

1. Piece geometry, deterministic bags, SRS kicks, collision, ghost placement, compaction and scoring through the actual rule libraries.
2. Two keyboard-driven boards, movement repeat, hold/drop behavior, pause/restart and the fixed entity pool.
3. The same board behavior using two independent synthetic controller slots.
4. Lock delay and the fifteen-reset limit.
5. Top-out and rescue.
6. Journey progression, chapter selection, settings commits and fresh-runtime save readback.
7. Sprint completion, Duet progression/reserve/failure and controller disconnect pause.
8. Journey, Sprint and Duet completion when the final clear would otherwise be followed by a blocked spawn.

Authored fixtures also count their own checks; the 95 figure counts the outer C assertions. Fresh-runtime save tests pass exported `saved_state` into an isolated runtime. They do not open the player's disk file or browser store. The SDK tests separately cover numeric round trips, malformed/nonfinite values, project isolation, file reopening and explicit save-path overrides. See [tests/README.md](../tests/README.md) for fixture ownership and deadline behavior.

## Observed player behavior

The native export was operated through its real window with keyboard input. Play, hard drop, hold, pause and settings responded. Master volume changed from 80 to 70, and the isolated disk save selected with `--save` contained the updated setting. The bounded session completed 2,100 displayed frames and 2,139 simulation steps in 41.805 seconds under its 55-second supervisor deadline. A second launch with the same save file displayed Master 70 in settings. It exited successfully after 2,400 displayed frames and 2,421 simulation steps in 42.368 seconds under the same 55-second deadline.

The browser export was operated in the in-app browser. Keyboard solo play and both Duet boards responded, including dropping and holding pieces. Pause and settings worked. Master 70 persisted after reloading the page. The game was also observed at a 960 × 540 viewport. No browser errors were observed during those interactions. This is a local browser check, not a browser/device compatibility matrix.

No physical controllers were available. Synthetic two-slot input verifies action routing, independent boards and disconnect behavior; it does not verify host hardware mapping, hotplug or a particular controller model.

## Audio and rendering

The original music and effects have source provenance, successful decoding, measured levels, exact loop lengths and loop-boundary measurements in [audio.md](audio.md). Their playback hooks are present in the authored game. Those checks do not establish audible output, event latency, runtime crossfades or the quality of the mixed game bus.

The focused `--viewport-resize` GPU selector passed all four cases in 1.219 seconds under one 60-second deadline. Its added 64 × 64 case checks source-over blending of a translucent rectangle and antialiased glyph, plus the public straight-alpha texture. The previous renderer failed with red 255 where 128 was required. Native presentation and the final pinned browser export were inspected after the fix. The [title](screenshots/title.png) and [Duet HUD](screenshots/duet.png) screenshots show the final export. Both boards responded independently to move, hard drop and hold.

Browser developer instrumentation observed a stereo 48 kHz realtime audio context switch from suspended to running after keyboard interaction. Its ScriptProcessor was connected to the audio destination, and audio time advanced to 20.09 seconds while play, drop and hold were exercised. This verifies browser audio initialization and processing. No human listening review or recording of audible game output is claimed. Runtime mix levels, acoustic output and crossfade quality remain coverage limits.

## Reproduce

Use the engine revision and matching SDK in [toolchain.json](../toolchain.json). From the game root, with the documented build dependencies installed:

```sh
make authoring ENGINE=/path/to/pinned/chibi
make test-build ENGINE=/path/to/pinned/chibi
make test ENGINE=/path/to/pinned/chibi
make validate ENGINE=/path/to/pinned/chibi
```

`ENGINE_BUILD` and `CTL` may select an already-built matching controller. `make test` delegates to `tests/Makefile` and the engine's single supervised deadline. The recorded game run used that route. From the same engine checkout, the admitted groups are `make test-native`, `make test-language-all`, and `make test-tools`; follow that revision's test policy and configured build paths.

The [README](../README.md) describes `make native`, `make web` and `make serve` for local exports. Repeat the observed input, save/reload and resize steps above after rebuilding. A successful build alone does not replace those observations.

The recorded native session launched this exported executable from the game root, with the engine's `tools/test_policy.py` production supervisor wrapping the process at 55 seconds:

```sh
dist/Stillwater.app/Contents/MacOS/Stillwater --frames 2100 --save build/native-smoke-save.json
```

The frame limit alone is not a wall-clock deadline; repeat it through the production supervisor when validating. The explicit path keeps this check's saved state separate from the default project save.

Full native/render/Studio verification and other quarantined suites were not run and are not passed. Physical controllers, prolonged play and broad platform/browser compatibility remain untested. The observed player checks above preceded public deployment.
