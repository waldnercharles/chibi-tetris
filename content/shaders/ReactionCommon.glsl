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

// Independent event channels vary the whole school, not its identity per frame.
vec3 reaction_profile(float seed, uint channel)
{
    return particle_samples((uint(seed) >> 4u) ^ particle_hash(channel));
}

float reaction_heading(vec3 velocity)
{
    if (dot(velocity.xy, velocity.xy) < 0.0001) return 0.0;
    return atan(velocity.y, velocity.x);
}

float reaction_population(float index, float seed, vec3 event)
{
    // Density is independent of birth time and position along a drop column.
    float density_sample = float(particle_hash(uint(index) ^ particle_hash(uint(seed) + 2843u)) >> 8u) / 16777216.0;
    return 1.0 - step(0.58 + event.x * 0.42, density_sample);
}

// Character is captured once at birth, so a new section cannot reclassify an
// existing trail or change its pool lifetime. The visible cooling stays inside Life.
float reaction_cooling(vec3 character)
{
    return 1.0 - character.x * 0.22 + character.y * 0.14 - character.z * 0.08;
}

float reaction_duration(float life, vec3 random, vec3 event, vec3 character, float birth)
{
    float duration = life * (0.70 + event.z * 0.16 + random.z * 0.12) * reaction_cooling(character);
    return min(duration, max(0.001, life - birth - 0.015));
}

vec3 reaction_launch(vec3 velocity, vec2 direction, vec3 random, vec3 event, vec3 character)
{
    vec2 axis = direction / max(length(direction), 0.001);
    vec2 cross_axis = vec2(-axis.y, axis.x);
    float speed = max(length(velocity.xy), 1.0);
    vec2 radial = vec2(random.x - 0.5, random.y - 0.5);
    radial = axis * 0.85 + radial * 1.5;
    radial /= max(length(radial), 0.001);
    vec3 pulse = vec3(radial * speed * 1.25, velocity.z * 1.45);
    float coil = sin(random.x * 6.283185 + event.z * 6.283185);
    vec3 ribbon = vec3((axis + cross_axis * coil * 0.16) * speed, velocity.z * 0.62);
    float branch = (floor(random.x * 5.0) - 2.0) * 0.34;
    vec3 filament = vec3((axis * cos(branch) + cross_axis * sin(branch)) * speed *
        (0.78 + random.y * 0.46), velocity.z * (0.7 + abs(branch)));
    float total = dot(character, vec3(1.0));
    vec3 weights = character / max(1.0, total);
    vec3 shaped = velocity * (1.0 - min(1.0, total)) +
        pulse * weights.x + ribbon * weights.y + filament * weights.z;
    float complexity = clamp((total - 1.35) * 0.8, 0.0, 0.7);
    shaped += cosmic_curl(random * 11.0, event.z * 7.0) * speed * complexity;
    float angle = (event.x - 0.5) * 0.8;
    shaped.xy = mat2(cos(angle), sin(angle), -sin(angle), cos(angle)) * shaped.xy;
    return shaped * vec3(0.66 + event.y * 0.86, 0.72 + event.z * 0.68, 0.62 + event.z * 0.86);
}

ChibiParticle reaction_illuminate(ChibiParticle particle, ChibiParameters u, vec3 random, float seed)
{
    vec3 style = reaction_profile(seed, 421u);
    vec3 shade = reaction_profile(seed, 1579u);
    float hue = shade.x * 3.0;
    vec3 palette = mix(vec3(0.14, 1.0, 1.18), vec3(0.3, 0.6, 1.35), min(hue, 1.0));
    if (hue > 1.0) palette = mix(vec3(0.3, 0.6, 1.35), vec3(0.72, 0.43, 1.24), min(hue - 1.0, 1.0));
    if (hue > 2.0) palette = mix(vec3(0.72, 0.43, 1.24), vec3(1.12, 1.0, 0.72), hue - 2.0);
    float peak = max(particle.color.r, max(particle.color.g, particle.color.b));
    float tint_amount = u.Pattern == 7.0 ? 0.08 : 0.30 + shade.y * 0.2;
    particle.color.rgb = mix(particle.color.rgb, palette * peak, tint_amount) * (0.72 + style.z * 0.72);
    particle.color.a *= 0.70 + shade.z * 0.5;
    particle.size *= 0.68 + style.y * 0.80;
    vec3 character = clamp(u.Character, 0.0, 1.0) * (1.0 - u.Timing.z);
    float beam = 1.0 - step(0.5, abs(u.Pattern - 8.0));
    float fine = 1.0 - smoothstep(0.88, 0.96, random.z);
    float streak = smoothstep(0.78, 0.95, random.x) * fine;
    float elongation = (character.y * 2.8 + character.z * 1.7) * streak * (1.0 - beam);
    particle.scale *= vec3(1.0 + elongation, 1.0 / (1.0 + elongation * 0.45), 1.0);
    particle.size *= 1.0 + character.x * 0.26 - character.z * 0.28;
    vec3 tint = vec3(1.0) + character.x * vec3(0.12, 0.02, -0.04)
        + character.y * vec3(-0.12, 0.10, 0.15) + character.z * vec3(0.12, 0.08, 0.20);
    particle.color.rgb *= tint;
    float wave = particle_wave_light(particle.position + u.Origin, u) * (1.0 - u.Timing.z);
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
    float seconds = min(p.delta, max(0.0, p.time - birth)) * (1.0 - u.Timing.z);
    vec3 world = state.position + u.Origin;
    vec3 event = reaction_event(p.seed);
    vec3 character = clamp(u.Character, 0.0, 1.0);
    float complexity = clamp(dot(character, vec3(1.0)) - 1.35, 0.0, 1.0);
    vec3 current = cosmic_curl(world * (0.004 + event.y * 0.003 + character.z * 0.006), u.Current.x);
    // An event keeps its broad initial arc while the live common current bends
    // it. Neighboring schools still exchange the same four input forces.
    vec3 motion = reaction_profile(p.seed, 773u);
    float swirl = (motion.x < 0.5 ? -1.0 : 1.0) * (0.4 + motion.y * 1.2);
    vec3 bend = vec3(-state.velocity.y, state.velocity.x, state.velocity.x * 0.2);
    state.velocity += bend * swirl * seconds * (0.62 + character.y * 1.5) *
        exp(-max(0.0, p.time - birth) * (0.65 - character.y * 0.42));
    vec3 target = current * (45.0 + u.Current.z * 30.0 + complexity * 115.0) + vec3(u.Current.y * 28.0 + (motion.z - 0.5) * 60.0, 5.0 + motion.y * 35.0, -8.0);
    state.velocity += (target - state.velocity) * (1.0 - exp(-seconds * response * (1.0 - character.y * 0.55 + character.z * 0.5)));
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
