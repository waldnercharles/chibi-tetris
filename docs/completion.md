# Completion status

Stillwater's authored game, original assets, and local native/browser exports are implemented. Deterministic checks and observed keyboard play pass. Native save reopening and the focused renderer regression also pass. Browser audio initialization and processing were observed; physical controllers and audible mix quality remain unverified. See [validation](validation.md) for the evidence and limits.

## Rules and modes

Implemented and covered by the C/pico_unit dispatcher against the actual authored libraries and behaviors:

- Deterministic seven-piece bags, quarter-turn SRS kicks, hidden spawn rows, collision, ghost placement and row compaction.
- Hold limits, five-piece previews, soft/hard drop, movement repeat, 500 ms lock delay and the fifteen-reset limit.
- Ordinary, T-spin and mini scoring, combos, back-to-back awards, level progression, pause, restart and top-out.
- Six Journey chapters with saved unlocks and selected-chapter restarts, Endless, and the fixed-seed 40-line Sprint.
- Duet's independent boards, shared chapter progress, reserve capped at eight, four-charge rescue, and failure without a rescue. The final required clear wins even when the following spawn is blocked.
- Separate keyboard and controller slots, disconnect pause, and explicit reconnection or keyboard fallback. Controller coverage uses synthetic device states; physical controllers have not been tested.

The eight game cases pass with 95 C assertions. Source-level rule fixtures make additional assertions inside the authored runtime.

## Presentation and access

Implemented: three original ocean paintings, three related music arrangements, twelve effects, fixed visual-entity pools, chapter transitions, piece colors, event feedback, and reduced-motion/effects settings. Audio files decode and have measured levels and loop boundaries; browser playback initializes a running stereo audio context. Audible output and runtime mix quality have not received a listening review.

The main menu, guide, pause, results, chapter selection, audio, movement, effects, reduced-motion and ghost controls are authored in the game. Native keyboard play exercised dropping, holding, pause and settings. Browser keyboard play exercised solo and both Duet boards, drop, hold, pause and settings, including a 960 × 540 viewport.

Chapter unlocks, scores, Sprint best and settings use the SDK save service, with a visible save-failure notice. Simulation checks verify committed values in a fresh runtime. A native settings change from Master 80 to 70 reached an isolated disk save selected with `--save`; browser Master 70 survived a page reload. The reopened native app displayed Master 70.

## Coverage limits

- The focused GPU regression and final browser inspection pass with the UI blend repair. Screenshots are linked in the validation record.
- Browser audio processing was observed, but there is no recording or human listening review of the audible mix.
- Physical controller hardware remains untested. Synthetic two-slot tests cover the authored behavior.

Project validation passes for 38 assets with no errors or warnings. The admitted engine native, language and tooling groups pass. Full and quarantined suites remain disabled, not passed. See the [README](../README.md) for browser access and publishing steps.
