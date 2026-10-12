# Stillwater

A falling-block game authored in Chibi. A six-chapter journey unfolds through cyan streams, ivory orbits, amber geometry and teal currents. Light and motion respond to the music and your play.

![Stillwater Journey](docs/screenshots/reference-polish.jpg)

Journey starts at your highest unlocked chapter after you choose music. Each of its six chapters takes twelve lines. After a loss, the blocks tumble away and the main menu returns with your chapter and audio choice retained.

The rules include seven-piece bags, SRS quarter turns, hold, five previews, a ghost, soft and hard drops, a 500 ms lock delay, fifteen lock resets, T-spins, combos, and back-to-back bonuses. Progress, best results, volume, effects, reduced motion and the ghost setting save locally.

## Play

[Published browser release](https://waldnercharles.github.io/chibi-tetris/).

[Edit Stillwater in the browser](https://waldnercharles.github.io/chibi-tetris/dev/). Add `?dev` to the play URL to open the same editor.

The native export is `dist/Stillwater.app` on macOS. Open it normally. The local browser export is `dist/web`; serve it with `make serve`, then open [the local game](http://127.0.0.1:18769). Exports are generated locally and kept out of the source branch.

| Action | Player one | Player two | Controller |
| --- | --- | --- | --- |
| Move | Left / right | A / D | D-pad or left stick |
| Soft drop | Down | S | Down |
| Hard drop | Space | W | D-pad up |
| Clockwise | Up / X | E | A |
| Counterclockwise | Z | Q | B |
| Hold | C / Shift | R | Either shoulder |
| Pause | P | P | Start |

A live demonstration plays beside the opening controls. The main menu offers **Journey** and uses pointer controls. Click **Game options** for particle density, motion, ghost and volume. Click **Connect audio** to share music already playing in another tab, then select that tab and enable audio in the browser's picker. Connection leaves you on this screen. **Play** keeps a connected source, or uses the bundled score when no source is ready. Play clears the demonstration, centers the camera and reveals a fresh board. Browser and operating-system capture support varies; there is no service account sign-in inside the game.

These keyboard controls work during a run, pause or completion; main-menu hotkeys are disabled:

| Action | Key |
| --- | --- |
| Start Journey / Endless / Forty lines / Duet during a run | 1 / 2 / 3 / 4 |
| Pause or resume | P |
| Resume when paused, restart after a result | Enter |
| Restart the same run while paused or after a result | Backspace |
| Previous / next unlocked Journey chapter while paused or after a result | 7 / 8 |
| Connect or reconnect audio while preserving the run | M |
| Cycle effect density | V |
| Toggle reduced motion | F |
| Toggle the ghost | G |
| Lower / raise master volume | 9 / 0 |

Controller 1's A confirms resume or restart; B restarts while paused or after a result. In Duet, controller 1 shares the left board with player one's keyboard keys and controller 2 shares the right board with player two's keys. Either controller's Start pauses or resumes. Disconnecting a controller pauses play. Reconnect and confirm, or press Enter to continue with the keyboard.

Chapter selection starts a fresh run at the selected chapter and stays paused. Press Enter when ready. The chapter keys stop at the first and highest unlocked chapters. New games release the old blocks outward before the fresh board appears. Earning a Journey chapter preserves your pile through a brief camera passage; reduced motion skips it.

External music uses browser audio sharing or a native loopback device. Click **Connect audio** on the menu, or press M during a run. The scene responds to its measured sound. See [audio capture](docs/audio.md#music-reaction-and-capture) for setup.

### Phone controls

Open the [browser release](https://waldnercharles.github.io/chibi-tetris/) on your phone. Phone View appears automatically on touch devices and narrow windows; **Phone view** on the opening menu or **H** toggles it manually. Portrait puts the score and piece previews above the well and the controls below it. Landscape puts the controls alongside the board. The 3D blocks, particles, music response and effect settings remain available. Screen rotation keeps the current run.

Tap **Enable tilt**, allow motion access if the browser asks, and hold the phone in a comfortable position while it centers. Tilt left or right to move. Rotation, soft drop, hard drop and hold have separate touch buttons, along with pause and restart. Left and Right also work without a sensor. Pause to **Recenter tilt** or turn tilt off. Switching away from the game pauses play; returning waits for Resume.

**Vibration** on the opening or pause screen enables short pulses for successful moves, turns, holds, drops, locks and clears. The browser exposes pulse duration, not directional or spatial haptics. Safari on iPhone currently does not expose this vibration API; unsupported browsers show **Vibration unavailable** and retain the game's visual feedback. Motion requires HTTPS (the published site uses it), browser permission and a working sensor. Denying permission leaves touch controls usable. See the engine's [device API and browser constraints](https://github.com/waldnercharles/chibi/blob/cwaldner/phone-device-controls/docs/device-input.md).

Layout uses logical viewport dimensions. Cute's default high-DPI support scales the render target separately, preserving sharp text and blocks without changing control size. The browser shell follows its visible height and keeps controls clear of the notch and home indicator. Physical sensor feel and vibration still need a check on your actual phone.

## Build from source

This game consumes the Chibi SDK. KDL owns scene composition and material graphs, `.chibi` owns game rules and presentation, and ordinary GLSL files in `content/shaders` own particle motion and appearance. Blocks, scenery and text use the 3D renderer. There is no game-specific native module or separate browser implementation.

The exact engine commit and package identities are recorded in `toolchain.json`. Use that checkout through `.chibi/engine`, or pass `ENGINE=/absolute/path/to/that/checkout` to Make. The engine's documented CMake, C compiler, libclang and platform dependencies are required. Browser packaging also requires Emscripten.

```sh
git clone --no-checkout https://github.com/waldnercharles/chibi.git .chibi/engine
git -C .chibi/engine checkout "$(python3 -c 'import json; print(json.load(open("toolchain.json"))["engine"]["revision"])')"
make sdk
make authoring
make play
make native
make web-player
make web
```

`SDK`, `CTL`, `ENGINE_BUILD`, and `WEB_PLAYER` can select existing matching installations. SDK and build output live under ignored `.chibi/`, `build/`, or `dist/` directories. Native exports include the player, prepared game, resources, and dependency licenses. Browser exports use Chibi's WebGPU player with its WebGL 2 fallback.

## Publish to GitHub Pages

Every completed game iteration is published to the same public play URL. GitHub Pages automatically deploys the root of `gh-pages` after publication. Run the focused checks and inspect browser play with the pinned toolchain before publishing.

Commit and push the tested source branch. If the engine pin changed, push its source branch too. Then publish from the clean source checkout (requires Git, GitHub CLI, jq, curl, rsync and shasum):

```sh
git push -u origin HEAD
make publish
```

`make publish` checks the remote source revision and pinned SDK/player, exports the game and its `/dev/` editor, preserves dependency licenses, writes `deployment.json` and `SHA256SUMS`, and pushes the export. It removes its temporary checkout on exit. Source pushes alone do not update the game.

The editor package must explicitly support the game's pinned engine revision. Publishing downloads and verifies the matching reusable Studio package. A feature engine without a matching public Studio package uses the game repository's `web-editor-<engine revision>` release, containing `studio.json` and the archive it names. `EDITOR=/path/to/editor` can select a locally built matching package.

The manifest records the actual clean SDK and Studio source revisions and the archive checksum. By default its `engine` must equal the game pin. A reviewed SDK repair can also declare specific `compatible_engines` revisions after checking that public APIs, serialization and runtime behavior remain compatible, including project format and native functions; a shared format number alone is insufficient. This lets an SDK memory fix serve an existing game release without changing its pin.

Projects with prototype `includes` also require the editor's `capabilities.prototype_includes` flag. Projects with WAV or OGG sound assets require `capabilities.project_audio`.

The **Publish game editor** workflow refreshes `/dev/` after successful Pages deployments, so older source branches that publish only a player still receive an editor for their deployed source revision. It can also be run manually, or locally with `make publish-dev`. It preserves the deployed game and reads its `deployment.json`; it never substitutes source from a different release. Editor workspaces save locally and are isolated by source revision.

In the pinned editor, edit prototypes using `includes` and their composed dependencies in the Text Editor. Inspector saves report an error and preserve their authored source. Changes to child definitions or entity-reference paths require reopening the project; root scalar edits update the live scene. Sound-file changes also require reopening the project.

Wait for the repository's Pages deployment to finish. `make verify-published` uses the engine's 60-second production supervisor to compare the public revision, checksum manifest, game and player assets with the published branch. Then open the public URL and check pointer menu controls, movement/drop, audio response and loss recovery in a bounded browser session. Publication is complete only after those checks pass.

### Reuse for another repo or project folder

The reusable workflow accepts a project folder and a folder within the deployed site. A repository that already publishes a player and `deployment.json` to `gh-pages` can call it with:

```yaml
jobs:
  editor:
    permissions:
      contents: write
      pages: write
    uses: waldnercharles/chibi-tetris/.github/workflows/dev.yml@main
    with:
      project-path: projects/tetris
      site-path: tetris
```

Use `.` for a standalone game. Each project owns its `toolchain.json`; each deployed folder owns its `deployment.json`. The local assembly helper also accepts explicit project, site and installed editor directories: `bash tools/assemble-dev.sh PROJECT_ROOT SITE EDITOR_DIR`.

For a future combined examples site, use `projects/<name>` for source and `<name>/` plus `<name>/dev/` for the published routes. Assemble all selected projects before one publishing commit. This keeps each game's engine pin and provenance explicit while allowing one Pages deployment. A shared workflow avoids copying the publishing implementation into a GitHub template; no repository moves are needed to adopt it.

## Verify and edit

```sh
make format
make validate
make test-build
```

The C/pico_unit dispatcher calls the real authored libraries and behaviors through the controller. It covers deterministic rules, both keyboard and controller slots, lock/reset/hold behavior, topout/rescue, Journey and Sprint completion, cooperative progression, disconnect pause, save readback, the music gate, controlled soft drop, piece-surface reactions, scene transitions and the 3D environment's effect controls. Run the affected selectors described in [tests](tests/README.md) under the engine's 60-second production supervisor. The game-wide aggregate is quarantined. Configuration and compilation are separate from that deadline.

`content/behaviors/Board.chibi` handles each board, `content/libraries/Pieces.chibi` and `Rules.chibi` own piece geometry and rule calculations, and `Shell.chibi` owns progression and in-game controls. `StageDirector.chibi` transitions between four sound-selected particle formations and their lighting. `Environment.chibi` drives two GPU particle fields and fixed pools of reaction geometry and lights. Ordinary GLSL in `content/shaders` defines persistent particle advection, birth and appearance; both fields retain position, velocity and age on the GPU. `AudioReactive.chibi` reads measured audio, `MusicResponse.chibi` combines transients with tracked tempo and phase, and `MusicCharacter.chibi` identifies sustained sound traits and passage changes. These traits also shape new action trails and block finishes. `Hud.chibi` places mesh text. Test-only fixtures stay in `tests/fixtures` and are never exported.

Read [validation](docs/validation.md) for executed checks and limits, [rules](docs/rules.md) for scoring, [audio](docs/audio.md) and [visuals](docs/visuals.md) for asset provenance, and [licenses](LICENSES.md) for terms.
