# Tetris Effect reference study

Studied the supplied [Tetris Effect Connected gameplay recording](https://www.youtube.com/watch?v=oEeGq3srOgM&t=56s) in the browser on October 11, 2026. This is a visual study of selected sequences, including frame stepping through the clear at 1:34 and sampling later stages and the ending. It is not a claim to have measured the original game engine or watched every frame. The browser delivered a 1280 × 720 video; its frame-step control advanced the media clock by about 1/30 second. Compression and possible capture stalls limit timing precision.

## The ordinary clear

The double clear around [1:34](https://www.youtube.com/watch?v=oEeGq3srOgM&t=94s) has three distinct parts. The completed rows become almost white while their square silhouettes remain readable. A dense sheet of fine blue-white particles opens beneath and outside the board. The luminous cells then lose their solid appearance into granular debris, while the wider particle sheet keeps moving and fading.

| Video time | Observed state |
| --- | --- |
| 1:34.127 | Rows remain blue; clear has not visibly begun. |
| 1:34.327 | Two rows are brilliant white-blue. A wide, dense spray already extends below the board. |
| 1:34.527 | Luminous square cells remain visible, with more granular breakup. |
| 1:34.727 | The row silhouettes are gone. Surviving blocks have dropped; fine particles remain inside and outside the well. |
| 1:34.927–1:35.127 | The next piece is available while the dim particle tail continues. |

The sampled bounds suggest roughly **0.3–0.5 seconds of recognizable luminous row shapes**, not several seconds of maximum brightness. The exact start and end are between sampled frames. The particle tail visibly outlasts both the white row and the delay before the next piece. Its precise endpoint cannot be isolated from the persistent background particles in this recording.

The debris reads primarily as many fine grains and small square fragments, with a few larger highlights. It does not resemble a few enormous tumbling cubes or two parallel drop lines. The source spans the actual cleared row. The spray extends laterally and toward the foreground, then loses brightness and density. Depth comes from scale variation, occlusion around the well, and separate foreground and distant particle layers. The video's pixels do not establish the engine's actual blend modes or lighting implementation.

## Input, material and composition

At [1:03](https://www.youtube.com/watch?v=oEeGq3srOgM&t=63s), ordinary falling pieces have softly luminous blue edges, darker faces and a simple rounded square silhouette. The ghost is a thin, brighter outline. Sparse grains above and around the well are much quieter than a clear. In the 1:33–1:35 sequence, placement preserves crisp grid alignment; a clear changes the whole scene's visual energy. The video contains no input overlay, so key-to-effect latency, failed-input behavior and exact rotation-versus-movement attribution cannot be measured reliably.

The early scene is mostly dark. By [5:39](https://www.youtube.com/watch?v=oEeGq3srOgM&t=339s), large coherent schools of particles, a planet rim and layered haze occupy the surrounding space. The well remains a quiet dark region. At [9:26](https://www.youtube.com/watch?v=oEeGq3srOgM&t=566s), the palette changes to amber and the blocks become sharp outlined squares with diagonal faces. At [13:13](https://www.youtube.com/watch?v=oEeGq3srOgM&t=793s), blocks are red with distinct inset centers, surrounded by rotating forms and smoke. At [16:59](https://www.youtube.com/watch?v=oEeGq3srOgM&t=1019s), those forms fill much more of the screen.

The important variation is structural: different silhouettes, depth distributions, shapes and motion fields accompany the palette changes. Each stage has a coherent visual vocabulary. Background motion can be elaborate because the board and HUD occupy quieter regions.

## Completion and a full board

The recording ends in successful area completion. At [18:14–18:20](https://www.youtube.com/watch?v=oEeGq3srOgM&t=1094s), the count changes from 35/36 to 37/36. The completed rows remain luminous at the sampled 18:16.580 and 18:17.580 frames, then dissolve while fine particles, larger pink-white highlights, smoke and a red scene wash spread throughout the view. The board and camera composition also move during the transition. This is a longer, scene-wide completion treatment than the ordinary 1:34 clear.

I did not establish a top-out or completely filled Zone board in the inspected sequences. The successful completion cannot be used as evidence for those states. Their timing and choreography need a separate reference that visibly contains them.

## Proposed application to Stillwater

These are design recommendations from the reference, not additional implemented changes in this release.

| Layer | Purpose and geometry | Emission and motion | Timing and compositing |
| --- | --- | --- | --- |
| Retained row silhouette | Let the player recognize exactly what cleared. Keep the existing solid cells with an emissive material ramp. | Each cleared cell, at its real depth. Little or no displacement initially. | Approximately 0.3–0.5 seconds total, with a shorter peak. Opaque or alpha surface with restrained bloom. |
| Surface breakup | Transfer the cell silhouette into fine detail. Small lit mesh chips and grains; no soft smoke texture. | Emit across the cell faces and exposed edges. Brief outward velocity with exponential drag. | Begin during the luminous hold; fade through roughly 0.5–1.2 seconds. Depth-tested alpha; only a minority of hot grains need additive contribution. |
| Wide clear wake | Give the event scale beyond the board. Small sharp grains with varied sizes. | A broad sheet from the row, biased downward/outward and into depth, with continuous curl. | Strong initial emission, rapid falloff, then a dim 2–6 second tail. Keep the region behind active play quiet. |
| Scene response | Connect gameplay to the surrounding visualizer. Reuse existing background particles and lights. | A bounded wave starts at the cleared row, changing velocity, size and radiance with depth. | Fast response and slower recovery. Reserve the longest and brightest response for four-line clears and completion. |

For drag coefficient k, use v(t) = v₀ exp(−kt) and displacement v₀(1−exp(−kt))/k. For a sharply decaying emission profile over duration T, use r(t) = r₀[1−log(1+at/T)/log(1+a)] for 0 ≤ t ≤ T and a > 0. A separate smooth birth ramp and smooth death envelope control alpha and size. Longer tails should gain travel and changing form, not a longer white bloom peak.

The current release already lengthens and cools clear tails, varies background formations and adds spatial input forces. The strongest next change suggested by the footage is to preserve the recognizable row silhouette before its fine breakup, and to give completion its own choreography.

## The open-source comparison

[dcts/tetris-spheres](https://github.com/dcts/tetris-spheres/tree/401f083cc0e825ff1192f17fc4ee9b686e02d05b) provides a useful foreground/background composition reference. Its [background is a looping video](https://github.com/dcts/tetris-spheres/blob/401f083cc0e825ff1192f17fc4ee9b686e02d05b/index.html#L24-L27). Its [650 ms board bounce](https://github.com/dcts/tetris-spheres/blob/401f083cc0e825ff1192f17fc4ee9b686e02d05b/css/style.css#L21-L31) reaches a 5 px offset at 30% progress. [Clearing removes rows directly](https://github.com/dcts/tetris-spheres/blob/401f083cc0e825ff1192f17fc4ee9b686e02d05b/js/GameField.js#L68-L82), with no timed shine or fragment system. It therefore cannot supply the particle or breakup techniques visible in the gameplay recording. No code or media was copied.
