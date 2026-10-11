/* Original Stillwater score and sound design. See docs/audio.md. */
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/vorbisenc.h>

#define RATE 44100
#define BPM 96
#define BEAT (60.0 / BPM)
#define BAR (4.0 * BEAT)
#define TAU 6.2831853071795864769
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

enum Voice
{
    PAD,
    FELT,
    GLASS,
    PLUCK,
    BASS,
    KICK,
    BRUSH,
    SHAKER,
    AIR
};

typedef struct
{
    int frames;
    bool loop;
    float *left;
    float *right;
    float *send_left;
    float *send_right;
} Mix;

typedef struct
{
    double beat;
    int pitch;
    double length;
    double strength;
} PhraseNote;

static uint32_t random_state = 0x71a93f12;
static const char *music_names[] = { "shore", "current", "aurora" };

static void *allocate(size_t size)
{
    void *memory = calloc(1, size);
    assert(memory);
    return memory;
}

static double noise()
{
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return (double)random_state / UINT32_MAX * 2.0 - 1.0;
}

static double frequency(int pitch)
{
    return 440.0 * exp2((pitch - 69) / 12.0);
}

static double ramp(double time, double length)
{
    double x = fmax(0.0, fmin(1.0, time / length));
    return x * x * (3.0 - 2.0 * x);
}

static Mix make_mix(double seconds, bool loop)
{
    Mix mix = { .frames = (int)llround(seconds * RATE), .loop = loop };
    size_t size = sizeof(float) * (size_t)mix.frames;
    mix.left = allocate(size);
    mix.right = allocate(size);
    mix.send_left = allocate(size);
    mix.send_right = allocate(size);
    return mix;
}

static void free_mix(Mix *mix)
{
    free(mix->left);
    free(mix->right);
    free(mix->send_left);
    free(mix->send_right);
}

static void note(Mix *mix, enum Voice voice, double start, double duration, int pitch, double gain, double pan)
{
    double hz = frequency(pitch);
    int begin = (int)llround(start * RATE);
    int frames = (int)llround(duration * RATE);
    double left = sqrt((1.0 - pan) * 0.5);
    double right = sqrt((1.0 + pan) * 0.5);
    double low_noise = 0.0;
    double old_noise = 0.0;
    double send = voice == PAD ? 0.50 : 0.34;
    if (voice == BASS || voice == KICK) {
        send = 0.025;
    }
    if (voice == BRUSH || voice == SHAKER) {
        send = 0.16;
    }
    for (int i = 0; i < frames; ++i) {
        int frame = begin + i;
        if (mix->loop) {
            frame = (frame % mix->frames + mix->frames) % mix->frames;
        }
        if (frame < 0 || frame >= mix->frames) {
            continue;
        }
        double time = (double)i / RATE;
        double phase = TAU * hz * time;
        double tail = ramp(duration - time, fmin(duration * 0.25, 0.28));
        double sample = 0.0;
        double width = 0.0;
        switch (voice) {
            case PAD: {
                double env = ramp(time, 0.38) * ramp(duration - time, 0.85);
                double bloom = 0.5 + 0.5 * sin(TAU * time / 7.3 + pitch);
                double drift = 0.0025 * sin(TAU * time / 4.7 + pitch);
                sample = sin(phase + drift) * 0.60;
                sample += sin(phase * 1.0017 + 0.8) * 0.25;
                sample += sin(phase * 0.9983 + 2.1) * 0.25;
                sample += sin(2.0 * phase) * (0.075 + bloom * 0.045);
                sample += sin(3.0 * phase + 0.3) * 0.038;
                sample *= env;
                width = sin(phase * 0.9991 + 1.5) * env * 0.10;
                break;
            }
            case FELT: {
                double strike = exp(-time * 80.0);
                sample = sin(phase) * exp(-time * 1.9);
                sample += sin(phase * 2.0006) * 0.27 * exp(-time * 3.5);
                sample += sin(phase * 3.004) * 0.12 * exp(-time * 6.0);
                sample += sin(phase * 4.012) * 0.045 * exp(-time * 10.0);
                low_noise += 0.12 * (noise() - low_noise);
                sample += low_noise * strike * 0.13;
                sample *= ramp(time, 0.005) * tail;
                break;
            }
            case GLASS:
                sample = sin(phase + sin(phase * 2.0) * exp(-time * 9.0) * 0.65);
                sample *= exp(-time * 1.7);
                sample += sin(phase * 3.001) * exp(-time * 2.8) * 0.19;
                sample += sin(phase * 5.004) * exp(-time * 5.7) * 0.065;
                sample *= ramp(time, 0.008) * tail;
                break;
            case PLUCK:
                sample = sin(phase + 0.7 * sin(phase) * exp(-time * 8.0));
                sample += sin(phase * 2.0) * 0.12 * exp(-time * 8.0);
                sample *= exp(-time * 4.7) * ramp(time, 0.004) * tail;
                break;
            case BASS:
                sample = sin(phase) + 0.15 * sin(phase * 2.0) +
                         0.045 * sin(phase * 3.0);
                sample *= ramp(time, 0.018) * ramp(duration - time, 0.17);
                sample *= 0.60 + 0.40 * exp(-time * 3.5);
                break;
            case KICK:
                sample = sin(TAU * (46.0 * time + 1.28 * (1.0 - exp(-time * 43.0))));
                sample *= exp(-time * 14.0) * ramp(time, 0.0025) * tail;
                break;
            case BRUSH: {
                double white = noise();
                low_noise += 0.22 * (white - low_noise);
                sample = (white - low_noise) * 0.30 + sin(TAU * 185.0 * time) * 0.12;
                sample *= exp(-time * 29.0) * ramp(time, 0.004) * tail;
                break;
            }
            case SHAKER: {
                double white = noise();
                sample = (white - old_noise) * 0.31;
                old_noise = white;
                sample *= ramp(time, 0.006) * exp(-time * 42.0) * tail;
                break;
            }
            case AIR:
                low_noise += 0.035 * (noise() - low_noise);
                sample = low_noise * 1.6 + sin(phase) * 0.12;
                sample *= sin(3.141592653589793 * time / duration);
                sample *= ramp(time, 0.02) * tail;
                break;
        }
        float l = (float)((sample + width) * gain * left);
        float r = (float)((sample - width) * gain * right);
        mix->left[frame] += l;
        mix->right[frame] += r;
        mix->send_left[frame] += (float)(l * send);
        mix->send_right[frame] += (float)(r * send);
    }
}

typedef struct
{
    int length;
    int position;
    float *buffer;
    double damped;
} Delay;

static Delay delay(int length)
{
    return (Delay){ .length = length,
                    .buffer = allocate(sizeof(float) * (size_t)length) };
}

static double comb(Delay *line, double input)
{
    double delayed = line->buffer[line->position];
    line->damped += 0.43 * (delayed - line->damped);
    line->buffer[line->position] = (float)(input + line->damped * 0.82);
    line->position = (line->position + 1) % line->length;
    return delayed;
}

static double allpass(Delay *line, double input)
{
    double delayed = line->buffer[line->position];
    double output = delayed - input * 0.5;
    line->buffer[line->position] = (float)(input + output * 0.5);
    line->position = (line->position + 1) % line->length;
    return output;
}

static void space(Mix *mix)
{
    static const int lengths[] = { 1559, 1993, 2381, 2749,
                                   1613, 2053, 2473, 2833 };
    Delay lines[8];
    Delay diffuse[4] = { delay(241), delay(673), delay(307), delay(709) };
    for (int i = 0; i < COUNT(lines); ++i) {
        lines[i] = delay(lengths[i]);
    }
    int warmup = mix->loop ? 5 * RATE : 0;
    for (int i = -warmup; i < mix->frames; ++i) {
        int frame = (i + mix->frames) % mix->frames;
        double wet_left = 0.0;
        double wet_right = 0.0;
        double in_left = mix->send_left[frame] * 0.78 + mix->send_right[frame] * 0.22;
        double in_right = mix->send_right[frame] * 0.78 + mix->send_left[frame] * 0.22;
        for (int j = 0; j < 4; ++j) {
            wet_left += comb(&lines[j], in_left) * 0.16;
            wet_right += comb(&lines[j + 4], in_right) * 0.16;
        }
        wet_left = allpass(&diffuse[1], allpass(&diffuse[0], wet_left));
        wet_right = allpass(&diffuse[3], allpass(&diffuse[2], wet_right));
        if (i < 0) {
            continue;
        }
        mix->left[frame] += (float)wet_left;
        mix->right[frame] += (float)wet_right;
    }
    for (int i = 0; i < COUNT(lines); ++i) {
        free(lines[i].buffer);
    }
    for (int i = 0; i < COUNT(diffuse); ++i) {
        free(diffuse[i].buffer);
    }
}

static void put16(FILE *file, uint16_t value)
{
    fputc(value & 255, file);
    fputc(value >> 8, file);
}

static void put32(FILE *file, uint32_t value)
{
    put16(file, (uint16_t)value);
    put16(file, (uint16_t)(value >> 16));
}

static void write_mix(Mix *mix, const char *directory, const char *name, double target_rms, double ceiling)
{
    space(mix);
    double peak = 0.0;
    double squares = 0.0;
    double sum_left = 0.0;
    double sum_right = 0.0;
    for (int i = 0; i < mix->frames; ++i) {
        sum_left += mix->left[i];
        sum_right += mix->right[i];
    }
    for (int i = 0; i < mix->frames; ++i) {
        double fade = mix->loop
                        ? 1.0
                        : ramp((double)i / RATE, 0.002) *
                              ramp((double)(mix->frames - 1 - i) / RATE, 0.12);
        mix->left[i] = (float)(tanh((mix->left[i] - sum_left / mix->frames) * 1.08) * fade);
        mix->right[i] = (float)(tanh((mix->right[i] - sum_right / mix->frames) * 1.08) *
                                fade);
        peak = fmax(peak, fmax(fabs(mix->left[i]), fabs(mix->right[i])));
        squares += mix->left[i] * mix->left[i] + mix->right[i] * mix->right[i];
    }
    double rms = sqrt(squares / (2.0 * mix->frames));
    assert(peak > 0.0 && isfinite(peak) && rms > 0.0);
    double gain = fmin(target_rms / rms, ceiling / peak);
    char path[1024];
    int size = snprintf(path, sizeof(path), "%s/%s.wav", directory, name);
    assert(size > 0 && size < (int)sizeof(path));
    FILE *file = fopen(path, "wb");
    assert(file);
    uint32_t bytes = (uint32_t)mix->frames * 4;
    fwrite("RIFF", 1, 4, file);
    put32(file, bytes + 36);
    fwrite("WAVEfmt ", 1, 8, file);
    put32(file, 16);
    put16(file, 1);
    put16(file, 2);
    put32(file, RATE);
    put32(file, RATE * 4);
    put16(file, 4);
    put16(file, 16);
    fwrite("data", 1, 4, file);
    put32(file, bytes);
    for (int i = 0; i < mix->frames; ++i) {
        double dither = (noise() - noise()) * 0.5;
        put16(file, (uint16_t)(int16_t)lrint(mix->left[i] * gain * 32767.0 + dither));
        dither = (noise() - noise()) * 0.5;
        put16(file, (uint16_t)(int16_t)lrint(mix->right[i] * gain * 32767.0 + dither));
    }
    assert(!ferror(file));
    int close_result = fclose(file);
    assert(close_result == 0);
    double seam = fmax(
                      fabs(mix->left[0] - mix->left[mix->frames - 1]),
                      fabs(mix->right[0] - mix->right[mix->frames - 1])
                  ) *
                  gain;
    printf(
        "%-10s duration=%6.2fs peak=%6.2fdBFS rms=%6.2fdBFS seam=%9.6f\n",
        name,
        (double)mix->frames / RATE,
        20.0 * log10(peak * gain),
        20.0 * log10(rms * gain),
        seam
    );
}

/* Voicings preserve common tones while the bass moves D, Bb, A, C. */
static const int chords[8][4] = { { 57, 60, 64, 65 }, { 57, 60, 64, 65 },
                                  { 57, 60, 62, 65 }, { 57, 60, 62, 65 },
                                  { 53, 60, 64, 67 }, { 53, 60, 64, 67 },
                                  { 55, 60, 62, 65 }, { 55, 60, 62, 64 } };
static const int roots[8] = { 38, 38, 34, 34, 33, 33, 36, 36 };

/* Each eight-bar melody breathes at the ends of two-bar phrases. */
static const PhraseNote melody[] = {
    { 0.5, 69, 1.0, .85 },  { 2.0, 72, .6, .66 },   { 3.0, 74, 1.6, .86 },
    { 5.5, 76, .7, .64 },   { 6.5, 72, 1.2, .72 },  { 8.5, 77, 1.4, .83 },
    { 10.5, 74, .8, .69 },  { 12.0, 72, .6, .62 },  { 13.0, 69, 1.7, .78 },
    { 16.0, 72, 1.2, .82 }, { 17.5, 76, .7, .66 },  { 18.5, 79, 1.5, .84 },
    { 21.0, 77, .8, .69 },  { 22.0, 76, 1.2, .70 }, { 24.5, 74, 1.0, .80 },
    { 26.0, 72, .6, .64 },  { 27.0, 67, 1.0, .71 }, { 29.0, 69, .7, .67 },
    { 30.0, 72, 1.3, .76 }
};

static void melody_part(Mix *mix, int stage, int cycle)
{
    for (int i = 0; i < COUNT(melody); ++i) {
        PhraseNote n = melody[i];
        if (stage == 0 && cycle == 0 && i % 3 == 1) {
            continue;
        }
        if (stage == 1 && cycle == 0 && i % 4 == 3) {
            continue;
        }
        double time = (cycle * 32.0 + n.beat) * BEAT;
        int octave = stage == 2 && cycle == 1 ? 12 : 0;
        enum Voice voice = stage == 0 ? FELT : GLASS;
        double pan = (i % 2 ? .16 : -.18);
        double gain = (stage == 0 ? .13 : .105) * n.strength;
        note(mix, voice, time + .012, n.length * BEAT + 1.1, n.pitch + octave, gain, pan);
        if (stage == 2 && cycle == 2 && i % 3 == 0) {
            note(mix, FELT, time + .06, 1.5, n.pitch - 12, .060, -.32);
        }
        if (cycle == 1 && i % 4 == 0) {
            note(mix, GLASS, time + BEAT * .75, 1.65, n.pitch + 12, stage == 2 ? .045 : .025, .48);
        }
    }
}

static void music(const char *directory, int stage)
{
    static const int arp[] = { 0, 2, 1, 3, 2, 1, 3, 2 };
    Mix mix = make_mix(BAR * 24, true);
    random_state = 0x71a93f12 + (uint32_t)stage * 193;
    for (int bar = 0; bar < 24; ++bar) {
        int harmony = bar % 8;
        int cycle = bar / 8;
        double time = bar * BAR;
        double lift = cycle == 1 ? 1.06 : cycle == 2 ? .95 : .88;
        for (int voice = 0; voice < 4; ++voice) {
            int pitch = chords[harmony][voice];
            double pan = -.62 + .41 * voice;
            note(&mix, PAD, time - .19, BAR + 1.10, pitch, (stage == 0 ? .057 : .060) * lift, pan);
            if (stage == 2) {
                note(&mix, PAD, time + .2, BAR + .4, pitch + 12, .016 * lift, -pan);
            }
        }
        note(
            &mix,
            BASS,
            time + .018,
            BEAT * (stage == 1 ? 1.4 : 3.2),
            roots[harmony],
            stage == 0 ? .155 : .190,
            0.0
        );
        if (stage != 0 || cycle == 1) {
            note(
                &mix,
                BASS,
                time + 2.5 * BEAT,
                BEAT * 1.1,
                roots[harmony] + (bar % 2 ? 12 : 7),
                stage == 0 ? .068 : .095,
                0.0
            );
        }
        int steps = stage == 0 ? 4 : 8;
        for (int step = 0; step < steps; ++step) {
            if (cycle == 2 && bar % 8 == 7 && step > 3) {
                continue;
            }
            int index = arp[(step + (bar % 2 ? 2 : 0)) % 8];
            int pitch = chords[harmony][index] + 12;
            if (stage == 2 && step % 4 == 3) {
                pitch += 12;
            }
            double beat = stage == 0 ? step + .25 : step * .5 + .25;
            double velocity = (step % 4 == 0 ? .072 : .048) * lift;
            if (stage == 0) {
                velocity *= .76;
            }
            note(
                &mix,
                stage == 0 ? FELT : PLUCK,
                time + beat * BEAT,
                stage == 0 ? 1.55 : .85,
                pitch,
                velocity,
                step % 2 ? .48 : -.46
            );
        }
        if (bar % 4 == 3) {
            note(&mix, GLASS, time + 2.75 * BEAT, 2.4, chords[harmony][3] + 24, .033, -.55);
            note(&mix, GLASS, time + 3.5 * BEAT, 2.1, chords[harmony][1] + 24, .023, .56);
        }
        if (stage == 0 && cycle == 0) {
            continue;
        }
        note(&mix, KICK, time + .006, .32, 36, stage == 0 ? .10 : .145, 0.0);
        if (stage != 0) {
            note(&mix, KICK, time + 2.0 * BEAT + .006, .31, 36, .113, 0.0);
            if (stage == 1 && bar % 2 == 1) {
                note(&mix, KICK, time + 3.5 * BEAT, .27, 36, .078, 0.0);
            }
        }
        for (int beat = 0; beat < 4; ++beat) {
            if (beat % 2 == 1) {
                note(&mix, BRUSH, time + beat * BEAT + .012, .23, 36, stage == 0 ? .038 : .075, .18);
            }
            note(
                &mix,
                SHAKER,
                time + (beat + .5) * BEAT + .01,
                .14,
                36,
                stage == 0 ? .020 : .033,
                beat % 2 ? -.34 : .36
            );
            if (stage == 1 && cycle != 0) {
                note(&mix, SHAKER, time + (beat + .75) * BEAT, .09, 36, .013, -.24);
            }
        }
        if (bar % 8 == 7) {
            note(&mix, AIR, time + BEAT * 2.5, BEAT * 1.5, 74, .036, .3);
        }
    }
    for (int cycle = 0; cycle < 3; ++cycle) {
        melody_part(&mix, stage, cycle);
    }
    write_mix(&mix, directory, music_names[stage], .105, .71);
    free_mix(&mix);
}

static void chord(Mix *mix, double time, const int *pitches, int count, double duration, double gain)
{
    for (int i = 0; i < count; ++i) {
        note(
            mix,
            GLASS,
            time + i * .035,
            duration,
            pitches[i],
            gain,
            count == 1 ? 0.0 : -.35 + .70 * i / (count - 1)
        );
    }
}

static void effect(const char *directory, const char *name, int id)
{
    static const double durations[] = { .18,  .28, .55,  .90,  1.25, 1.65,
                                        2.40, .70, 1.80, 3.60, 2.50, .65 };
    static const int clear_notes[] = { 62, 65, 69, 72 };
    Mix mix = make_mix(durations[id], false);
    random_state = 0x813f5412 + (uint32_t)id * 971;
    switch (id) {
        case 0: note(&mix, FELT, 0, .14, 86, .23, 0); break;
        case 1:
            note(&mix, PLUCK, 0, .22, 81, .22, -.12);
            note(&mix, GLASS, .045, .20, 86, .10, .16);
            break;
        case 2:
            note(&mix, FELT, 0, .42, 50, .30, 0);
            note(&mix, PLUCK, .012, .32, 69, .12, -.12);
            note(&mix, BRUSH, 0, .10, 36, .045, .15);
            break;
        case 3:
            note(&mix, KICK, 0, .35, 36, .43, 0);
            note(&mix, FELT, .008, .65, 50, .20, -.15);
            note(&mix, GLASS, .014, .65, 74, .07, .15);
            note(&mix, BRUSH, 0, .14, 36, .11, 0);
            break;
        case 4:
            chord(&mix, 0, clear_notes, 3, .98, .16);
            note(&mix, GLASS, .10, .9, 77, .12, .25);
            break;
        case 5:
            chord(&mix, 0, clear_notes, 4, 1.30, .15);
            note(&mix, GLASS, .15, 1.20, 77, .15, -.2);
            note(&mix, GLASS, .26, 1.05, 81, .12, .3);
            break;
        case 6:
            chord(&mix, 0, clear_notes, 4, 1.80, .18);
            note(&mix, BASS, 0, .7, 38, .17, 0);
            for (int i = 0; i < 4; ++i) {
                note(&mix, GLASS, .12 + .10 * i, 1.65, clear_notes[i] + 12, .14, -.4 + .26 * i);
            }
            note(&mix, AIR, 0, .5, 86, .06, .2);
            break;
        case 7:
            note(&mix, FELT, 0, .5, 77, .20, -.2);
            note(&mix, GLASS, .06, .45, 72, .12, .2);
            break;
        case 8:
            note(&mix, AIR, 0, .45, 74, .17, -.2);
            for (int i = 0; i < 3; ++i) {
                note(&mix, GLASS, .18 + .095 * i, 1.20, clear_notes[i] + 12, .19, -.25 + i * .25);
            }
            break;
        case 9: {
            static const int arrival[] = { 53, 60, 65, 69, 74 };
            chord(&mix, 0, clear_notes, 4, 1.60, .11);
            chord(&mix, .66, arrival, COUNT(arrival), 2.55, .15);
            note(&mix, BASS, .66, 1.7, 41, .16, 0);
            note(&mix, GLASS, 1.04, 2.2, 81, .12, .3);
            break;
        }
        case 10:
            note(&mix, FELT, 0, 1.4, 69, .18, .25);
            note(&mix, FELT, .18, 1.4, 65, .18, 0);
            note(&mix, FELT, .36, 1.7, 62, .22, -.25);
            note(&mix, PAD, .14, 2.10, 50, .20, 0);
            break;
        case 11:
            note(&mix, FELT, 0, .48, 74, .19, -.14);
            note(&mix, GLASS, .075, .44, 81, .13, .14);
            break;
    }
    write_mix(&mix, directory, name, id <= 3 ? .070 : .085, .56);
    free_mix(&mix);
}

static void write_page(FILE *file, const ogg_page *page)
{
    size_t header = fwrite(page->header, 1, (size_t)page->header_len, file);
    size_t body = fwrite(page->body, 1, (size_t)page->body_len, file);
    assert(header == (size_t)page->header_len && body == (size_t)page->body_len);
}

/* Encode the exact PCM format written above; this is not a general WAV reader. */
static void encode(const char *directory, int track)
{
    char input_path[1024];
    char output_path[1024];
    int input_length = snprintf(input_path, sizeof(input_path), "%s/%s.wav", directory, music_names[track]);
    int output_length = snprintf(output_path, sizeof(output_path), "%s/%s.ogg", directory, music_names[track]);
    assert(input_length > 0 && input_length < (int)sizeof(input_path));
    assert(output_length > 0 && output_length < (int)sizeof(output_path));
    FILE *input = fopen(input_path, "rb");
    FILE *output = fopen(output_path, "wb");
    assert(input && output);
    unsigned char header[44];
    size_t read = fread(header, 1, sizeof(header), input);
    assert(read == sizeof(header));
    assert(!memcmp(header, "RIFF", 4) && !memcmp(header + 8, "WAVEfmt ", 8));
    assert(header[20] == 1 && header[22] == 2 && header[34] == 16);
    assert(header[24] == 0x44 && header[25] == 0xac && !memcmp(header + 36, "data", 4));

    vorbis_info info;
    vorbis_info_init(&info);
    int result = vorbis_encode_init_vbr(&info, 2, RATE, .6f);
    assert(result == 0);
    vorbis_comment comments;
    vorbis_comment_init(&comments);
    vorbis_comment_add_tag(&comments, "TITLE", music_names[track]);
    vorbis_comment_add_tag(&comments, "ARTIST", "Stillwater");
    vorbis_comment_add_tag(&comments, "LICENSE", "CC0-1.0");
    vorbis_comment_add_tag(&comments, "LOOPSTART", "0");
    vorbis_comment_add_tag(&comments, "LOOPLENGTH", "2646000");
    vorbis_dsp_state state;
    result = vorbis_analysis_init(&state, &info);
    assert(result == 0);
    vorbis_block block;
    result = vorbis_block_init(&state, &block);
    assert(result == 0);
    ogg_stream_state stream;
    result = ogg_stream_init(&stream, 0x57111 + track);
    assert(result == 0);
    ogg_packet packets[3];
    result = vorbis_analysis_headerout(&state, &comments, &packets[0], &packets[1], &packets[2]);
    assert(result == 0);
    for (int i = 0; i < COUNT(packets); ++i) {
        ogg_stream_packetin(&stream, &packets[i]);
    }
    ogg_page page;
    while (ogg_stream_flush(&stream, &page)) {
        write_page(output, &page);
    }
    bool finished = false;
    while (!finished) {
        unsigned char pcm[2048 * 4];
        int frames = (int)fread(pcm, 4, 2048, input);
        assert(!ferror(input));
        float **buffer = vorbis_analysis_buffer(&state, frames);
        for (int i = 0; i < frames; ++i) {
            buffer[0][i] = (int16_t)(pcm[i * 4] | pcm[i * 4 + 1] << 8) / 32768.0f;
            buffer[1][i] = (int16_t)(pcm[i * 4 + 2] | pcm[i * 4 + 3] << 8) / 32768.0f;
        }
        vorbis_analysis_wrote(&state, frames);
        while (vorbis_analysis_blockout(&state, &block) == 1) {
            vorbis_analysis(&block, NULL);
            vorbis_bitrate_addblock(&block);
            ogg_packet packet;
            while (vorbis_bitrate_flushpacket(&state, &packet)) {
                ogg_stream_packetin(&stream, &packet);
                while (ogg_stream_pageout(&stream, &page)) {
                    write_page(output, &page);
                    if (ogg_page_eos(&page)) {
                        finished = true;
                    }
                }
            }
        }
    }
    ogg_stream_clear(&stream);
    vorbis_block_clear(&block);
    vorbis_dsp_clear(&state);
    vorbis_comment_clear(&comments);
    vorbis_info_clear(&info);
    result = fclose(input);
    assert(result == 0);
    result = fclose(output);
    assert(result == 0);
    printf("Encoded %s with %s, quality 6\n", output_path, vorbis_version_string());
}

int main(int argc, char **argv)
{
    if (argc != 3 || (strcmp(argv[1], "--all") && strcmp(argv[1], "--music") &&
                      strcmp(argv[1], "--effects") && strcmp(argv[1], "--encode"))) {
        fprintf(stderr, "Usage: %s --all|--music|--effects|--encode existing-output-directory\n", argv[0]);
        return 1;
    }
    if (!strcmp(argv[1], "--encode")) {
        for (int i = 0; i < COUNT(music_names); ++i) {
            encode(argv[2], i);
        }
        return 0;
    }
    bool all = !strcmp(argv[1], "--all");
    if (all || !strcmp(argv[1], "--music")) {
        for (int stage = 0; stage < 3; ++stage) {
            music(argv[2], stage);
        }
    }
    if (all || !strcmp(argv[1], "--effects")) {
        static const char *names[] = { "move",      "rotate", "lock",
                                       "hard_drop", "clear1", "clear2",
                                       "clear4",    "hold",   "rescue",
                                       "complete",  "topout", "menu" };
        for (int i = 0; i < COUNT(names); ++i) {
            effect(argv[2], names[i], i);
        }
    }
    return 0;
}
