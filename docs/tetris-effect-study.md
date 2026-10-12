# Tetris Effect reference study

Reviewed the supplied [recording](https://www.youtube.com/watch?v=urbLIyd-VsQ)
only before **9:00**. The first pass retained nine sheets containing 180 frames
at three-second intervals, a scene overview and a denser clear sequence. The
second pass revisited all nine sheets, sampled 0:20–1:05 into 90 frames, and
inspected 29.5–32.4 seconds at ten frames per second with the board cropped at
native resolution. All selected frames were inspected. The retained sheets
live in ignored `.chibi/reference-polish` and `.chibi/trance-pass`; reference
media is not shipped with the game.

There is no input overlay. The images establish visible action sequences,
not exact button latency or the original simulation. Overview labels are
approximate. No footage after nine minutes informed this pass.

The third comparison revisited these same sheets against consecutive browser
captures. It found three remaining visible gaps: the perspective well was
about 510 pixels high at 720p instead of the reference's roughly 330; a drop
lit only touching cells rather than sending a highlight through the pile;
and first-use particle slots repeated the same event seed. Gameplay now uses
a 0.88-radian lens, producing a roughly 340-pixel well at that display size.
The menu retains its larger demonstration. All seven pieces share one cool
slate/cyan treatment, including Hold, Next and departing blocks.

Settled cells and existing background particles receive traveling drop
illumination. The captured comparison also rejected an excessively bright,
continuous background annulus: the reference's asymmetric clouds, coarse
foreground highlights and black gaps remain the visual target. Ordinary
action clouds often cool within one or two seconds; the strongest long tails
follow line clears. Each event now has its own coherent variation in spread,
density, curl, size and cooling instead of repeating a pool slot's pattern.

## Action details

| Action | Visible evidence | Direction for Stillwater |
| --- | --- | --- |
| Move | The active cells change column immediately. Fine local specks and highlights are much smaller than the surrounding plumes. | Keep input immediate; leave a subtle trailing edge wake and bend existing particles near the action. |
| Rotate | Around 30.0–30.4 and 31.8–32.4, the silhouette changes orientation while a cyan plume curls beyond the board edge. Older grains continue moving through the new response. | Tangential motion from the captured pivot, followed by a longer drifting tail; successive forces influence already living grains. |
| Hard drop | Blue columns remain visible across several samples at 29.5–29.9 and 30.6–30.8. The beam narrows and cools while particles spread through the surrounding space. | A sustained narrow beam, immediate placement and landing light, then a separate deep plume. |
| Landing | The newly occupied area and neighboring faces brighten. At 30.6–31.1 the highlight cools across the pile. The well yields slightly and returns. | Local point lights ahead of the face, a damped downward response, and soft settling dust. |
| Line clear | Around 52–53 and 56–58 seconds, a bright row opens into a wide fan. Large foreground highlights coexist with fine distant dust and remain behind subsequent inputs. | Separate coherent row, fragments and persistent grains; distribute motion both toward and away from the camera. |
| Ghost | The close-ups show a continuous outside contour around the whole tetromino, without cell seams. | Mask shared cell edges and keep a fine luminous outline around the full landing silhouette. |

The large cyan plumes near 24–32 and 47–50 seconds have an offset, coiling
shape. They are not small rings centered on the active piece. Brightness,
size and apparent depth vary through each cloud. At 49 seconds, large nearby
grains overlap smaller background grains. The later clear fan remains visible
while fresh cyan plumes form, so replacing one short effect with the next
would lose a major part of the motion.

Quiet space is equally important. Much of the early scene is black between
actions. Persistent scenery can be dense later in a phrase, but its resting
brightness must leave room for player and music accents.

## Scenes and passages

About 2:00–2:55, broad blue streams and a planetary rim surround the well.
The passage near 2:58–3:11 removes the board, moves through particles and
restores the continuing pile. From roughly 3:10, faint concentric geometry
and symbols sit behind a large triangle that appears as an accent. Amber
arrives near 5:40, followed by teal near 7:30. Across these stages, drop beams
remain legible even when most background geometry is dim.

Stillwater uses original cyan streams, ivory orbits, amber geometry and teal
currents. Its shorter Journey passage preserves the pile and score. The
shared new-run/loss block departure and return to the main menu are requested
adaptations; this bounded study does not establish a reference loss sequence.

## What the recording cannot establish

The apparent particle interaction does not identify a particular fluid solver
or collision model. Stillwater uses persistent GPU positions and velocities
with shared curl and action forces, not pairwise particle collisions. Similarly,
bloom is visible in the recording but does not prove that each grain emits a
scene light. Stillwater uses emissive grains and a bounded pool of actual local
lights for landing and clear events.

No claim about the recording's beat detection follows from silent frame
sampling. Stillwater's music response was checked separately by replaying its
actual bundled PCM through the production analyzer and authored response.
See [audio](audio.md#feedback), [effects](vfx.md) and
[validation](validation.md) for implementation and executed checks.
