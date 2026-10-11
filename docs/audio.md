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

Each launch offers two in-world actions beside a live demonstration: **Enter Play** starts the selected mode; **M Connect audio** opens the browser's sharing chooser directly. Click either label or use its key. Play keeps a connected external source or chooses the bundled score when no external source is ready. There are no provider launch buttons or account credentials in the game.

Play music in a separate browser tab, press M, select that tab in the sharing picker and enable its audio option. A desktop Chromium browser is the practical starting point for tab audio. Window or system audio is also usable when the browser and operating system offer it. Source selection always needs a user gesture and permission; the game cannot select a tab on your behalf. [Screen Capture API](https://developer.mozilla.org/en-US/docs/Web/API/MediaDevices/getDisplayMedia).

A connected stream remains on the opening screen until you press Play, including during a quiet passage. The HUD distinguishes connection from measured sound: **Audio connected** or **Connected · quiet**. An unavailable, cancelled or ended capture retains **Play** and **Connect audio** for fallback or retry. Pressing Enter when no stream is ready explicitly chooses the bundled score; capture failure does not silently change the source. Reconnecting during an existing run preserves the board and returns to its previous play or pause state after explicit Play.

During play, pause or a completed run, M reconnects while preserving the board, elapsed time and return screen. A paused run remains paused after connection. The source measures actual samples; level, frequency bands and transients drive the scene and cosmetic piece motion. Logical falling, input and lock timing stay predictable. External music stays audible in its own player; the game does not play a duplicate, save or upload the capture. Browser sharing requires a video track, but only audio is consumed.

The game does not sign into YouTube, Spotify or TIDAL. A service's player can supply shared audio when the browser, operating system and playback protection permit it. Account sign-in is not proof of captured audio. The same path can use a local audio file playing in a browser tab. Scene changes follow sustained spectrum changes, silence/resumption and musical phrase timers; they do not read track names or provider metadata.

Native capture uses a loopback recording device. Set `CHIBI_AUDIO_INPUT` to its exact device name before launching. Without that setting, the engine selects the first recognized loopback device: `Background Music`, or a name containing `BlackHole`, `Soundflower`, `Stereo Mix`, `CABLE Output` or `Monitor of`. It does not choose the default microphone automatically.

For example, with Background Music installed on macOS:

```sh
CHIBI_AUDIO_INPUT="Background Music" dist/Stillwater.app/Contents/MacOS/Stillwater
```

Route the external music player's audio through that virtual device, then press **M Connect audio**. Selecting a device alone does not route audio into it. Other loopback devices need their corresponding system or player routing. Configure monitoring or a multi-output route if the virtual device otherwise makes your speakers silent; Stillwater does not monitor captured audio. On Linux, PulseAudio monitor inputs also require `SDL_AUDIO_INCLUDE_MONITORS=1` before launch. An unavailable device leaves **Connect audio** and **Enter Play** available.

Stillwater does not implement Spotify sign-in or the Web Playback SDK. Spotify's [Developer Policy](https://developer.spotify.com/policy) excludes games and synchronization of sound recordings with visual media. Its [Web Playback SDK](https://developer.spotify.com/documentation/web-playback-sdk/reference) repeats the synchronization restriction and requires Premium for playback. Spotify also removed new-app access to [Audio Features and Audio Analysis](https://developer.spotify.com/blog/2024-11-27-changes-to-the-web-api), so those endpoints are not a beat-data source for this game. This is an engineering choice based on the published platform rules, not a claim that every use of user-selected audio capture is permitted by every music service.

Spotify's newer [Soloist client](https://developer.spotify.com/documentation/soloist) provides playback and local controls on Linux hardware. It requires each user to obtain an API key and install a periodically renewed native client. Its [WebSocket interface](https://developer.spotify.com/documentation/soloist/features) has no browser-facing security controls and is not a PCM input for this browser game. It is not part of Stillwater.

These sources were checked on October 11, 2026. Build and platform verification, including unverified external routing, are recorded in [validation](validation.md).

## Feedback

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
