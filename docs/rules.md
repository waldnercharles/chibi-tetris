# Stillwater board rules

Stillwater uses a ten-column field with twenty visible rows and two hidden spawn rows. The seven four-cell pieces spawn in their standard orientation at column 3, row 1. Coordinates in the source start at the top left and increase downward. Hidden cells participate in collision and line clears.

## Pieces and movement

Each bag contains all seven pieces exactly once. Fisher-Yates shuffling uses a local Park-Miller generator and rejection sampling, so the same session seed produces the same sequence on native and web players. Duet boards start with identical bags and advance independently. Five upcoming pieces remain visible. The bag structure follows the [NullpoMino randomizer's seven-piece rule](https://github.com/nullpomino/nullpomino/blob/master/nullpomino-core/src/main/java/mu/nu/nullpo/game/subsystem/randomizer/BagRandomizer.java); Stillwater's generator and shuffle are independent implementations.

Clockwise and counterclockwise turns use the five ordered Super Rotation System tests. The I piece has its own kick table and half-cell pivot; O occupies the same cells in every orientation. Offsets were checked against the [primary NullpoMino SRS implementation](https://github.com/nullpomino/nullpomino/blob/master/nullpomino-core/src/main/java/mu/nu/nullpo/game/subsystem/wallkick/StandardWallkick.java). Stillwater implements only quarter turns.

Hold stores or exchanges one piece and resets the replacement to its spawn orientation. It is available once between locks. A ghost shows the current landing cells and can be disabled. Hard drop lands and locks immediately. Soft drop accelerates gravity by twenty times, with a minimum of fifty cells per second. The [official licensed Mattel manual](https://service.mattel.com/instruction_sheets/N0494.pdf) describes hold and immediate hard drop; the timings here are Stillwater's choices.

Horizontal input moves immediately, waits for the configured delayed auto-shift, then repeats at the configured interval. A zero repeat interval moves to the wall after the delay. With both directions held, the latest pressed direction wins. Rotation and hard drop use press edges, so holding a button does not repeat them. Pause freezes simulation and resets horizontal repeat timing.

Ground contact starts a half-second lock timer. A successful grounded shift or rotation can restart it at most fifteen times for one piece. The timer continues during subsequent airborne kicks; it can only lock while grounded. Failed actions never restart it. Hard drop bypasses the timer. Input is processed before gravity, with hold first, then horizontal movement, rotation, and drop.

## Scoring

| Action | Base points |
| --- | ---: |
| One / two / three / four lines | 100 / 300 / 500 / 800 |
| Full T-spin with zero / one / two / three lines | 400 / 800 / 1200 / 1600 |
| Mini T-spin with zero / one / two lines | 100 / 200 / 400 |
| Consecutive line clear after the first | 50 × combo index |
| Soft drop | 1 per cell |
| Hard drop | 2 per cell |

Line, spin and combo awards use the level before the clear. Drop points do not scale with level. Consecutive four-line clears or spin clears receive a 1.5 multiplier on their base award. A placement without a clear preserves that chain; an ordinary one-, two-, or three-line clear breaks it. Any placement without a clear breaks the combo. The [official mobile scoring reference](https://playstudios.helpshift.com/hc/en/16-tetris-mobile/faq/2437-scoring-in-tetris/) provides the standard clear and full-spin values. Stillwater deliberately specifies the combo, mini and drop rules above and makes no claim of licensed guideline conformance.

A T-spin requires T, a successful rotation as the last movement, and three occupied corners around its pivot. Two occupied front corners make a full spin. Otherwise it is a mini, except a successful fifth kick upgrades it to full. A successful horizontal shift or downward movement clears the rotation marker; a hard drop that moves zero cells preserves it.

## Progress and failure

Endless, Sprint and Duet gain a level every ten cleared lines. Journey uses its chapter number as the level, starting at one. Stillwater's gravity curve begins at 0.8 seconds per row and reaches twenty rows per 60 Hz frame at level twenty. The authored table in `Rules.Gravity` is the full specification; the calmer progression is original tuning.

Rows compact immediately, retaining their order. Optional 140 ms flashes mark the old clear rows without delaying input. Reduced motion and effects-off disable those flashes.

A blocked spawn ends the board. Locking all four cells in the hidden rows without clearing a line also ends it. The field has a solid ceiling above the two hidden rows. These explicit limits avoid placing cells outside the grid.

Duet rescue shifts existing cells down six rows and discards the bottom six. It clears the failure state, restores the current piece at spawn, and makes hold available. Score, line count, held piece and queued sequence remain intact. Rescue does not award line-clear points. Session logic owns when a rescue is available and how shared progress wins or loses the run.

## Authored implementation

`Pieces.chibi` owns piece geometry, SRS offsets, colors and bag generation. `Rules.chibi` owns field operations and score calculations. `Board.chibi` owns input, per-piece timers and rendering. `BoardState` exposes live telemetry to the HUD and scenario checks.

Each board creates 232 reusable block entities once: 200 settled or empty grid cells, four active cells, four ghost cells, four held cells, and twenty preview cells. Only changed groups redraw. The board frame is its owner's appearance. No per-frame entity creation or destruction is needed.
