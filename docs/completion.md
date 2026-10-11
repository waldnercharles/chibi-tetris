# Completion status

Stillwater's authored game now starts directly in a 3D Journey. The implementation includes four modes, saved progress, mesh text, procedural scenery and responses to measured music and successful player actions. The bounded game check passes ten C/pico_unit cases with 428 assertions against the integrated version 22 SDK. See [validation](validation.md) for the final toolchain, executed checks, captures and remaining limits.

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

M switches between analysis of the real game output and external audio capture. Native capture needs a routed loopback device; browser capture needs the user's audio-sharing selection. Capture failure restores the bundled score. The three original music arrangements and twelve generated effects retain their documented provenance, level measurements and loop checks in [audio](audio.md).

Progress, best results and preferences use the SDK save service. Save failures appear as mesh text. Pause, results and settings controls remain inside the game scene. The [README](../README.md) lists the controls and build steps.

## Coverage limits

- Final export and rendering evidence belongs to the exact toolchain recorded in [validation](validation.md). Earlier exports do not establish coverage for this revision.
- Physical controllers remain untested. Synthetic two-slot checks cover the authored behavior.
- Audio-file measurements do not establish a human listening review, capture routing on every host, or permission behavior in every browser.
- Full and quarantined suites remain disabled, not passed.
