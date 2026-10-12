# Reaction effects

`Environment.chibi` consumes immutable action records. A fixed pool of 96 GPU
fields and eight point lights holds the reactions. No grain creates an entity
or runs a behavior, and a live field is never moved to a newer event. The main
131,072-grain cloud and 16,384-grain scenery field remain separate populations.

## Player actions

| Action or layer | Instances | Field life | Motion |
| --- | ---: | ---: | --- |
| Move | 48 | 1.65 s | Feathers away from exposed trailing edges, then follows the live current. |
| Rotate | 96 | 2.3 s | Leaves the captured SRS pivot tangentially, then bends through shared forces. |
| Soft fall | 32 | 440 / 520 / 300 ms | Start, step and stop; the step wake spans its captured travel. |
| Hold | 48 | 720 ms | Draws inward and forward through depth. |
| Rejection | 20 | 280 ms | Marks the rejected edge or rotation perimeter. |
| Hard-drop beam | 96 | 400 ms | Thin shafts occupy the traveled volume at varied depths and lengths, then cool. |
| Hard-drop plume | 768 | 4.8–6 s | Starts along the traveled path, rises in an offset school and spreads toward and away from the camera. |
| Contact flecks | 96 | 620 ms | Starts immediately at the exposed lower edges. |
| Settling dust | 128 | 1.05 s | Starts 25 ms after contact and drifts through depth. |

A hard drop emits the beam, plume and two landing layers. Its following lock
record does not repeat them. Natural locking uses the landing layers alone.
Logical placement remains immediate. Ordinary cues begin at the four captured
occupied cell fronts, including the block finish and music lift at that action.
Internal edges do not emit grains through adjoining occupied cells.

Move, turn, drop, clear embers and clear fans retain GPU position, velocity and
age. `ParticleFlow.glsl` supplies the same curl and spatial impulse functions
used by the ambient cloud. Four bounded origins carry overlapping action
forces. Live uniforms change acceleration; a later action does not teleport
older grains or replace their captured birth positions. Update and display
share the same birth calculation so a delayed grain does not travel invisibly
before appearing. This is shared-force particle advection, not pairwise
collisions or a pressure-based fluid simulation.

About one-third of plume grains initially move forward; the others recede.
Perspective, differing sizes and continued acceleration give the cloud depth.
A soft forward bound keeps grains away from the camera plane. Resting scenery
is dimmer than active plumes. Measured musical attacks increase its light and
radial/curl acceleration without manufacturing beats during silence.

Accepted input also drives small filtered active-block lean, compression and
emission. Landed blocks settle independently of the next piece. A critically
damped spring lowers the well after a hard drop and returns smoothly, retaining
velocity across overlapping drops. Logical cells and ghost placement stay
fixed. The ghost uses four existing thin cells with transparent interiors and
masked shared edges, forming one continuous outside silhouette. Block faces
are darker with narrower luminous borders so landing illumination remains
visible.

## Light and antialiasing

The grains are emissive and can receive scene light. An emissive grain does
not automatically become a point light. Instead, landing and clearing drive
the eight actual pooled point lights. Landing lights sit 14 units ahead of the
captured face; clear lights sit 16 units ahead. This illuminates nearby front
faces more directly than the former grazing placement. The light has a 25 ms
attack and a one-second cooling envelope. Quiet settings rapidly retire it;
a fully occupied pool skips new lights instead of moving a live one.

The pinned renderer tone-maps the HDR scene, then runs an edge-aware FXAA pass
before UI composition. Geometry and additive particles use the same path on
native, WebGPU and WebGL. Flat interiors remain unchanged. The player supplies
the display pixel density to its viewport while preserving logical input
coordinates. Bloom and emitted brightness remain separate from antialiasing.

## Cleared rows

Every original row in the recorded mask receives four layers. The origin is
the board center in X, `276 - 24 * row` in Y and the captured front in Z. It
never reads the next piece position or the compacted grid. Four cleared rows
create 10,624 instances across sixteen fields.

| Layer | Instances per row | Delay / field life | Treatment |
| --- | ---: | --- | --- |
| Fracture front | 320 | 0 / 500 ms | Bright, thin splinters retain the coherent row before cooling. |
| Heavy fragments | 96 | 35 ms / 1–1.35 s | Opaque chunks tumble with drag and gravity, writing depth. |
| Microembers | 2,048 | 55 ms / 3.2–6 s | Fine persistent grains spread through depth and join the live current. |
| Outer fan | 192 | 20 ms / 3.116–5.58 s | Faster lateral grains broaden the clear around the well. |
| Row illumination | One light | 20 ms / 1 s | Actual local light responds at the original row front. |

The release captures the current song response for its lifetime. Tempo
confidence blends a default half-second period toward the measured interval;
energy and bass modestly extend the tail. Later beats can bend the current
without abruptly changing a field's retirement time. The bright opening cools
over 550 ms. Fracture, embers and fan use source-alpha additive blending with
depth testing and no depth writes. Heavy fragments use opaque depth-tested
geometry. Grain size and opacity retire continuously.

## Departure and menu recovery

Top-out stops control and the run clock immediately. Duet rescue and winning
final clears take priority. After a 700 ms hold, visible rows release from top
to bottom at 12.5 ms intervals. The actual colored block meshes fly outward
and forward, rising and tumbling at full size. They retire after leaving the
well; their geometry is never scaled to zero. Sparse occupied-row grains
accompany the departure.

After the 2.8-second loss phase, the main menu returns directly. It retains
the reached Journey chapter and audio choice; score persistence happens before
that return. The camera eases toward the menu. Clicking Play during that ease
starts from its current pose. Pointer Play begins one new run through the same
departure choreography; no Game Over panel appears. Gameplay and pause retain
their keyboard controls, while the main menu offers only Journey and uses
pointer controls.

Automatic Journey chapters preserve the pile and score during a 2.6-second
camera passage. Reduced motion skips that passage and retires departing
blocks without movement or shrinking. Gameplay field, light and well clocks
freeze while paused. Quiet/effects-off settings preserve game rules while
reducing or stopping decorative motion.

## Bounds and verification

The largest field contains 2,048 instances, giving a 196,608-instance upper
bound for the fixed reaction pool. Ordinary events use far fewer. When full,
the consumer increments `VisualState.cue_dropped` and leaves living fields
unchanged. `cue_overwrites` stays zero. Medium quality halves ordinary counts;
quiet settings use eight grains and short lifetimes. Neither field nor light
entity counts grow during play.

Authored fixtures exercise captured positions, exposed edges, birth/delay and
retirement, clear row masks, pause, quiet controls, pool saturation, new-run
handoff and menu recovery. Rules checks cover all ghost edges across all
seven pieces and four rotations. Renderer checks establish fractional
silhouette coverage for solid and additive geometry, unchanged flat interiors,
particle depth and logical input at doubled pixel density. Those checks do not
replace rendered motion inspection or warmed browser frame-time measurement.
See [validation](validation.md) for the executed coverage and its limits.
