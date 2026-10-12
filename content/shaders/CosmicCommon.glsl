#include "ParticleFlow.glsl"

// Stable identities, a continuous divergence-free flow, and board-space forces.
const float COSMIC_TAU = 6.28318530718;

uint cosmic_hash(uint value)
{
    value = (value ^ (value >> 16u)) * 2146121005u;
    value = (value ^ (value >> 15u)) * 2221713035u;
    return value ^ (value >> 16u);
}

vec3 cosmic_random(float index, float seed, float variation)
{
    uint identity = uint(index) ^ cosmic_hash(uint(seed) + 19u)
        ^ cosmic_hash(uint(variation) + 127u);
    return vec3(float(cosmic_hash(identity) >> 8u),
        float(cosmic_hash(identity + 1013u) >> 8u),
        float(cosmic_hash(identity + 7919u) >> 8u)) / 16777216.0;
}

vec4 cosmic_stage_weights(float style)
{
    vec4 distance = abs(fract((style - vec4(0.0, 1.0, 2.0, 3.0) + 2.0) * 0.25) * 4.0 - 2.0);
    return 1.0 - smoothstep(0.0, 1.0, distance);
}

float cosmic_board_quiet(vec2 projected, float duet)
{
    float horizontal = mix(abs(projected.x), abs(abs(projected.x) - 254.0), duet) / 148.0;
    float vertical = abs(projected.y - 20.0) / 268.0;
    return mix(0.055, 1.0, smoothstep(0.78, 1.65, max(horizontal, vertical)));
}

vec3 cosmic_ball(vec3 random)
{
    float z = random.y * 2.0 - 1.0;
    float ring = sqrt(max(0.0, 1.0 - z * z));
    float angle = random.x * COSMIC_TAU;
    return vec3(cos(angle) * ring, sin(angle) * ring, z) * pow(random.z, 0.3333333333);
}

vec3 cosmic_event_force(vec3 position, ChibiParameters u)
{
    vec2 projected = cosmic_board_projection(position, u.Eye);
    vec3 force = cosmic_impulse(projected, u.Source0, u.Drive0)
        + cosmic_impulse(projected, u.Source1, u.Drive1)
        + cosmic_impulse(projected, u.Source2, u.Drive2)
        + cosmic_impulse(projected, u.Source3, u.Drive3);
    float depth_scale = (u.Eye.z - position.z) / max(u.Eye.z, 1.0);
    return force * vec3(depth_scale, depth_scale, 1.0);
}

float cosmic_source_light(vec2 projected, vec3 source, vec3 drive)
{
    if (source.z <= 0.0) return 0.0;
    float radius = length(projected - source.xy);
    float spread = 100.0 + min(source.z, 1.0) * 140.0;
    float distance = radius / spread;
    float attack = smoothstep(0.0, 0.10, source.z);
    return (drive.x + abs(drive.y) * 0.28) * attack /
        (1.0 + distance * distance * distance * distance);
}

// Shared draw response for both ambient fields. The bounded result comes only
// from accepted action origins; brightness follows their existing force clock.
float cosmic_event_light(vec3 position, ChibiParameters u)
{
    vec2 projected = cosmic_board_projection(position, u.Eye);
    float light = cosmic_source_light(projected, u.Source0, u.Drive0)
        + cosmic_source_light(projected, u.Source1, u.Drive1)
        + cosmic_source_light(projected, u.Source2, u.Drive2)
        + cosmic_source_light(projected, u.Source3, u.Drive3);
    return min(1.0, light);
}

vec3 cosmic_tint(vec3 random, vec3 position, vec4 weights, ChibiParameters u)
{
    // The reference's ivory highlights emerge from density, with colored shadow
    // grains. Separate frequency bands color different depths and populations.
    float hot = smoothstep(0.38, 0.94, random.y + u.Bass * 0.14);
    vec3 swarm = mix(vec3(0.10, 0.34, 0.56), vec3(0.58, 1.02, 1.16), hot);
    vec3 filament = mix(vec3(0.30, 0.38, 0.48), vec3(1.12, 1.03, 0.82), hot);
    vec3 volume = mix(vec3(0.36, 0.16, 0.08), vec3(1.20, 0.71, 0.25), hot);
    vec3 wind = mix(vec3(0.08, 0.30, 0.34), vec3(0.54, 1.04, 0.84), hot);
    vec3 tint = swarm * weights.x + filament * weights.y + volume * weights.z + wind * weights.w;
    vec3 spectrum = vec3(0.88 + u.Bass * 0.36, 0.91 + u.Mid * 0.27, 0.94 + u.Treble * 0.48);
    float depth = 1.0 - smoothstep(1850.0, 3500.0, -position.z);
    return tint * spectrum * mix(0.17, 1.0, depth);
}
