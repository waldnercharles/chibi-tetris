# Reaction effects

`Environment.chibi` consumes immutable reaction records. `ReactionParticles.Cue`
evaluates ordinary contact grains; `ReactionParticles.Release` evaluates clear
layers. Both evaluate solid octahedra on the GPU from their index, seed and age. The fixed
pool holds 96 fields and eight local lights. No grain creates an entity or runs
a behavior, and a visible field or light is never moved to a newer event.

Ordinary input starts on the four captured occupied cell fronts. Their depth
includes the block finish, chapter depth and music lift at the action. The
effects stay local, with a brief white accent and longer dim tails.
Grains have small variations in proportion and tumble, and receive some scene
lighting. A turn follows an arc around the captured SRS pivot, including the I
piece's half-cell offset.

| Action or layer | Instances | Field life | Placement and timing |
| --- | ---: | ---: | --- |
| Movement | 48 | 620 ms | Spans the vacated cell behind exposed trailing edges. |
| Rotation | 96 | 720 ms | Curves from the exposed silhouette around the SRS pivot. |
| Soft fall | 32 | 440 / 520 / 300 ms | Start, step and stop; the step wake spans its captured travel. |
| Hold | 48 | 720 ms | Draws slightly inward and forward through depth. |
| Rejection | 20 | 280 ms | Marks the wall, grounded lower edge or rejected rotation perimeter. |
| Hard-drop wash | 96 | 90 ms | Scattered across traveled cells; contracts in 60 ms and vanishes by 75 ms. |
| Hard-drop wake | 768 | 4.8–6 s | Samples the whole traveled path, spreads beyond both board edges and drifts into the background. Captures the current wind once; fine dust includes sparse larger flecks. |
| Contact flecks | 96 | 620 ms | Starts immediately on exposed lower cell edges. |
| Settling dust | 128 | 1,050 ms | Starts 25 ms after contact and drifts gently through depth. |

A hard drop uses the wash, persistent wake and both landing layers. Its immediately
following lock record does not replay them. A natural lock uses the two landing
layers without a wash. Internal edges are masked, so cells do
not emit grains through the middle of their shape. The contact light starts
immediately and has a peak setting of 0.44; a natural lock uses 0.324. Their shared
light envelope stays below those settings.
The wake mixes 1.2–2.8-unit grains with sparse flecks up to 5.2 units. Its
stronger sideways velocity clears the opaque well before the grains retreat
through depth; size and opacity fade continuously to zero before retirement.

Accepted movement and rotation also add capped impulses to the active blocks'
lean, depth compression and emission. Exponential filtering retains the current
pose through overlapping inputs. Newly locked cells settle independently of the
next piece; logical cell positions, collision rules and the ghost stay immediate.
The same accepted action origins illuminate and accelerate the surrounding
cloud. Movement and turns bend its wind; force targets decay after input stops.
Gameplay field and light clocks freeze while paused, including overlapping
wakes from both Duet boards. Menu confirmations can still complete.

## A cleared row

Every original row in the recorded mask gets its own four-layer release. The
field origin is the board center in X, `276 - 24 * row` in Y, and the average
captured block front in Z. It never reads the new piece position or compacted
grid. Particle seeds place grains across all ten original block faces, with
eighteen units of scatter inside each cell. Four rows create 10,624 instances
across sixteen fields. The prior release used 112 instances per row.

| Layer | Instances per row | Delay / field life | Geometry and motion |
| --- | ---: | --- | --- |
| Fracture front | 320 | 0 / 500 ms | Thin bright splinters, 2.2 by 0.38 by 0.6 proportions. A center-out front preserves the coherent row longer before cooling into cyan or occasional amber. |
| Heavy fragments | 96 | 35 ms / 1–1.35 s | Opaque chunks, 2.2 to 5.8 units before the envelope, with independent uneven proportions and tumbling. Initial X speed reaches 220 units/s, Y starts at 50 to 150 units/s, and Z starts at 20 to 65 units/s. Gravity is -250 units/s² and drag is 1.8/s. |
| Microembers | 2,048 | 55 ms / 3.2–6 s | Fine solids, 0.45 to 1.52 units before the envelope. They disperse through depth with 0.9/s drag, -7 units/s² gravity and bounded curl drift. Dimmer larger grains give the trailing dust a broken texture. |
| Expanding energy | 192 | 20 ms / 3.116–5.58 s | A sparse elliptical front begins at the row perimeter. Its horizontal and vertical radii grow by `180F` and `90F`; depth advances by `30F`, using 1.1/s drag. It leaves the center open. |
| Row illumination | One light | 20 / 300 ms | Centered on the actual row, two units ahead of its captured front. Strength ranges from 0.89 for one row to 1.1 for four. The existing light pool skips a new light if all eight are still alive. |

The release captures the current song response once, so its lifetime cannot jump
when a later beat arrives. Let `c` be tempo confidence, `p` the measured beat
period in seconds, and `s = clamp(0.7 energy + 0.3 bass, 0, 1)`:

```
period = 0.5 + (clamp(p, 0.3, 0.9) - 0.5) * c
ember_life = clamp(7 * period + 0.8 + 0.7 * s, 3.2, 6)
energy_life = 0.88 * ember_life + 0.3
chunk_life = 1 + 0.35 * s
```

The bright opening cools over 550 ms regardless of tail length. Ember and energy
alpha also multiply by `0.3 + 0.7 exp(-2.8a)`, leaving dim drifting grains rather
than holding a bright glow for six seconds. Departing-block release fields
survive the new-run handoff; ordinary action cues retire through the existing
sixty-millisecond envelope.

Fracture, ember and energy grains use source-alpha additive blending with depth
testing and no depth writes. Heavy chunks use opaque depth-tested geometry and
write depth, so nearer blocks can hide them and they can hide more distant
grains. Their size and radiance shrink smoothly to zero. There are no screen
overlays or broad opaque row flashes.

The shader's `Scale` and `Rotation` outputs give each chunk its own proportions
and tumble. `Lighting` is 0.85 for chunks and 0.35 for microembers. The renderer
uses the scene's direct and hemisphere lights for this response. Grains do not
sample shadow maps or screen-space occlusion. The fracture and expanding
front remain emissive. Reaction lights have a smooth 25 ms attack and 275 ms
release; their intensity is zero before arrival and at retirement. The short
light pulse is separate from the longer fragment lifetime.

## A lost board

A final top-out enters a separate loss phase. Controls and the run clock stop
immediately, while the scenery and released geometry continue to advance.
Scoring, line totals and the occupied rule grid remain intact. Duet rescue is
resolved first, and a winning final clear still takes priority over a blocked
next spawn. A single stranded Duet board waits for its partner without releasing.

After 700 ms, a release steps through the visible rows from top to bottom at
12.5 ms per row. The existing block meshes detach intact, retaining each piece's
color, material, bevel and depth. Each body follows an analytic path from its
original grid position rather than accumulating offsets each frame. Its upward
travel is `180t + 180t²` world units. A cell-dependent outward velocity and
`45t²` lateral acceleration carry bodies beyond the well, with forward depth.
Independent yaw and roll make the blocks tumble. Bodies stay full size and
retire after 1.8 seconds of flight, once outside the visible board. The board
border, HUD, Hold and Next previews stay fixed throughout departure. Hold
and Next hide when the result appears.

The result appears at 2.8 seconds. The decorative loss clock continues afterward
until four seconds so departing bodies can finish their flight; gameplay time
remains stopped. Restart is available as soon as the result appears. It resets
all tile rotations and positions and retires secondary particles through the
existing 60 ms release.

Each immutable row record carries its actual occupied-column mask and captured
front face. One field of 96 quiet microembers accompanies each occupied row;
missing cells emit nothing. Reduced effects halves this count. Two full Duet
boards use at most 40 fields and 3,840 instances, within the existing 96-field
pool. The original 400 settled tile slots provide the block bodies, with no new
entities. Pre-loss cues retire before departure starts. Loss adds no large row
flash, energy ring or reward illumination. Reduced motion and effects-off keep
cells stationary for 300 ms, then hide them, emit no drifting fields and show results at
400 ms.

Opening Play, occupied restarts and mode switches share this departure,
including the active piece. The fresh run starts at 2.8 seconds and its HUD
reveals over 800 ms. Quiet settings take 480 ms altogether. A lost board that
has already departed restarts immediately. These timings and the stronger
outward flight are original tuning for the requested choreography.
The `--loss` C/pico_unit check drives real legal drops
into top-out and checks each occupied row, original mesh colors and size,
upward travel, tumble, fixed field budget, frozen gameplay, Solo and Duet,
quiet settings, retirement and restart. It also runs the existing rescue,
completion-priority and chapter-restart checks. Authored simulation does not
establish rendered appearance or browser frame time.

## Timing and motion

For clear layers except the fracture front, birth time uses an independent stable hash `q`
in `[0, 1)`. For field life `L`, the emission window is `E = 0.26L` for chunks and `0.10L` for the longer ember and energy tails:

```
b(q) = E * (exp(log(65) * q) - 1) / 64
N(t) / N = log(1 + 64t/E) / log(65)
```

This gives a dense initial burst followed by fewer births. A fracture grain's
birth is `(abs(source.x) / 118 * 0.04 + q * 0.008) * L / 0.24`. Particle duration is
`D = L * (0.56 + 0.16h)` for fracture/chunks and `L * (0.72 + 0.18h)` for tails, using another hash component. With event delay `d` and
field age `t`, elapsed age is `a = max(0, t - d - b)`.

```
A(a) = smoothstep(0, min(0.016, 0.16D), a)
       * (1 - smoothstep(0.28D, D, a))
phase = clamp(a / D, 0, 1)
clear_size = base_size * (1 - 0.24phase) * A(a) * retirement_fade
alpha = base_opacity * A(a) * retirement_fade
```

Size and alpha are zero at birth and death. Every grain reaches zero before
its field deadline, including the latest fracture grain. Changing run or
turning motion off applies a smooth sixty-millisecond retirement envelope to
existing geometry. Reduced effects halves each layer count, giving 1,328
instances per row. Reduced motion and effects-off emit only eight stationary
confirmation grains for 140 ms, with no new lights.

For clear particles, constant gravity and linear drag use the exact analytic
solution, evaluated at the particle's age:

```
F(a, k) = (1 - exp(-k * a)) / k
G(a, k) = (a - F(a, k)) / k
position = source + initial_velocity * F + gravity * G
```

The microember curl adds three phase-offset sine/cosine differences multiplied
by `18 * (1 - exp(-1.6a))`. It starts at zero and stays bounded. This is a
prescribed flowing trajectory, not a fluid solver. Chunk tumble is constant
angular motion. Reaction fields use analytic motion; the background separately
retains GPU position, velocity and age. Neither simulates collisions,
rigid-body contacts or particle-to-particle forces.

Ordinary cues use `E = min(0.12L, 0.045)` and `D = L * (0.70 + 0.16h)`.
Contact and hard-drop births instead finish within 6 ms. Attack and release are:

```
A(a) = smoothstep(0, min(0.012, 0.12D), a)
       * (1 - smoothstep(0.12D, D, a))
```

Alpha uses this envelope and the mode-change retirement fade; size uses
`sqrt(A)` to retain coverage through the tail. The white core cools according to
`exp(-20a)`, independently of lifetime. Translation uses `F(a, 3.6)`;
contact and settling gravity use `G(a, 3.6)`, with downward acceleration of 125
and 20 units/s² respectively. Rotation uses angular displacement
`-direction * (0.28 + 0.52h) * (1 - exp(-5a))`. These are continuous authored
paths, without particle collision simulation. The hard-drop wash contracts
toward the already occupied destination with `smoothstep(0, 0.06, t - d)`.

## Bounds and validation

The largest field holds 2,048 instances, and the 96-field worst-case bound is
196,608 reaction instances. Normal events remain at their smaller counts.
When the pool is full, the next cue increments `VisualState.cue_dropped` and
keeps the existing fields unchanged. Event queue consumption continues, and
`cue_overwrites` remains zero. The field and light entity pools do not grow on clear. The renderer may allocate or resize an instance buffer when a field first activates or its count changes.

`VfxChecks` exercises the production consumer: local action origins, hard-drop
arrival and lighting, separate contact and settling lifetimes, suppression of
the following lock's duplicate effect, extended ordinary tails, full-pool
saturation, four-row budgets and independent
layer lifetimes, original row masks after compaction, captured front depth,
Solo and Duet placement, reduced effects and continuous mode-change retirement.
The C/pico_unit dispatcher runs the fixture under the production supervisor.
Shader compilation, rendered appearance and warmed browser frame times remain
separate checks from the authored simulation.
