# Stillwater audio

The score and effects were composed and synthesized for Stillwater on October 10, 2026. All sound comes from `tools/compose.c`: additive and FM voices, filtered seeded noise, and a stereo reverb. There are no recordings, sample packs, borrowed melodies, or generated-audio service outputs.

The composition data, sound design, and rendered audio in `content/sounds/` are released under CC0-1.0. The composer source uses the repository's source-code license. Xiph's libVorbis and libOgg are local encoding dependencies, not audio source material. They are not bundled with the game.

## Score

Every track is 60 seconds, 24 bars of 4/4 at 96 BPM. All share the same D minor / F major pitch family, tempo, and eight-bar harmonic timeline:

| Bars | Harmony | Bass |
| --- | --- | --- |
| 1–2 | D minor 9 | D |
| 3–4 | B-flat major 9 | B-flat |
| 5–6 | F major 9 / A | A |
| 7 | C suspended 4, add 9 | C |
| 8 | C add 9 | C |

The score repeats that progression three times with changes in melody density, register, accompaniment, and percussion. The main melody rises from A through C and D, reaches F over B-flat, opens to G over F, then returns through D and C. The final phrase leaves room for the next pass.

| File | Arrangement |
| --- | --- |
| `shore.ogg` | Felt keys, slow bass, restrained glass answers, and softly detuned pads. Percussion enters after the first eight bars. |
| `current.ogg` | Plucked eighth-note figures, glass melody, moving bass, low kicks, brushes, and shakers. The middle pass adds quieter sixteenth-note motion. |
| `aurora.ogg` | Wider pads with an upper octave, bright answering notes, and the melody an octave higher in the middle pass. The final pass adds lower felt-key doubles. |

Notes and their releases wrap around the render buffer. The reverb processes five seconds of the preceding cycle before writing the beginning. There is no fade to silence at the loop point. Each Ogg has `LOOPSTART=0` and `LOOPLENGTH=2646000`; an engine that loops the entire decoded file also gets the intended length.

The game uses the SDK's two-second crossfade when changing arrangements. Incoming arrangements begin at zero. This transition is not synchronized to the outgoing track's bar or chord position. The loops themselves retain their full 60-second cycle.

## Music reaction and capture

Each launch offers two in-world pointer actions beside a live demonstration: **Play** starts Journey; **Connect audio** opens the browser's sharing chooser directly. The menu has no hotkeys. Play keeps a connected external source or chooses the bundled score when no external source is ready. There are no provider launch buttons or account credentials in the game.

Play music in a separate browser tab, click **Connect audio**, select that tab in the sharing picker and enable its audio option. A desktop Chromium browser is the practical starting point for tab audio. Window or system audio is also usable when the browser and operating system offer it. Source selection always needs a user gesture and permission; the game cannot select a tab on your behalf. [Screen Capture API](https://developer.mozilla.org/en-US/docs/Web/API/MediaDevices/getDisplayMedia).

A connected stream remains on the opening screen until you click Play, including during a quiet passage. The HUD distinguishes connection from measured sound: **Audio connected** or **Connected · quiet**. An unavailable, cancelled or ended capture retains **Play** and **Connect audio** for fallback or retry. Clicking Play when no stream is ready explicitly chooses the bundled score; capture failure does not silently change the source. Reconnecting during an existing run preserves the board and returns to its previous play or pause state after explicit Play.

During play, pause or a completed run, M reconnects while preserving the board, elapsed time and return screen. A paused run remains paused after connection. The source measures actual samples; level, frequency bands and transients drive the scene and cosmetic piece motion. Logical falling, input and lock timing stay predictable. External music stays audible in its own player; the game does not play a duplicate, save or upload the capture. Browser sharing requires a video track, but only audio is consumed.

The game does not sign into YouTube, Spotify or TIDAL. A service's player can supply shared audio when the browser, operating system and playback protection permit it. Account sign-in is not proof of captured audio. The same path can use a local audio file playing in a browser tab. Scene changes follow measured timbre, rhythm and sustained spectral changes; they do not read track names or provider metadata.

Native capture uses a loopback recording device. Set `CHIBI_AUDIO_INPUT` to its exact device name before launching. Without that setting, the engine selects the first recognized loopback device: `Background Music`, or a name containing `BlackHole`, `Soundflower`, `Stereo Mix`, `CABLE Output` or `Monitor of`. It does not choose the default microphone automatically.

For example, with Background Music installed on macOS:

```sh
CHIBI_AUDIO_INPUT="Background Music" dist/Stillwater.app/Contents/MacOS/Stillwater
```

Route the external music player's audio through that virtual device, then click **Connect audio**. Selecting a device alone does not route audio into it. Other loopback devices need their corresponding system or player routing. Configure monitoring or a multi-output route if the virtual device otherwise makes your speakers silent; Stillwater does not monitor captured audio. On Linux, PulseAudio monitor inputs also require `SDL_AUDIO_INCLUDE_MONITORS=1` before launch. An unavailable device leaves **Connect audio** and **Play** available.

Stillwater does not implement Spotify sign-in or the Web Playback SDK. Spotify's [Developer Policy](https://developer.spotify.com/policy) excludes games and synchronization of sound recordings with visual media. Its [Web Playback SDK](https://developer.spotify.com/documentation/web-playback-sdk/reference) repeats the synchronization restriction and requires Premium for playback. Spotify also removed new-app access to [Audio Features and Audio Analysis](https://developer.spotify.com/blog/2024-11-27-changes-to-the-web-api), so those endpoints are not a beat-data source for this game. This is an engineering choice based on the published platform rules, not a claim that every use of user-selected audio capture is permitted by every music service.

Spotify's newer [Soloist client](https://developer.spotify.com/documentation/soloist) provides playback and local controls on Linux hardware. It requires each user to obtain an API key and install a periodically renewed native client. Its [WebSocket interface](https://developer.spotify.com/documentation/soloist/features) has no browser-facing security controls and is not a PCM input for this browser game. It is not part of Stillwater.

These sources were checked on October 11, 2026. Build and platform verification, including unverified external routing, are recorded in [validation](validation.md).

## Feedback

Native and browser audio use the same PCM analyzer. `get_audio_analysis()`
retains its level, three frequency bands, onset and source-status outputs.
The authored response normalizes quiet music and detects rises in every band.
Its single attack/release envelope preserves transients; the environment eases
the visible motion and light.

The engine now includes the MIT-licensed
[Beat-and-Tempo-Tracking](https://github.com/michaelkrzyzaniak/Beat-and-Tempo-Tracking/tree/c039090f1af771092d95c3ffc402e557940f7384)
C core. The additive `get_audio_rhythm()` function supplies estimated BPM,
phase in [0,1), rhythmic confidence and a beat envelope with an 80 ms release.
It needs several seconds to acquire a rhythm. Confidence is accumulated
evidence, not a calibrated probability. The game keeps band attacks responsive
during acquisition, strengthens tracked beats, and gradually aligns the shared
particle field with valid measured phase. Gameplay timing is unchanged.

The import fixes callback sample positions, startup timestamp underflow,
incomplete resets and an FFT autocorrelation packing error. A source change,
format change or missing queued PCM resets history. The host gates published
rhythm with audible level and recent spectral onsets, because the tracker alone
continues predicting beats through silence. Analysis consumes queued PCM outside
the audio callback; processing and resets allocate no memory.

Full 60-second replays of the decoded score at master/music gain 0.80 × 0.65,
sampled by the visual consumer at 60 Hz, produced these results. A valid frame
has raw tracking confidence of at least 0.08, the game's phase threshold.

| Track | Valid frames | Valid estimates within 3% of 96 BPM | Tracked pulses | Beat-grid RMS error |
| --- | ---: | ---: | ---: | ---: |
| Shore | 3,247 / 3,600 | 90.3% | 84 | 76 ms |
| Current | 3,385 / 3,600 | 100% | 90 | 90 ms |
| Aurora | 3,310 / 3,600 | 89.2% | 83 | 142 ms |

All three returned zero tempo, phase, confidence and beat after 100 ms of
silence. Native processing took 1.30–1.35 seconds per minute of PCM in the
bounded replay. These are analyzer measurements, not browser frame-time or
human listening results. Phase is approximate, particularly in Aurora, and
tempo can briefly follow another subdivision. It therefore guides cosmetic
motion through a smooth correction rather than controlling the falling piece.
Tracking retains a fixed sample-history size; unusual capture rates can narrow
its supported tempo range.

The earlier authored-envelope fix increased measured attack pulses from 31 to
61 in Shore, 42 to 59 in Current, and 48 to 70 in Aurora without changing average
energy. Those attacks and the tracked pulses above are different signals; the
final response combines them instead of treating either count as all 96 score
beats. This preserves off-beat percussion and bright notes alongside the beat
clock.

[KissFFT](https://github.com/mborgerding/kissfft) is not bundled. BTT already
contains its FFT, and the measured processing cost does not justify a second
FFT implementation. The retained BTT license and local patch record ship in the
SDK and exported player's license directory.

### Sound character and section changes

`get_audio_character()` reads the existing BTT spectrum before the tracker
compresses or normalizes it. It exposes spectral centroid in Hz, power flatness,
positive spectral flux, power below 250 Hz, stereo width and crest. The last
five values are normalized to 0–1. These describe brightness, tonal versus noisy
sound, change, bass weight, spatial spread and transients. A complete analysis
window precedes the first result; silence and PCM discontinuities clear stale
measurements. There is no second FFT or additional library.

`MusicCharacter.chibi` combines these measurements with the existing energy
and beat confidence. It retains independent percussive, sustained/tonal and
airy/noisy traits, plus density and rhythmic regularity. A 1.25-second response
preserves the passage's character across individual beats. Initial identity
needs 2.5 seconds of audible input. Later sections compare those features with
a slower 12.5-second baseline, require two seconds of sustained difference and
allow at least eight seconds between accepted changes. Silence retains the
established identity rather than selecting a new scene.

Full 60-second production-score traces were replayed through the actual
`MusicResponse` and `MusicCharacter` libraries at 60 Hz. Shore's arrangement
entry at 20 seconds is accepted at 22.983 seconds, retaining the tonal family.
Current and Aurora remain tonal with continuous trait variation; their subtler
20/40-second overlays do not produce discrete section events. This detector
does not identify every composed section. The compact regression for a modest,
sustained tonal change fails with the previous averaged timbre distance and
passes with accumulated percussive/tonal/airy distance.

The traits continuously reshape the field. A clear winning score can also
select broad orbital swarms, elongated tonal streams, folded airy sheets or
dense turbulent knots. Family changes ease over ten seconds along the shortest
cyclic path. Fixed scene timers and chapter-driven family changes are removed.
New block-action trails capture the current traits at birth, changing their
spread, curl and form while older trails finish continuously. Blocks keep a
shared color and ease their surface finish and musical depth lift with the same
traits. Pause and reduced motion retain the last rendered shape.

These are sound-driven choices, not genre, instrument or chorus labels. The
library review considered [LibXtract](https://github.com/jamiebullock/LibXtract)
for its C99 spectral descriptors and current zlib license. Its broader feature
set is unnecessary for these six bounded measurements. KissFFT would duplicate
the transform already in BTT. Neither is bundled.

The October 12 review also followed the
[audio-visualization list](https://github.com/willianjusten/awesome-audio-visualization)
and [music-visualizer topic](https://github.com/topics/music-visualizer).
[Clubber](https://github.com/wizgrav/clubber#instructions) provides useful
note-band distributions and independent envelopes;
[Meyda](https://meyda.js.org/audio-features) offers chroma and perceptual spread.
Those could distinguish harmonic passages at similar energy, but are not part
of this pass. Their JavaScript wrappers are unnecessary for the shared native
and Wasm analyzer. [Muser's tagger](https://github.com/jonshamir/muser/blob/master/tools/tagger.py)
runs musicnn offline and supplies saved song tags; it is not live browser genre
recognition.

[projectM](https://github.com/projectM-visualizer/projectm/blob/master/docs/building.rst)
and [Butterchurn](https://github.com/jberg/butterchurn/blob/master/src/audio/audioLevels.js)
are complete visualization renderers whose immediate and smoothed three-band
controls overlap the existing pipeline. [aubio](https://github.com/aubio/aubio#readme),
used by LedFx, adds C pitch, MFCC and transient/steady-state analysis and has an
[Emscripten build](https://github.com/aubio/aubio/blob/master/scripts/build_emscripten),
but duplicates onset/tempo work and uses GPL-3.0-or-later. It does not supply a
song-section or instrument classifier by itself. No code from these projects
was imported. BTT remains the analysis dependency; the new descriptors reduce
its existing spectrum.

Effects use the same felt, glass, pluck, bass, and air voices as the score. The move and rotate sounds are quiet pitched touches. Lock has a wooden body; hard drop adds a low impact. Clears widen from a small chord to a rising D minor 7 arpeggio. Hold descends, rescue rises after an intake of air, completion settles on F major 6, and top-out falls gently to D.

The table records suggested source gains. The shipped game uses lower per-event gains in `Board.chibi` and `Shell.chibi`, with master, music and effect defaults of 0.80, 0.65 and 0.75. Movement feedback follows successful movement. These file measurements do not measure the mixed runtime output.

| Event | File | Duration | Starting gain | Source peak dBFS |
| --- | --- | ---: | ---: | ---: |
| Move | `move.wav` | 0.18 s | 0.40 | -15.69 |
| Rotate | `rotate.wav` | 0.28 s | 0.45 | -13.82 |
| Lock | `lock.wav` | 0.55 s | 0.55 | -13.13 |
| Hard drop | `hard_drop.wav` | 0.90 s | 0.65 | -7.78 |
| Single clear | `clear1.wav` | 1.25 s | 0.65 | -7.44 |
| Double or triple clear | `clear2.wav` | 1.65 s | 0.70 | -6.96 |
| Four-line clear | `clear4.wav` | 2.40 s | 0.80 | -5.71 |
| Hold | `hold.wav` | 0.70 s | 0.50 | -10.13 |
| Rescue | `rescue.wav` | 1.80 s | 0.75 | -7.72 |
| Completion | `complete.wav` | 3.60 s | 0.80 | -5.04 |
| Top-out | `topout.wav` | 2.50 s | 0.65 | -7.19 |
| Menu confirmation | `menu.wav` | 0.65 s | 0.55 | -10.57 |

Effects are standard RIFF/WAVE PCM format 1, 16-bit little-endian stereo at 44,100 Hz. They have short onset ramps and a 120 ms final fade before triangular dither. The music uses Ogg Vorbis at quality 6, also stereo at 44,100 Hz. The shipped audio totals 5,954,740 bytes, about 5.68 MiB.

## Reproduce

The offline tool needs a C compiler, `pkg-config`, libVorbis, and libOgg. This render used Apple Clang, libVorbis 1.3.7, and libOgg 1.3.6. Build on macOS from the game root:

```sh
mkdir -p build/audio content/sounds
clang -std=c11 -O2 -Wall -Wextra -Werror -Wl,-dead_strip \
  $(pkg-config --cflags vorbisenc vorbis ogg) tools/compose.c \
  $(pkg-config --libs vorbisenc vorbis ogg) -lm -o build/audio/compose
build/audio/compose --all content/sounds
build/audio/compose --encode content/sounds
```

On Linux, replace `-Wl,-dead_strip` with `-Wl,--gc-sections`. `--music` and `--effects` render their respective subsets. `--encode` reads the three WAV masters produced by this tool and writes the Ogg tracks. The pseudo-random seeds and Ogg serial numbers are fixed. Floating-point library or encoder-version changes can change the exact bytes.

The installed FFmpeg 9.0.1 did not include its libVorbis encoder, so `--encode` uses the installed Xiph reference encoder directly. FFmpeg remains useful for independent decoding and loudness measurements. With a libVorbis-enabled FFmpeg, the equivalent encoding quality is `-c:a libvorbis -q:a 6`; preserve the loop metadata and exact decoded sample count.

After inspecting the regenerated files, remove the three WAV music masters and the scratch directory. Keep the twelve effect WAV files and three music Ogg files in `content/sounds/`.

## Measured validation

Two encoding passes produced identical SHA-256 hashes for all three Ogg files with the installed libVorbis version.

The generator compiled with `-Wall -Wextra -Werror` and was formatted with clang-format 22.1.8. FFprobe identified every WAV as 16-bit PCM stereo at 44,100 Hz. All three shipped Ogg files decoded without errors to exactly 2,646,000 stereo frames, 10,584,000 bytes of signed 16-bit PCM each. The full 60-second loop therefore survives encoding without inserted silence or a sample-count mismatch.

FFmpeg 9.0.1 `ebur128=peak=true` measured the final Ogg files:

| Track | Integrated loudness | True peak | Loudness range | Ogg size |
| --- | ---: | ---: | ---: | ---: |
| Shore | -18.2 LUFS | -5.8 dBFS | 1.9 LU | 943,675 bytes |
| Current | -18.0 LUFS | -5.3 dBFS | 1.8 LU | 1,081,308 bytes |
| Aurora | -18.3 LUFS | -6.6 dBFS | 1.6 LU | 1,025,685 bytes |

Before encoding, each music master measured -19.58 dBFS RMS. The largest channel change between the last and first decoded PCM sample was 99/32768 for Shore, 191/32768 for Current, and 125/32768 for Aurora. Those measurements establish the boundary size, not a subjective listening result. The original circular PCM boundaries measured 0.003470, 0.003823, and 0.003506 on a full-scale range of -1 to 1.

The four-line clear measured -17.5 LUFS and -5.7 dBFS true peak; hard drop measured -22.3 LUFS and -7.8 dBFS true peak. Short-effect LUFS values depend on the measurement gate, so the gain table uses sample peaks and the intended event hierarchy instead. No source file clipped. This validation covers file formats, decoding, levels, lengths, and loop boundaries. It does not establish in-game latency, runtime crossfade behavior, or a human listening review.

## Embedded playback

Embedded players are technically available from [YouTube](https://developers.google.com/youtube/iframe_api_reference), [Spotify](https://developer.spotify.com/documentation/embeds) and [TIDAL](https://developer.tidal.com/documentation/embeds/embeds-overview). They do not by themselves expose decoded PCM for this visualizer. YouTube's documented iframe API exposes transport, state and timing, not an audio-sample stream. Spotify's platform policy excludes games and visual synchronization; TIDAL's [developer guidelines](https://developer.tidal.com/documentation/guidelines/guidelines-developer-guidelines) also prohibit synchronizing embedded content with visual output. Account authentication does not remove those limits. Stillwater retains user-selected audio sharing; this visual revision adds no embedded service player or account integration.
