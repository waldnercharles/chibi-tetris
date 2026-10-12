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

vec2 cosmic_board_projection(vec3 position, vec3 eye)
{
    return eye.xy + (position.xy - eye.xy) * eye.z / max(eye.z - position.z, 1.0);
}

float cosmic_board_quiet(vec2 projected, float duet)
{
    float horizontal = mix(abs(projected.x), abs(abs(projected.x) - 254.0), duet) / 148.0;
    float vertical = abs(projected.y - 20.0) / 268.0;
    return mix(0.055, 1.0, smoothstep(0.78, 1.65, max(horizontal, vertical)));
}

// Curl of a trigonometric vector potential, with zero divergence. Rotated
// octaves break axis alignment. The GPU integrates velocity through this field;
// this function never prescribes a particle's final position or a surface.
vec3 cosmic_curl(vec3 q, float phase)
{
    vec3 a = q + vec3(phase * 0.071, -phase * 0.053, phase * 0.037);
    vec3 first = vec3(sin(a.z) + cos(a.y), sin(a.x) + cos(a.z), sin(a.y) + cos(a.x));
    mat3 basis = mat3(0.0, 0.8, 0.6, -0.8, 0.36, -0.48, -0.6, -0.48, 0.64);
    vec3 b = basis * q * 2.13 + vec3(-phase * 0.041, phase * 0.033, phase * 0.027);
    vec3 second = transpose(basis) * vec3(sin(b.z) + cos(b.y), sin(b.x) + cos(b.z), sin(b.y) + cos(b.x));
    vec3 c = q * 4.31 + vec3(1.7, -2.1, 4.3) + phase * 0.017;
    vec3 detail = vec3(sin(c.z) + cos(c.y), sin(c.x) + cos(c.z), sin(c.y) + cos(c.x));
    return first * 0.62 + second * 0.26 + detail * 0.09;
}

vec3 cosmic_ball(vec3 random)
{
    float z = random.y * 2.0 - 1.0;
    float ring = sqrt(max(0.0, 1.0 - z * z));
    float angle = random.x * COSMIC_TAU;
    return vec3(cos(angle) * ring, sin(angle) * ring, z) * pow(random.z, 0.3333333333);
}

vec3 cosmic_impulse(vec2 projected, vec3 source, vec3 drive)
{
    if (source.z <= 0.0 || source.z >= 2.6) return vec3(0.0);
    vec2 delta = projected - source.xy;
    float radius = length(delta);
    float front = (radius - source.z * 320.0) / 170.0;
    float decay = source.z * exp(-source.z * 1.7) * (1.0 - smoothstep(1.8, 2.6, source.z));
    vec2 radial = delta * drive.x * 300.0 / max(radius, 1.0) / (1.0 + front * front);
    vec2 tangent = vec2(-delta.y, delta.x) * drive.y * 0.40;
    return vec3(radial + tangent + vec2(0.0, drive.z * 70.0), drive.x * 55.0)
        * decay / (1.0 + radius * 0.003);
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

vec3 cosmic_tint(vec3 random, vec3 position, vec4 weights, ChibiParameters u)
{
    // The reference's ivory highlights emerge from density, with colored shadow
    // grains. Separate frequency bands color different depths and populations.
    float hot = smoothstep(0.38, 0.94, random.y + u.Bass * 0.14);
    vec3 swarm = mix(vec3(0.26, 0.49, 0.62), vec3(1.04, 0.91, 0.65), hot);
    vec3 filament = mix(vec3(0.31, 0.13, 0.46), vec3(1.18, 0.62, 0.27), hot);
    vec3 volume = mix(vec3(0.17, 0.40, 0.59), vec3(0.64, 0.91, 0.80), hot);
    vec3 wind = mix(vec3(0.24, 0.34, 0.57), vec3(0.90, 0.76, 0.65), hot);
    vec3 tint = swarm * weights.x + filament * weights.y + volume * weights.z + wind * weights.w;
    vec3 spectrum = vec3(0.88 + u.Bass * 0.36, 0.91 + u.Mid * 0.27, 0.94 + u.Treble * 0.48);
    float depth = 1.0 - smoothstep(1850.0, 3500.0, -position.z);
    return tint * spectrum * mix(0.17, 1.0, depth);
}
