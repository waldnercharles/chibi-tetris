// Stable particle identities and continuous forces shared by both depth layers.
const float COSMIC_TAU = 6.28318530718;

vec3 cosmic_random(float index, float seed, float variation)
{
    vec3 phase = index * vec3(0.731, 0.593, 0.419)
        + seed * vec3(0.113, 0.317, 0.713)
        + variation * vec3(0.017, 0.029, 0.043);
    return fract(sin(phase) * vec3(4375.585, 2257.145, 1964.267));
}

vec4 cosmic_stage_weights(float style)
{
    vec4 distance = abs(fract((style - vec4(0.0, 1.0, 2.0, 3.0) + 2.0) * 0.25) * 4.0 - 2.0);
    return 1.0 - smoothstep(0.0, 1.0, distance);
}

float cosmic_phase(ChibiParameters u)
{
    return u.Flow + u.Turn * 0.45 + fract(u.Variation * 0.0017) * COSMIC_TAU;
}

// Intersect the eye-to-particle ray with the board's world plane, z = 0.
// This remains correct during intro translation and camera rotation.
vec2 cosmic_board_projection(vec3 position, vec3 eye)
{
    return eye.xy + (position.xy - eye.xy) * eye.z / max(eye.z - position.z, 1.0);
}

float cosmic_board_quiet(vec2 projected, float duet)
{
    float horizontal = mix(abs(projected.x), abs(abs(projected.x) - 254.0), duet) / 148.0;
    float vertical = abs(projected.y - 20.0) / 268.0;
    return mix(0.075, 1.0, smoothstep(0.78, 1.65, max(horizontal, vertical)));
}

vec3 cosmic_music_motion(vec3 position, ChibiParameters u, float amount)
{
    float radius = length(position.xy);
    float compression = 1.0 + amount * u.Bass * (0.11 * sin(radius * 0.0035 - u.Pulse) - 0.065);
    vec3 curl = vec3(
        sin(position.y * 0.0019 + u.Wind),
        cos(position.x * 0.0017 - u.Wind * 0.7),
        sin((position.x + position.y) * 0.0013 + u.Wind * 0.55));
    vec3 displaced = vec3(position.xy * compression,
        position.z + amount * u.Bass * 150.0 * sin(radius * 0.0024 - u.Pulse));
    return displaced + curl * amount * u.Mid * (45.0 + u.Bass * 35.0);
}

// Source.xy is the recorded board-plane origin; Source.z is a continuous age.
// Drive carries radial, tangential, and vertical momentum. Drag and endpoint
// easing keep reuse invisible: displacement is zero at age 0 and age 2.6.
vec3 cosmic_impulse(vec2 projected, vec3 source, vec3 drive)
{
    if (source.z <= 0.0 || source.z >= 2.6) return vec3(0.0);
    vec2 delta = projected - source.xy;
    float radius = length(delta);
    float front = (radius - source.z * 320.0) / 170.0;
    float decay = source.z * exp(-source.z * 1.7) * (1.0 - smoothstep(1.8, 2.6, source.z));
    vec2 radial = delta * drive.x * 300.0 / max(radius, 1.0) / (1.0 + front * front);
    vec2 tangent = vec2(-delta.y, delta.x) * drive.y * 0.40;
    vec3 force = vec3(radial + tangent + vec2(0.0, drive.z * 70.0), drive.x * 55.0);
    return force * decay / (1.0 + radius * 0.003);
}

vec3 cosmic_event_motion(vec3 position, ChibiParameters u, float amount)
{
    vec2 projected = cosmic_board_projection(position, u.Eye);
    vec3 force = cosmic_impulse(projected, u.Source0, u.Drive0)
        + cosmic_impulse(projected, u.Source1, u.Drive1)
        + cosmic_impulse(projected, u.Source2, u.Drive2)
        + cosmic_impulse(projected, u.Source3, u.Drive3);
    float depth_scale = (u.Eye.z - position.z) / max(u.Eye.z, 1.0);
    return position + force * vec3(depth_scale, depth_scale, 1.0) * amount;
}

vec3 cosmic_palette(vec3 random, vec4 weights)
{
    vec3 nebula = mix(vec3(0.20, 0.10, 0.85), vec3(0.20, 0.88, 1.12), random.z);
    vec3 disk = mix(vec3(1.25, 0.84, 0.30), vec3(0.72, 0.15, 0.07), random.y);
    vec3 cells = mix(vec3(0.12, 0.95, 0.57), vec3(0.90, 0.19, 0.54), random.y);
    vec3 current = mix(vec3(0.25, 0.52, 1.10), vec3(0.94, 1.02, 1.18), random.z);
    return nebula * weights.x + disk * weights.y + cells * weights.z + current * weights.w;
}

vec3 cosmic_band_tint(vec3 tint, vec3 position, ChibiParameters u)
{
    vec3 spectrum = vec3(0.82 + u.Bass * 0.52 + u.Treble * 0.12,
        0.82 + u.Mid * 0.30 + u.Bass * 0.18,
        0.90 + u.Treble * 0.38 + u.Mid * 0.12);
    float band = sin(position.y * 0.0018 + u.Pulse * 0.18) * u.Mid * 0.08;
    return tint * (spectrum + band);
}

float cosmic_music_size(float size, vec3 random, ChibiParameters u)
{
    return size * (1.0 + u.Bass * 0.16
        + u.Treble * (0.15 + 0.9 * pow(random.z, 6.0)) + u.Beat * 0.14);
}

vec4 cosmic_color(vec3 random, vec3 position, vec4 weights, float density, ChibiParameters u)
{
    vec3 tint = cosmic_band_tint(cosmic_palette(random, weights), position, u);
    float radiance = 0.38 + u.Energy * 0.20 + u.Treble * 0.12 + u.Beat * 0.18 + u.Reaction * 0.12;
    float opacity = (0.10 + 0.68 * pow(random.z, 3.0))
        * cosmic_board_quiet(cosmic_board_projection(position, u.Eye), u.Duet)
        * (mix(0.08, 0.86, density) * weights.x
            + (0.28 + 0.5 * pow(1.0 - random.y, 2.0)) * weights.y
            + (0.3 + 0.5 * random.y) * weights.z + 0.8 * weights.w);
    return vec4(tint * radiance, opacity);
}
