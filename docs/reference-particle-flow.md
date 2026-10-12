# Particula and the Tetris Spheres particle film

Studied on October 11, 2026, with a further source comparison on October 12. The source comparison uses [Particula commit 9c05c66](https://github.com/Humprt/particula/tree/9c05c6664dd318eeeefd14457fa15862c72ae8c3). Visual inspection used selected passages of its [linked demonstration](https://www.youtube.com/watch?v=AroTkLpfSSA), including the opening, 1:27 and 2:53–3:21, and the [30-second film used behind Tetris Spheres](https://dcts-tetris-spheres.web.app/video/background-particles.mp4). Neither reference's code or media is copied into the game.

## What the references actually show

Particula's demonstration has a compact, densely populated amber core, quieter violet and blue outer particles, and irregular threads that form and disperse. Around 3:21 the visible mass opens into much sparser curved blue and magenta streams. Black space is a large part of the composition. The particles are fine enough to accumulate into luminous areas while their less dense edges remain granular.

The Tetris Spheres film fills the view with pale blue and warm ivory swarms against dark blue-black. Dense patches curl into one another. Their boundaries are irregular, with streams escaping into sparse regions. Near grains are larger and softer; distant grains form fine detail. These are observations of the rendered film, not measurements of its original simulation or camera.

Our previous repeated outlined blooms and broad parallel ribbons did not reproduce either treatment. Their particles changed position but continued to describe recognizable prescribed shapes. Raising their brightness or count would not fix that difference.

## The useful implementation in Particula

[Particula's simulation](https://github.com/Humprt/particula/blob/9c05c6664dd318eeeefd14457fa15862c72ae8c3/main.js#L1341-L1454) retains position, velocity, lifetime and a beat envelope for each particle. Three Perlin samples change acceleration. Damping retains momentum while confinement pulls particles back toward their volume. Birth positions fill an inner volume rather than lying on a sphere's surface. Five colocated systems can respond to separate frequency bands.

The presets change those dynamics. Filaments combines a dense short-lived core and a sparse longer-lived outer cloud. Fuzzy and Mixed use similar particle counts but very different turbulence. Cell separates a small hot core, a detailed middle volume and a dim, larger-grained halo. These are changes to density, residence time and motion, not only palette choices. [Preset definitions](https://github.com/Humprt/particula/blob/9c05c6664dd318eeeefd14457fa15862c72ae8c3/particula_presets.json)

Particula draws additive points with depth fog. It does not use a custom lighting or bloom shader. Its separately declared expanding beat-wave helper is not called by the particle update; the implemented beat reaction is radial acceleration. Several coefficients are per-frame, so copying them would also copy frame-rate dependence.

## Stillwater's translation

The main field now uses GPU-resident position, velocity and age. Ordinary GLSL defines births, acceleration, damping, confinement and appearance. A two-target floating-point update pass advances each particle once, and the instanced mesh pass reads the result. Small octahedra keep the scene and particles three-dimensional.

Four force regimes replace the prescribed geometric formations:

- Percussive sound favors broad orbital swarms with radial beat pressure and depth motion.
- Sustained tonal sound favors elongated filled volumes and coherent circulation that stretches knots into streams.
- Airy sound favors folded thin volumes with stronger fine turbulence along their ragged edges.
- Dense sound favors nested knots with strong broad and fine turbulent transport.

Transitions change future forces and birth distributions. Existing particles keep their positions and velocities. Five overlapping populations respond to bass, bass/mid, mid, mid/treble and treble. Bass and onsets push the interior outward; each population's band changes transport, and accepted sections briefly broaden the volumes and strengthen fine turbulence. Recorded gameplay impulses add acceleration at the projected action origin. No audio value directly teleports the whole cloud into a new shape. The October 12 comparison rejected an intermediate implementation whose strong per-grain springs produced exact parallel curves; confinement now acts only near each filled volume's boundary.

The flow uses rotated, multiscale divergence-free trigonometric fields. Velocity relaxation follows `1 - exp(-rate * dt)`; the position update uses the resulting velocity. This is persistent particle advection through authored force fields. It does not solve fluid pressure, particle collisions or mutual gravity. Birth and retirement envelopes fade size and alpha; pause freezes the simulation clock. Distant grains remain quiet and sparse. Most main grains are nearly isotropic, with only a small fast foreground population stretched along its velocity.

Actual visual acceptance and browser frame-time evidence are recorded in [validation](validation.md). Reference resemblance must be judged from moving output, not from the presence of these mechanisms in source.
