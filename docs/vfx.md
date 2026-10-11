# Reaction effects

`Environment.chibi` consumes immutable reaction records. `ReactionParticles.Cue`
evaluates ordinary contact grains; `ReactionParticles.Release` evaluates clear
layers. Both evaluate solid octahedra on the GPU from their index, seed and age. The fixed
pool holds 96 fields and eight local lights. No grain creates an entity or runs
a behavior, and a visible field or light is never moved to a newer event.

Ordinary input starts on the four captured occupied cell fronts. Their depth
includes the block finish, chapter depth and music lift at the action. Movement
uses sixteen grains, turns forty-eight, soft fall twenty, hold thirty-two and
rejection twelve. These remain brief local cues. A traveled hard drop uses 112
descending grains, followed after 95 ms by 56 landing grains and a local light.
The compact packet covers at most eighteen units of its path at a time.

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
| Heavy fragments | 96 | 35 / 950 ms | Opaque chunks, 2.2 to 5.8 units before the envelope, with independent uneven proportions and tumbling. Initial X speed reaches 220 units/s, Y starts at 50 to 150 units/s, and Z starts at 20 to 65 units/s. Gravity is -250 units/s² and drag is 1.8/s. |
| Microembers | 2,048 | 55 / 1,400 ms | Fine solids, 0.45 to 1.52 units before the envelope. They disperse through depth with 3.1/s drag, -18 units/s² gravity and bounded curl drift. Dimmer larger grains give the trailing dust a broken texture. |
| Expanding energy | 192 | 20 / 600 ms | A sparse elliptical front begins at the row perimeter. Its horizontal and vertical radii grow by `350F` and `180F`; depth advances by `60F`, using 4.5/s drag. It leaves the center open. |
| Row illumination | One light | 20 / 300 ms | Centered on the actual row, two units ahead of its captured front. Strength ranges from 0.89 for one row to 1.1 for four. The existing light pool skips a new light if all eight are still alive. |

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

Except for the fracture front, birth time uses an independent stable hash `q`
in `[0, 1)`. For field life `L`, the emission window is `E = 0.26L`:

```
b(q) = E * (exp(log(65) * q) - 1) / 64
N(t) / N = log(1 + 64t/E) / log(65)
```

This gives a dense initial burst followed by fewer births. A fracture grain's
birth is `(abs(source.x) / 118 * 0.04 + q * 0.008) * L / 0.24`. Particle duration is
`D = L * (0.56 + 0.16h)`, using another hash component. With event delay `d` and
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
by `9 * (1 - exp(-3a))`. It starts at zero and stays bounded. This is a
prescribed flowing trajectory, not a fluid solver. Chunk tumble is constant
angular motion. The GPU does not persist particle state or simulate collisions,
rigid-body contacts or particle-to-particle forces.

Ordinary contact drag also uses `F(a, 7)`. Its small landing flecks retain their
ballistic vertical formula `vy * a - 150a²`. The hard-drop packet follows
`-24 * rows * smoothstep(0, 0.11, t - d)`.

## Bounds and validation

The largest field holds 2,048 instances, and the 96-field worst-case bound is
196,608 reaction instances. Normal events remain at their smaller counts.
When the pool is full, the next cue increments `VisualState.cue_dropped` and
keeps the existing fields unchanged. Event queue consumption continues, and
`cue_overwrites` remains zero. The field and light entity pools do not grow on clear. The renderer may allocate or resize an instance buffer when a field first activates or its count changes.

`VfxChecks` exercises the production consumer: local action origins, hard-drop
arrival and lighting, full-pool saturation, four-row budgets and independent
layer lifetimes, original row masks after compaction, captured front depth,
Solo and Duet placement, reduced effects and continuous mode-change retirement.
The C/pico_unit dispatcher runs the fixture under the production supervisor.
Shader compilation, rendered appearance and warmed browser frame times remain
separate checks from the authored simulation.
