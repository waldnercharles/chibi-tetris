// Shared deterministic samples. Both entry points retain the original hashes.
#ifndef REACTION_COMMON_GLSL
#define REACTION_COMMON_GLSL

#include "ParticleFlow.glsl"

vec3 reaction_random(float index, float seed)
{
    return fract(sin(vec3(
        index * 127.1 + seed,
        index * 269.5 + seed * 2.0,
        index * 311.7 + seed * 3.0
    )) * 43758.5453);
}

float reaction_quantile(float index, float seed)
{
    return fract(sin(index * 71.37 + seed * 5.0) * 19417.345);
}

// Preserve the authored float selector, including its half-unit boundary.
float reaction_pattern(float pattern, float value)
{
    return 1.0 - step(0.5, abs(pattern - value));
}

float reaction_birth(float span, float quantile)
{
    return span * (exp(log(65.0) * quantile) - 1.0) / 64.0;
}

float reaction_drag(float rate, float seconds)
{
    return (1.0 - exp(-rate * seconds)) / rate;
}

// Live uniforms change acceleration only. Existing grains keep their position
// and velocity when a later input bends the same current.
vec3 reaction_force(vec3 position, ChibiParameters u)
{
    vec3 world = position + u.Origin;
    vec2 projected = cosmic_board_projection(world, u.Eye);
    vec3 force = cosmic_impulse(projected, u.Source0, u.Drive0)
        + cosmic_impulse(projected, u.Source1, u.Drive1)
        + cosmic_impulse(projected, u.Source2, u.Drive2)
        + cosmic_impulse(projected, u.Source3, u.Drive3);
    float depth = (u.Eye.z - world.z) / max(u.Eye.z, 1.0);
    return force * vec3(depth, depth, 1.0);
}

ChibiParticleState reaction_advect(ChibiParticleState state, ChibiParticleInput p, ChibiParameters u,
    float birth, float response)
{
    float seconds = min(p.delta, max(0.0, p.time - birth)) * (1.0 - u.Quiet);
    vec3 world = state.position + u.Origin;
    vec3 current = cosmic_curl(world * 0.006, u.Current.x);
    vec3 target = current * (45.0 + u.Current.z * 30.0) + vec3(u.Current.y * 28.0, 20.0, -8.0);
    state.velocity += (target - state.velocity) * (1.0 - exp(-seconds * response));
    state.velocity += reaction_force(state.position, u) * seconds * 2.4;
    state.velocity.z -= max(0.0, world.z - 560.0) * seconds * 2.0;
    state.velocity.z += max(0.0, -1250.0 - world.z) * seconds * 0.7;
    state.velocity *= min(1.0, 420.0 / max(length(state.velocity), 1.0));
    state.position += state.velocity * seconds;
    state.age += p.delta;
    return state;
}

#endif
