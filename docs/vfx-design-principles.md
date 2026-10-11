# VFX design principles

Research reviewed 2026-10-11. This is a foundation for a future design skill,
not a skill definition or a record of implemented changes. The immediate goal
is to give rotation, landing and hard drop the craft of Stillwater's existing
line clear, with lower intensity, smoother movement and quieter, longer tails.
Current authored behavior is documented in [Reaction effects](vfx.md).

The sources below describe production practice and technical constraints.
They do not establish universal psychological laws. The design principles are
heuristics to test in context; the Stillwater recommendations are our synthesis.
Accessibility guidance is identified separately from visual preference.

## Principles and their reasoning

### 1. Match attention to the importance of the event

Riot's clarity guidance ranks effects by gameplay importance and limits
accumulated visual noise. This gives players a useful distinction between an
ordinary action and an event that changes their next decision.
[Riot, Clarity in League](https://www.leagueoflegends.com/en-us/news/dev/clarity-in-league/).

For Stillwater, keep the piece, ghost, stack and essential HUD readable through
every effect. Within reaction effects, rotation should be quieter than a hard
drop, and a clear should remain the reward peak. Ambient music motion sits
below all of them. Share materials and careful motion across actions without
giving them equal brightness, screen coverage or sound emphasis.

### 2. Give the effect one primary visual message

Riot's VFX education stresses accurate action feedback, restraint and a
consistent style in which each effect still belongs to its source.
[Riot, Visual Effects](https://www.riotgames.com/en/artedu/visual-effects).

Our working rule is to identify the action with one shape or movement before
adding detail. A turn gets an arc; a drop gets downward arrival; a landing gets
contact and settling. Secondary grains add texture and follow-through. Local
illumination can reinforce contact, but should not become another competing
flash. If the action is unclear with the decorative layer hidden, adding more
particles is unlikely to fix it.

### 3. Separate response time from decorative lifetime

Michael Lyndon's GDC session applies animation principles to VFX and emphasizes
the reasoning behind parameter choices.
[GDC 2018, Animation Principles of VFX](https://www.gdcvault.com/play/1025301/Visual-Effects-Bootcamp-Zip-Thwack).

Our application is an immediate, compact confirmation followed by a slower
release. Keep gameplay response immediate. Let brightness fall before the last
grains disappear, and let motion decelerate into a dim tail. This extends the
feeling of motion without holding attention at peak intensity. Anticipation
belongs to an action that actually has a wind-up; inserting it before an
already accepted rotation would misrepresent the action.

### 4. Shape motion and brightness with separate curves

Beyond-FX demonstrates how contrast in spacing and timing changes the read of
an effect. Its stepped-animation example is a stylistic choice, not a universal
frame-rate prescription.
[Beyond-FX, Timing and Spacing Tricks for Textures](https://blog.beyond-fx.com/articles/vfx-quick-tip-timing-and-spacing-tricks-for-textures).

For Stillwater's smoother treatment, tune travel, scale, opacity and light
independently. Use a small initial displacement, controlled deceleration and a
longer low-opacity finish. Stagger secondary births, while keeping the primary
confirmation prompt. Keep particle identity stable and trajectories continuous;
frame-to-frame random positions read as flicker rather than drift. Avoid a
single envelope that makes every layer expand, brighten and vanish together.

### 5. Preserve contrast and negative space in the actual scene

Riot's VALORANT renderer uses controlled shading for readability and preserves
gameplay information when graphics quality changes.
[Riot, VALORANT Shaders and Gameplay Clarity](https://www.riotgames.com/en/news/valorant-shaders-and-gameplay-clarity).

Our test is whether occupied cells, the ghost and text remain distinguishable
at the effect's brightest moment. Judge the result after bloom and lighting,
against both dark and bright backgrounds. Leave openings in expanding forms.
Use hue to support identity, with shape, direction and value doing enough work
to survive a grayscale inspection. Reduced particle counts must preserve the
same action message; compensate only as needed, without creating a new bright
flash.

### 6. Make attachment and depth agree with the event

Unity distinguishes particles that follow a parent, remain in world space or
follow another object. That choice determines what motion means.
[Unity, Particle System main module](https://docs.unity3d.com/6000.0/Documentation/Manual/PartSysMainModule.html).
Depth testing lets scene geometry correctly hide particles; intersection fades
are a separate technique for soft particles, not a requirement for solid debris.
[NVIDIA, GPU Gems 3 chapter 23](https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-23-high-speed-screen-particles).

For Stillwater, preserve the captured occupied cells and visible front depth.
Released landing grains should stay at the original contact after the next
piece spawns. A turn cue can describe rotation around that piece's captured
pivot. Do not move an old effect to a new event or pull particles through the
board to make them visible. Correct placement and occlusion make the cue belong
to the action.

### 7. Give audio the same hierarchy

Riot describes sound as feedback about events and player decisions.
[Riot, Sound Design](https://www.riotgames.com/en/artedu/sound-design).

Our recommendation is to align the sound's main transient with the visual
confirmation or contact. Use quiet, short cues for frequent actions and reserve
greater emphasis for a clear. Avoid making every lingering grain require a
lingering sound. Listen to repeated actions over the music, and inspect the
visuals muted. Essential feedback should remain understandable in either
condition.

### 8. Treat accessibility as part of the effect contract

Xbox guidance recommends multiple channels for essential information and warns
against relying on color alone. Distinct shapes and motion help, while sound or
haptics supplement the visible state.
[XAG 103](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/103).

Provide control over decorative motion and camera effects. In Stillwater,
reduced motion should retain stationary action confirmation and readable text;
do not replace removed movement with stronger flashing.
[XAG 117](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/117).

Evaluate photosensitivity on the composed output, including simultaneous
effects, bloom and audio-driven modulation. Flash frequency alone does not
establish safety; contrast, area, color and repeating patterns also matter.
Use appropriate testing, and fix triggers rather than relying on a warning.
These are accessibility recommendations, not a certification claim.
[XAG 118](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/118).

### 9. Budget for repeated events, not an isolated effect

Epic recommends profiling representative gameplay because individually cheap
effects can become expensive when many coexist. Overdraw depends on overlap
and screen area as well as particle count; count scaling also changes apparent
brightness.
[Epic, Measuring Performance in Niagara](https://dev.epicgames.com/documentation/unreal-engine/measuring-performance-in-niagara?lang=en-US).

Longer tails increase concurrency. As a planning approximation, average active
effects are event rate multiplied by average lifetime, assuming a steady rate
and no drops. This does not predict GPU time. Measure rapid rotations, repeated
drops and simultaneous Duet clears at the normal resolution. Reduce decorative
tails before losing essential confirmation. Keep existing visible effects
stable when a pool fills, and inspect saturation as well as frame-time spikes.

## Proposed Stillwater action mapping

These are design starting points, not measured perceptual thresholds or changes
already applied. The duration ranges describe decorative lifetime, not input
latency or the length of a bright pulse.

| Action | Relative emphasis | Primary message | Quieter follow-through |
| --- | --- | --- | --- |
| Rotate | Low | Short tangential arcs at the captured piece cells, matching turn direction | Sparse grains slow into the arc and fade over roughly 0.35–0.65 seconds. Keep broad illumination absent. |
| Land | Low to medium | Small contact response at the actual landing cells | A few weighted grains settle over roughly 0.45–0.8 seconds. Favor localized motion over a radial explosion. |
| Hard drop | Medium | Compact directional arrival, followed by contact | A restrained outward drift lasts roughly 0.65–1 second. Keep the arrival light shorter than the debris tail. |
| Line clear | Highest reaction reward | Preserve the liked fracture and row release | Use its existing layered chunks and microembers as the craft reference, including the separate short light and longer particle lives. |

When hard drop and landing represent one contact, design one combined payoff.
When a clear follows, let that clear take emphasis while the contact settles.
Do not stack independent full-strength rewards for each internal event.

## Review before adoption

Inspect normal-speed play first, then slow motion for births, attachment and
retirement. Compare actions together against the existing clear. Check rapid
repetition, overlapping events, Solo and Duet, narrow viewports, bright music
moments, muted audio and every effects setting. Review grayscale readability
and the next piece while the previous effect is still alive. Measure warmed
frame times separately from visual review.

A future skill can turn each principle into a card containing the decision it
addresses, the source, the reasoning, adjustable controls, a failure example
and a review test. Keep the constraints separate from style choices. This note
uses first-party written material and the GDC session description; it does not
claim a full viewing of the linked talks or experimental validation of the
proposed Stillwater timings.
