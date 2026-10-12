// Shared event identities and deterministic particle samples.
#ifndef REACTION_COMMON_GLSL
#define REACTION_COMMON_GLSL

#include "ParticleFlow.glsl"

vec3 reaction_random(float index, float seed)
{
    return particle_samples(uint(index) ^ particle_hash(uint(seed)));
}

float reaction_quantile(float index, float seed)
{
    return float(particle_hash(uint(index) ^ particle_hash(uint(seed) + 71u)) >> 8u) / 16777216.0;
}

// All layers of one captured action share their density, spread and cooling.
// Each new event has a new identity even before the field pool wraps around.
vec3 reaction_event(float seed)
{
    return particle_samples((uint(seed) >> 4u) + 997u);
}

float reaction_population(float index, float seed, vec3 event)
{
    // Density is independent of birth time and position along a drop column.
    float density_sample = float(particle_hash(uint(index) ^ particle_hash(uint(seed) + 2843u)) >> 8u) / 16777216.0;
    return 1.0 - step(0.58 + event.x * 0.42, density_sample);
}

float reaction_duration(float life, vec3 random, vec3 event)
{
    return life * (0.70 + event.z * 0.16 + random.z * 0.12);
}

ChibiParticle reaction_illuminate(ChibiParticle particle, ChibiParameters u)
{
    float wave = particle_wave_light(particle.position + u.Origin, u) * (1.0 - u.Quiet);
    particle.color.rgb += vec3(0.24, 1.65, 2.0) * wave;
    particle.color.a *= 1.0 + wave * 0.65;
    particle.size *= 1.0 + wave * 0.24;
    return particle;
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
    vec3 event = reaction_event(p.seed);
    vec3 current = cosmic_curl(world * (0.004 + event.y * 0.003), u.Current.x);
    // An event keeps its broad initial arc while the live common current bends
    // it. Neighboring schools still exchange the same four input forces.
    float swirl = (event.x - 0.5) * 2.0;
    vec3 bend = vec3(-state.velocity.y, state.velocity.x, state.velocity.x * 0.2);
    state.velocity += bend * swirl * seconds * 0.62 * exp(-max(0.0, p.time - birth) * 0.65);
    vec3 target = current * (45.0 + u.Current.z * 30.0) + vec3(u.Current.y * 28.0, 20.0, -8.0);
    state.velocity += (target - state.velocity) * (1.0 - exp(-seconds * response));
    float force_response = u.Pattern == 13.0 ? 1.3 : 2.4;
    state.velocity += reaction_force(state.position, u) * seconds * force_response;
    state.velocity.z -= max(0.0, world.z - 560.0) * seconds * 2.0;
    state.velocity.z += max(0.0, -1250.0 - world.z) * seconds * 0.7;
    state.velocity *= min(1.0, 680.0 / max(length(state.velocity), 1.0));
    state.position += state.velocity * seconds;
    state.age += p.delta;
    return state;
}

#endif
