# Reaction effects

`Environment.chibi` consumes the captured reaction queue. `ReactionParticles.Cue`
evaluates each grain on the GPU. The geometry is an instanced octahedron with
real depth testing. There are no sprites, screen overlays or full-column drop
ribbons. The renderer uses source-alpha additive blending and keeps depth
writes off for the grains; opaque blocks and the board still occlude them.

The effect starts at four occupied cell centers in XY. Each point's Z is the
front face of the rendered block, including chapter depth and music lift at the
action. The shader adds only 0.35 world units of clearance. Hard drop captures
the original cells before lock and translates the landing origin by
`-24 * traveled_rows`. Rotation derives each cell's tangential velocity from
the four-cell center, so the small spark displacement follows the actual piece.

## Layers

All layers use procedural 3D solids; no texture files are needed. Their shader supplies the flare-like hot core, stable grain variation and soft low-opacity wake. Depth testing places them behind nearer solids, while source-alpha additive blending accumulates their light. The layers below are ordered from the readable contact cue to the surrounding energy.

| Layer | Purpose and procedural material | Motion | Envelope and blending |
| --- | --- | --- | --- |
| Contact grains | Small white-hot octahedra identify the local edge; the color cools into pale cyan, restrained violet or amber for rejection. A stable hash varies grain size and brightness. | Movement starts on the trailing cell edge. Turns start on the cell perimeter at 55–120 units/s along the signed tangent. Horizontal movement starts at 18–52 units/s opposite travel. Landing starts on the bottom edge. | 16 ms maximum smooth attack, smooth release after 28% of particle life; source-alpha additive. |
| Soft wake | About 30% of the grains are larger, dimmer solids. Their low opacity and slight displacement leave a broken haze behind the contact instead of a hard outline. | Drag limits travel. Turn wakes lag the tangent by three units, with at most four units of additional lateral spread. | Same continuous size and alpha envelope; wake alpha is 22% of a bright grain's alpha. |
| Ballistic flecks | About 22% of landing grains are fine solid fragments. They add weight without covering the board. | Initial upward velocity is 30–75 units/s; acceleration is -300 units/s². Horizontal speed is at most 52.5 units/s before drag. Depth scatter stays close to the block front. | Individually staggered births and deaths; no rigid row-wide ring. |
| Hard-drop packet | Four compact vertical groups, one per original cell, show direction and distance. Narrow octahedra use 2.4× height. | The packet moves the traveled distance in 110 ms. Its trailing extent is capped at 18 units, less than one cell. | 260 ms field life. The packet never fills the entire path simultaneously. |
| Landing compression | Bottom-edge grains spread sideways a few units, followed by the soft wake and ballistic flecks. | Begins 95 ms after a traveled hard drop, just before packet arrival. A zero-distance drop begins immediately. | 340 ms field life plus the arrival delay; the logical stack remains in place. |
| Grounded light | A small cyan light binds a hard-drop contact to the nearby board. | Located below the lowest occupied cell, two units in front of the captured front face. Radius is 94 units at the authored drop strength. | Zero at creation, 25 ms smooth attack, 275 ms smooth release, maximum intensity 0.75. No live light is repositioned. |

A rejected action uses twelve low-energy amber grains at the local side edge.
A successful horizontal move uses sixteen grains. Turns use forty-eight. Soft
fall uses twenty. Hold contracts thirty-two grains slightly toward the captured
piece center. A cleared row uses 112 short-lived grains across its original
row, while combo adds light and current impulse without another geometric arc.
Topout and rescue have eighty grains with short ballistic motion. Session
confirmations stay near the relevant in-world control or lower board edge.

Reduced effects halve counts with a floor of eight. Reduced motion and effects
off use eight stationary confirmation grains with a 140 ms field life and no
new local light. Changing those settings releases already-visible grains over
60 ms. It does not remove them on the next frame.

## Implemented timing

For each grain, an independent stable hash gives `q` in `[0, 1)`. For field life
`L`, the emission window is `E = 0.26L` and the birth delay is

```
b(q) = E * (exp(log(65) * q) - 1) / 64
N(t) / N = log(1 + 64t/E) / log(65),  0 <= t <= E
r(t) / N = 64 / (E * log(65) * (1 + 64t/E))
```

The cumulative count is logarithmic. Its derivative, the emission rate, falls
reciprocally. The initial rate is 65 times the rate at the end of the window;
about 65% of births occur in its first 22%.

Particle duration is `D = L * (0.56 + 0.16h)`, where `h` is another hash
component. With event delay `d` and field age `t`, the grain's elapsed age is
`a = max(0, t - d - b)`. Let `S(a,b,x)` mean GLSL `smoothstep(a,b,x)`.

```
A(a) = S(0, min(0.016, 0.16D), a) * (1 - S(0.28D, D, a))
phase = clamp(a / D, 0, 1)
size = grain_size * (1 - 0.3phase) * A(a) * retirement_fade
alpha = grain_opacity * A(a) * retirement_fade
```

Both size and alpha are zero at birth and death. Color cools smoothly with
`S(0, 0.55, phase)`. Since `b + D < 0.98L`, every particle is invisible before
the field is retired at `d + L`. The remaining margin prevents the pool from
cutting off a late grain. A mode change multiplies size and alpha by a smooth
60 ms retirement envelope.

Drag displacement is `v * a / (1 + 7a)`. Ballistic vertical displacement is
`vy * a - 150a²`. Hard-drop displacement is
`-24 * rows * S(0, 0.11, t - d)`. Quiet confirmations multiply motion by zero.

## Bounds and validation

The pool contains 96 fields and eight lights. No grain creates an entity or
runs CPU behavior. Each field holds at most 112 instances. Full pools skip an
incoming cue and increment `VisualState.cue_dropped`; an existing visible field
is never overwritten. Event sequence accounting continues normally.
`cue_overwrites` remains zero. Light reuse also waits for the full envelope.

`VfxChecks` exercises the production consumer with captured points, hard-drop
arrival timing, light attack and release, cue expiration, saturation, and mode
change retirement. Shader compilation and a rendered view are separate checks;
CPU fixture success does not establish visual quality or browser performance.
