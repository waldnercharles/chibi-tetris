# Tetris Effect reference study

## Nine-minute polish pass

This pass is limited to 0:00–9:00 of the same recording. The watch skill
downloaded a local analysis copy. I inspected its 17 selected scene frames,
180 low-resolution frames sampled every three seconds, and a 20-frame detail
sheet of 0:54–0:58. The sheets are retained in ignored
`.chibi/reference-polish`. Their approximate labels describe sampling windows,
not frame-accurate measurements. No later footage was viewed for this pass.
Captions were unavailable and speech transcription was disabled; the findings
below concern visible motion and composition.

The early blue stage leaves most of the image dark. Small cyan clouds appear
around placements and escape the well. The clear near 0:55.7 opens into a dense
horizontal sheet by 0:55.9, with larger highlights among fine grains. Its bright
row remains recognizable for roughly half a second, followed by a cooler,
widely dispersed tail. Around 2:00–2:55, coherent blue streams and a planetary
rim surround the quiet board. The passage near 2:58–3:11 briefly removes the
board, travels through particles, and restores the continuing pile.

From about 3:10, pale concentric geometry and a large triangle give the scene
a different identity. Later passages shift that structure through amber near
5:40 and teal near 7:30. Quiet intervals matter: the brightest triangle and
particle fronts are accents, rather than a constant luminous frame.

The authored changes translate those observations into Stillwater's own
scene: separate fast drop contact and persistent drifting wake, accepted-input
illumination and wind in the existing cloud, a longer clear front, fine block
borders and a lighter grid, and four families of sparse orbital motifs. New
runs and losses share intact outward block flight. Automatic Journey chapters
preserve the pile during a shorter camera passage. The restart choreography is
the user's requested adaptation; this bounded study does not establish a loss
sequence. Stillwater keeps its own rules, music and original geometry.

The earlier notes below belong to the prior study and include different
sampling intervals. They are not evidence that this pass viewed past 9:00.

Studied October 11, 2026 from the supplied [Tetris Effect recording](https://www.youtube.com/watch?v=urbLIyd-VsQ). This study concerns that recording, separately from the [Particula and Tetris Spheres study](reference-particle-flow.md). The local analysis copy is 640 × 360 at approximately 29.97 frames/s and has no audio. Reference media and extracted frames are scratch evidence under ignored `build/particula`; they are not shipped with the game.

Times below are positions in the video, not the game's displayed timer. Native-frame measurements resolve about 33 ms. The clear contact sheet samples every 100 ms; handoff sheets sample every 500 ms. Those samples bound an onset between adjacent frames, rather than establishing an exact event timestamp. There is no input overlay or isolated sound, so button latency, lock delay and synchronization with music cannot be measured here.

## Moves, turns and drops

The observed action treatment is qualitative. Movement and rotation produce a brief local highlight and fine flecks around the active shape. The silhouette and grid position remain the dominant information. Rotation reads as a short local motion around the piece, rather than a large separate ring. Hard drops show a brief bright vertical wash through the traveled space and a compact landing response. The occupied cells settle immediately in the visible sequence; the footage does not establish an exact input-to-placement delay.

These observations support an immediate gameplay placement with a short residual wash, a local contact flash and a dimmer granular tail. They do not justify an exact 60 ms, 90 ms or 160 ms specification. Stillwater's previous 160 ms traveling packet delayed its contact light after the settled piece was already present. The current shorter wash and immediate landing response address that mismatch. The chosen particle counts, spring responses and trail lifetimes remain original tuning; they are documented in [reaction effects](vfx.md).

## A line clear

The [55–57 second passage](https://www.youtube.com/watch?v=urbLIyd-VsQ&t=55s), inspected at 10 samples/s, separates several layers:

| Video time | Observed image |
| --- | --- |
| 55.5–55.6 s | The clear begins between samples. A bright white-cyan row core appears with a dense granular front. |
| 55.7–55.9 s | The front spreads well beyond both board edges. By about 0.3 s after onset, grains occupy most of the image width. Larger bright pieces coexist with much finer dust. |
| 56.1–56.2 s | The coherent white row core breaks down, about 0.5–0.6 s after its first sampled appearance. |
| 56.3–56.9 s | A dimmer central residue persists for roughly one second after onset. Fine grains continue to fall and disperse through apparent depth. The sampled interval does not establish their final lifetime. |

The main differences from Stillwater's original small row burst are density, spatial reach, the sustained luminous core and the separation between heavy fragments and fine trailing grains. A palette change alone cannot reproduce them. The current four-layer clear supplies distinct fracture, chunk, ember and expanding-front populations. Its 240 ms fracture field is still shorter than the approximately half-second coherent core in this passage; its multi-second tails and exact trajectories are authored choices, not measurements from this clip. Rendering must establish whether their combined coverage and brightness match the observed sequence.

## Loss and departing blocks

The [end sequence](https://www.youtube.com/watch?v=urbLIyd-VsQ&t=606s) contains an actual occupied-stack breakup before the following title transition. It is not merely a cut from a static losing board to a title card.

The last sampled stack-shape change occurs around 606.9 s (100 ms sampling). Native frames still show an intact grid at 607.908 s. Separation and rotation begin around 607.941–607.975 s and are unmistakable at 608.008 s. This does not locate the exact logical top-out: the preceding hold could include lock delay or a failed spawn.

The existing solid red cells detach and tumble independently. They retain their color and recognizable cube bodies. Motion is mainly upward and accelerates; lateral spread is modest, approximately one to two cell widths beyond either board edge. The bottom of the departing mass rises about two rows by 608.509 s, five by 609.009 s and eight by 609.376 s. These distances are visual estimates from the low-resolution projection, not world-space measurements. The border, HUD, Hold and Next remain stationary while the cells leave. No large result modal covers the first approximately 1.4 seconds of departure.

A title/planet crossfade begins around 609.25 s and dominates near 610 s. The recording cannot establish whether that transition is native game behavior or an edit, nor the natural retirement time of every block. Stillwater's 700 ms hold, row stagger, analytic acceleration, 2.8-second result delay and four-second decorative clock are therefore original art direction. Its former immediate result had no whole-stack departure. Reusing actual block meshes addresses that central visual gap. The result panel and its timing remain an original continuation beyond the observed departure.

## Stage handoffs

Two sampled handoffs show the same structure. Times are approximate within the 500 ms sampling interval.

| Phase | First handoff | Second handoff |
| --- | --- | --- |
| Quota reached; displayed timer stops | 177.5 s, 30/30, timer 02:49 | 531.5 s, 47/45, timer 07:14 |
| Bright cells become localized particles | 179–180.5 s | 533–535 s |
| Board/HUD disappear; camera travels through particles | 180.5–185.5 s | 535–540.5 s |
| Small new board appears and grows | 186.5 s onward | 541.5–543 s |
| Retained pile reappears bright, then resolves | 189–190.5 s | 543.5–544.5 s |
| Timer and play resume | About 191 s, timer 02:50 | About 545.5 s, timer 07:15 |

Both pauses last roughly 13–14 seconds. Scores remain 6,242 and 43,959 respectively; the quota resets for the next stage. The broad pile shape survives the handoff. This supports a dissolve, journey and reconstruction of the continuing run, rather than an empty-board restart. Exact cell preservation was not measured.

Stillwater currently blends background force regimes and finishes over ten seconds while play continues. That preserves particle continuity, but does not reproduce the reference's paused handoff, disappearing board, camera journey or reconstructed pile. The attract-to-play transition is a separate, shorter effect. These are remaining fidelity gaps, not completed behavior implied by the new particle simulation.

The recording establishes visible outcomes only. It cannot reveal the original particle solver, collision model, bloom implementation or audio mappings. Implementation correctness and rendered acceptance are recorded separately in [validation](validation.md).
