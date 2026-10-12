// The same spatial current carries ambient dust and action trails.
#ifndef PARTICLE_FLOW_GLSL
#define PARTICLE_FLOW_GLSL

uint particle_hash(uint value)
{
    value = (value ^ (value >> 16u)) * 2146121005u;
    value = (value ^ (value >> 15u)) * 2221713035u;
    return value ^ (value >> 16u);
}

vec3 particle_samples(uint identity)
{
    return vec3(float(particle_hash(identity) >> 8u),
        float(particle_hash(identity + 1013u) >> 8u),
        float(particle_hash(identity + 7919u) >> 8u)) / 16777216.0;
}

vec2 cosmic_board_projection(vec3 position, vec3 eye)
{
    return eye.xy + (position.xy - eye.xy) * eye.z / max(eye.z - position.z, 1.0);
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

vec3 cosmic_impulse(vec2 projected, vec3 source, vec3 drive)
{
    if (source.z <= 0.0) return vec3(0.0);
    vec2 delta = projected - source.xy;
    float radius = length(delta);
    float front = (radius - min(source.z, 0.9) * 240.0) / 190.0;
    float decay = smoothstep(0.0, 0.14, source.z);
    vec2 radial = delta * drive.x * 105.0 / max(radius, 1.0) / (1.0 + front * front);
    vec2 tangent = vec2(-delta.y, delta.x) * drive.y * 0.22;
    return vec3(radial + tangent + vec2(0.0, drive.z * 70.0), drive.x * 25.0)
        * decay / (1.0 + radius * 0.003);
}

// A finite annulus travels from the actual landing center. The middle cools
// behind it; summing independent fronts also lights grains born before a drop.
float particle_wave(vec2 projected, vec3 wave, float power)
{
    if (wave.z <= 0.0 || wave.z >= 1.6 || power <= 0.0) return 0.0;
    vec2 offset = projected - wave.xy;
    float distance = length(offset) - wave.z * 650.0;
    float shell = smoothstep(-210.0, -40.0, distance) *
        (1.0 - smoothstep(40.0, 130.0, distance));
    float envelope = smoothstep(0.0, 0.055, wave.z) *
        (1.0 - smoothstep(1.18, 1.6, wave.z));
    return shell * envelope * power;
}

float particle_wave_light(vec3 position, ChibiParameters u)
{
    vec2 projected = cosmic_board_projection(position, u.Eye);
    float light = particle_wave(projected, u.Wave0, u.WavePower01.x)
        + particle_wave(projected, u.Wave1, u.WavePower01.y)
        + particle_wave(projected, u.Wave2, u.WavePower23.x)
        + particle_wave(projected, u.Wave3, u.WavePower23.y);
    return min(1.8, light);
}

#endif
