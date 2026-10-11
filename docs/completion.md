# Completion status

Stillwater opens with a live 3D demonstration beside music and mode controls. Explicit Play clears the demo through the shared row-clear effect, centers the camera and reveals a fresh player run. The implementation includes four modes, saved progress, mesh text, procedural scenery and responses to measured music and player actions. See [validation](validation.md) for the exact toolchain, executed checks, captures and remaining limits.

## Rules and modes

The dispatcher exercises the actual authored libraries and behaviors:

- Deterministic seven-piece bags, quarter-turn SRS kicks, hidden spawn rows, collision, ghost placement and row compaction.
- Hold limits, five-piece previews, soft and hard drop, movement repeat, 500 ms lock delay and the fifteen-reset limit.
- Ordinary, T-spin and mini scoring, combos, back-to-back awards, level progression, pause, restart and top-out.
- Six Journey chapters with saved unlocks and chapter keys while paused or after a result, Endless, and the fixed-seed 40-line Sprint with elapsed and best times.
- Duet's independent boards, shared chapter progress, reserve capped at eight, four-charge rescue, and failure when both boards are blocked without a rescue. The final required clear wins even when the following spawn is blocked.
- Separate keyboard and controller slots, disconnect pause, direct mode switching, and save readback in a fresh runtime.

Source-level fixtures make additional assertions inside the authored runtime. Controller checks use synthetic device states.

## Presentation and audio

The well, blocks, ghost, previews and text use 3D geometry. GPU particle fields form a deep spiral and star field that change with the chapter. Snapshot queues preserve the position and result of player actions, driving distinct pooled 3D cues. The lowest effects setting and reduced-motion option stop background motion and retain brief stationary confirmations.

M requests external audio capture. Native capture needs a routed loopback device; browser capture needs the user's audio-sharing selection. Connecting a source waits for explicit Play. Capture failure leaves Play and retry available; Play chooses the bundled score when no stream is ready. The three original music arrangements and twelve generated effects retain their documented provenance, level measurements and loop checks in [audio](audio.md).

Progress, best results and preferences use the SDK save service. Save failures appear as mesh text. Pause, results and settings controls remain inside the game scene. The [README](../README.md) lists the controls and build steps.

## Coverage limits

- Final export and rendering evidence belongs to the exact toolchain recorded in [validation](validation.md). Earlier exports do not establish coverage for this revision.
- Physical controllers remain untested. Synthetic two-slot checks cover the authored behavior.
- Audio-file measurements do not establish a human listening review, capture routing on every host, or permission behavior in every browser.
- Full and quarantined suites remain disabled, not passed.
