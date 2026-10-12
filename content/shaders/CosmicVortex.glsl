#include "CosmicCommon.glsl"

// Organic shapes use the same random identity through every stage transition.
vec3 nebula_cloud(vec3 random, float phase, ChibiParameters u)
{
    vec3 domain = (random - 0.5) * vec3(4.6, 2.0, 2.0);
    vec3 warp = vec3(
        sin(domain.y * 2.4 + sin(domain.z * 2.1 + phase * 0.065)),
        sin(domain.x * 1.8 + sin(domain.z * 2.8 - phase * 0.055)),
        cos(domain.x * 1.4 + domain.y * 1.9 + phase * 0.045));
    return vec3(domain.x * 970.0 + warp.x * (180.0 + u.Mid * 55.0),
        domain.x * 215.0 + warp.y * 280.0 + domain.y * (230.0 + 210.0 * warp.z * warp.z + u.Bass * 75.0),
        -1550.0 + domain.z * 440.0 + warp.z * (210.0 + u.Reaction * 80.0));
}

vec3 accretion_disk(vec3 random, float phase, ChibiParameters u)
{
    float radius = 660.0 + pow(random.y, 1.45) * 1600.0;
    float angle = random.x * COSMIC_TAU + phase * (0.035 + 0.2 / (0.32 + random.y));
    float azimuth = angle + sin(radius * 0.004 - phase * 0.04) * 0.12;
    float depth = (random.z - 0.5) * (42.0 + random.y * 180.0 + u.Mid * 24.0)
        + sin(angle * 3.0 + radius * 0.005 - phase * 0.055) * (18.0 + u.Bass * 16.0 + u.Reaction * 14.0);
    vec3 disk = vec3(cos(azimuth) * radius, sin(azimuth) * radius, depth);
    return vec3(disk.x * 0.94 - disk.y * 0.16, disk.x * 0.23 + disk.y * 0.43 + 90.0,
        -1700.0 + disk.y * 0.33 + disk.z);
}

vec3 cellular_bloom(vec3 random, float phase, ChibiParameters u)
{
    float cell = floor(random.z * 13.0);
    float angle = random.x * COSMIC_TAU + phase * 0.035;
    float spread = sqrt((cell + 1.0) / 13.0);
    vec3 center = vec3(sin(cell * 2.399963) * spread * 1940.0,
        cos(cell * 2.399963) * spread * 1110.0, -1650.0 + sin(cell * 4.13) * 300.0);
    float radius = (235.0 + pow(random.y, 0.25) * 180.0)
        * (1.0 + 0.17 * sin(angle * 3.0 + cell * 1.7 + phase * 0.09)
            + 0.13 * sin(angle * 5.0 - phase * 0.06 + cell)
            + 0.12 * sin(phase * 0.075 + cell * 2.1) + u.Bass * 0.08 + u.Reaction * 0.055);
    return center + vec3(cos(angle) * radius, sin(angle) * radius * (0.8 + u.Mid * 0.08),
        cos(angle * 2.0 + cell + phase * 0.06) * 120.0 + (fract(random.z * 13.0) - 0.5) * 160.0);
}

vec3 stellar_current(vec3 random, float phase, ChibiParameters u)
{
    float x = (random.x - 0.5) * 4400.0;
    float lane = floor(random.y * 7.0);
    float width = fract(random.y * 7.0) - 0.5;
    return vec3(x + sin(phase * 0.07 + lane * 1.3) * 160.0,
        (lane - 3.0) * 285.0 + x * 0.18
            + sin(x * 0.0018 + lane * 0.9 + phase * 0.095) * (145.0 + u.Bass * 60.0)
            + width * (26.0 + u.Mid * 28.0),
        -1750.0 + sin(x * 0.0012 + lane * 1.7 - phase * 0.055) * 350.0
            + (random.z - 0.5) * (140.0 + u.Reaction * 90.0));
}

// Smooth value noise changes density without changing particle identity.
float cloud_hash(vec3 cell)
{
    return fract(sin(dot(cell, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float cloud_noise(vec3 point)
{
    vec3 cell = floor(point);
    vec3 f = fract(point);
    f = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    float z0 = mix(mix(cloud_hash(cell), cloud_hash(cell + vec3(1.0, 0.0, 0.0)), f.x),
        mix(cloud_hash(cell + vec3(0.0, 1.0, 0.0)), cloud_hash(cell + vec3(1.0, 1.0, 0.0)), f.x), f.y);
    float z1 = mix(mix(cloud_hash(cell + vec3(0.0, 0.0, 1.0)), cloud_hash(cell + vec3(1.0, 0.0, 1.0)), f.x),
        mix(cloud_hash(cell + vec3(0.0, 1.0, 1.0)), cloud_hash(cell + vec3(1.0, 1.0, 1.0)), f.x), f.y);
    return mix(z0, z1, f.z);
}

vec3 vortex_scale(vec3 random, vec4 weights)
{
    return mix(vec3(1.0, 0.8, 0.7), vec3(3.4, 0.5, 1.2), random.y) * weights.x
        + vec3(1.8 + random.z * 2.0, 0.45, 0.6) * weights.y
        + vec3(0.7, 1.1 + random.z * 1.6, 0.8) * weights.z
        + vec3(2.4 + random.z * 4.0, 0.32, 0.42) * weights.w;
}

vec3 vortex_rotation(vec3 random, vec4 weights, float phase)
{
    float angle = (0.25 + sin(random.x * 5.0 + phase * 0.07) * 0.5) * weights.x
        + (random.x * COSMIC_TAU + sin(phase * 0.04) * 0.35) * weights.y
        + (random.x * COSMIC_TAU + sin(phase * 0.035) * 0.25) * weights.z
        + (0.18 + cos(random.x * 7.92 + random.y * 6.3 + phase * 0.095) * 0.23) * weights.w;
    return vec3(0.12 * sin(random.y * COSMIC_TAU + phase * 0.03),
        0.16 * cos(random.z * COSMIC_TAU + phase * 0.025), angle);
}

ChibiParticle vortex(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    float phase = cosmic_phase(u);
    // Stage weights are uniform across the field. Evaluate only the visible
    // formation (two during a transition), keeping all particle identities.
    vec3 position = vec3(0.0);
    if (weights.x > 0.0) position += nebula_cloud(random, phase, u) * weights.x;
    if (weights.y > 0.0) position += accretion_disk(random, phase, u) * weights.y;
    if (weights.z > 0.0) position += cellular_bloom(random, phase, u) * weights.z;
    if (weights.w > 0.0) position += stellar_current(random, phase, u) * weights.w;
    position = cosmic_music_motion(position, u, 1.0);
    position = cosmic_event_motion(position, u, 1.0);
    float size = (0.3 + 1.9 * pow(random.z, 5.0))
        * max(0.7, (u.Eye.z - position.z) / 2300.0)
        * dot(weights, vec4(1.18, 0.8, 0.92, 0.64));
    float density = 1.0;
    if (weights.x > 0.0) {
        density = smoothstep(0.18, 0.78,
            cloud_noise(random * vec3(7.0, 4.0, 5.0) + vec3(phase * 0.019, phase * 0.013, 0.0)));
    }
    ChibiParticle result = chibi_particle();
    result.position = position;
    result.size = cosmic_music_size(size, random, u);
    result.color = cosmic_color(random, position, weights, density, u);
    result.scale = vortex_scale(random, weights) * vec3(1.0 + u.Treble * 0.45, 1.0 + u.Beat * 0.10, 1.0);
    result.rotation = vortex_rotation(random, weights, phase);
    result.lighting = 0.6 + random.y * 0.28;
    return result;
}
