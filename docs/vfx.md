# Reaction effects

`Environment.chibi` consumes immutable reaction records. `ReactionParticles.Cue`
evaluates ordinary contact grains; `ReactionParticles.Release` evaluates clear
layers. Both evaluate solid octahedra on the GPU from their index, seed and age. The fixed
pool holds 96 fields and eight local lights. No grain creates an entity or runs
a behavior, and a visible field or light is never moved to a newer event.

Ordinary input starts on the four captured occupied cell fronts. Their depth
includes the block finish, chapter depth and music lift at the action. The
effects stay local, with a restrained initial color and longer dim tails.
Grains have small variations in proportion and tumble, and receive some scene
lighting. A turn follows an arc around the captured piece center.

| Action or layer | Instances | Field life | Placement and timing |
| --- | ---: | ---: | --- |
| Movement | 32 | 520 ms | Leaves the trailing edges and slows through drag. |
| Rotation | 72 | 760 ms | Curves around the occupied shape, then settles. |
| Soft fall | 24 | 480 ms | Starts beneath the captured cells; release uses 380 ms. |
| Hold | 48 | 720 ms | Draws slightly inward and forward through depth. |
| Rejection | 12 | 400 ms | Stays beside the blocked edge. |
| Hard-drop packet | 96 | 420 ms | Reaches the landing in 160 ms; its compact wake contracts by 320 ms. |
| Contact flecks | 64 | 640 ms | Starts at arrival, on exposed lower cell edges. |
| Settling dust | 96 | 1,000 ms | Starts 45 ms after contact and drifts gently through depth. |

A hard drop uses the packet and both landing layers. Its immediately
following lock record does not replay them. A natural lock uses the two landing
layers without a packet. Internal lower edges are masked, so stacked cells do
not emit contact grains through the middle of their shape. The packet's wake
spans at most twelve units along its path. Its light waits for the full 160 ms
arrival and has a peak setting of 0.44; a natural lock uses 0.324. Their shared
light envelope stays below those settings.

## A cleared row

Every original row in the recorded mask gets its own four-layer release. The
field origin is the board center in X, `276 - 24 * row` in Y, and the average
captured block front in Z. It never reads the new piece position or compacted
grid. Particle seeds place grains across all ten original block faces, with
eighteen units of scatter inside each cell. Four rows create 10,624 instances
across sixteen fields. The prior release used 112 instances per row.

| Layer | Instances per row | Delay / field life | Geometry and motion |
| --- | ---: | --- | --- |
| Fracture front | 320 | 0 / 240 ms | Thin bright splinters, 2.2 by 0.38 by 0.6 proportions. A forty-millisecond front sweeps from the row center toward its ends. The short white-cyan core cools into cyan or occasional amber. |
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
than holding a bright glow for six seconds. The opening demo's release survives
the first run handoff, while ordinary restarts still retire all old fields.

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
angular motion. The GPU does not persist particle state or simulate collisions,
rigid-body contacts or particle-to-particle forces.

Ordinary cues use `E = 0.20L` and `D = L * (0.62 + 0.14h)`. Their slower attack
and release are:

```
A(a) = smoothstep(0, min(0.050, 0.22D), a)
       * (1 - smoothstep(0.18D, D, a))
```

Alpha uses this envelope and the mode-change retirement fade; size uses
`A * (2 - A)` to retain readable coverage through the softer tail. Contact
fragments are 30% larger than the ordinary grains; settling dust remains finer.
The latest grain reaches zero before 96% of its field life. Ordinary translation
uses `F(a, 4.2)`. Contact and settling gravity use `G(a, 4.2)`, with downward
acceleration of 50 and 18 units/s² respectively. Settling dust adds bounded
sine differences that start at zero. Rotation uses an angular displacement of
`-direction * (0.35 + 0.25h) * (1 - exp(-4.2a))`. These are continuous authored
paths, without particle collision simulation. The hard-drop packet follows
`-24 * rows * smoothstep(0, 0.16, t - d)`.

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
