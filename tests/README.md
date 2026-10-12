# Focused authored checks

Build from the game root with `make -f tests/Makefile test-build ENGINE=/path/to/pinned/chibi`, then run `make -f tests/Makefile test ENGINE=/path/to/pinned/chibi CTL=/path/to/chibi_ctl`. The engine's production test supervisor applies its 60-second ceiling. The single C/pico_unit dispatcher also enforces one 58-second deadline for all runtime checks and kills its active controller process group on timeout. Build timing is separate. For diagnosis, the binary accepts `--all /path/to/chibi_ctl /path/to/game` or the `STILLWATER_CTL` and `STILLWATER_PROJECT` environment variables. Use `--hud` for display checks or `--consumer` for the visual consumer. Set `STILLWATER_REACTIONS=1` for the player, session, clear, top-out/rescue and production visual-consumer checks under the same supervisor.

The dispatcher clones the authored content with copy-on-write into a fresh `build/resonance/stillwater-tests-*` directory and adds the small fixtures in this directory. It does not modify the game's authored assets. Successful runs remove that directory. Failed runs print its path and retain the last request and response for diagnosis.

The rules fixture calls the live `Pieces` and `Rules` libraries. Board fixtures use the actual `Board` prototype and authored input map, driven through keyboard and two-slot controller input timelines. They check deterministic bags, geometry, SRS floor and wall kicks, collision, ghost placement, row compaction, spin scoring, combos, back-to-back awards, hold limits, auto-repeat, dropping, pause, restart, lock-reset limits, top-out, rescue, the fixed entity pool, and successful-action feedback serials with world-space drop positions.

Reaction fixtures read the fixed 32-slot queues after the production behaviors process input. They check simultaneous action order and positions, accepted and rejected inputs, auto-repeat suppression and rearming, grounded soft-drop feedback, pause/restart boundaries, and queue wrapping. A six-piece keyboard route clears original rows 20 and 21 on consecutive locks, verifying clear and combo records after the board has already collapsed and spawned the next piece. Session serials stay monotonic across restarts and retain setting values and completion results.

Shell fixtures run the actual `World` prototype and `Shell` behavior. Test-only board telemetry supplies clear counts and top-outs to exercise direct Journey launch, chapter unlocks and restarts, Sprint completion, Duet reserves, rescue and failure, controller-disconnect pause, keyboard preference changes and save commits. Real 7 / 8 key inputs select unlocked chapters while paused or after a result, stop at unlock bounds and leave active play alone. A second fresh runtime receives the first runtime's exported `saved_state` and verifies settings and chapter reopening. These isolated snapshots never access the player's save file; the SDK's storage tests cover filesystem transport. Browser storage was checked through save/reload in the real player, as recorded in `docs/validation.md`.

The unowned ReactionField prototype first runs by itself and must remain dormant before its owner configures an event. The production Stillwater scene also runs directly. Its check explicitly chooses the score, exercises drop, pause/resume and mode keys, verifies that the UI snapshot is empty, checks the authored GPU field counts and confirms that effects-off and reduced-motion settings freeze the complete field shape while gameplay remains active. Legacy sprite particle count remains zero. Mode changes retain the same entity pool.

The `--attract` cases cover legal autoplayer evaluation, real demo locks without progress, same-run queue continuity after recovery, explicit Play, shared whole-block departure, camera motion and actual HUD opacity during reveal. The `--onboarding` cases verify that gameplay waits for explicit Play, including direct capture requests, connected-source preservation, score selection, and preserved paused runs. World-space pointer rays exercise both music actions and an outside click through the actual HUD. The controller verifies authored connection state; real browser checks separately verify the permission request.

The `--previews` case drives the production Shell and both Boards. It counts actual hold and next-piece meshes: Solo and Duet hide them during pause and restore them on resume while keeping board context visible. It also runs with `--hud` and the complete dispatcher.

A small 3D scene runs the same production World and Hud with test boards so the snapshot includes every mesh label. The Sprint display check inspects the visible mesh digits and punctuation. It verifies a restored best time of 2:03.4, elapsed-time digits, a clock that remains stopped throughout pause, the missing-best label for a new profile, and chapter keys that do not change Sprint mode. It also checks compact backing geometry behind each visible stats group and the Sprint timers, correct Solo/Duet placement, inactive backing visibility, and separation from text and the pause panel.

These are bounded simulation checks. They do not claim GPU rendering, physical-controller input, audio output, browser playability, or full engine-suite coverage.

`--motion` covers a single-row soft tap, controlled held repeats, no stored-gravity burst, repeated short taps, high gravity, four solid block finishes with a shared bevel, a camera depth-precision margin for their thinnest preview coating, restrained idle emission, music-reactive settled and active coat opacity, physical reaction origins and cosmetic beat motion. `--stage` covers seeded starts, musical/chapter transitions, ten-second morphs through every family including wraparound, removal of surrounding facets, stage-light freeze and reduced motion. `--vfx` covers birth/delay/retirement, physical front-face and landing positions, contact-light timing, four clear layers with exact instance budgets and shader bindings, and saturation without overwriting live cues.

`--loss` drives real boards into top-out in Solo and Duet. It verifies sparse
row occupancy, original colored blocks flying outward and tumbling at full size, the fixed
field budget, frozen controls and clock, preserved scoring, quiet settings,
fixed visible Hold and Next previews and delayed results. It also checks
rescue priority, winning final clears and chapter selection after the loss
phase. Returning from the audio chooser preserves the completed result and
leaves retired blocks hidden. The same selection covers occupied new-game
departures, mode switches, quiet settings, and Journey passages that preserve
the run and freeze input and time. `--vfx` additionally checks four-second drop
wakes, accepted-action ownership, paused Solo/Duet tails and final retirement;
`--music-field` checks that the force targets decay after input stops.
A loss snapshot is simulation evidence;
rendered drift, shader compilation and browser frame time are separate checks.
