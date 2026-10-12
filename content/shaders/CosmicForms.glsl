#ifndef COSMIC_FORMS_GLSL
#define COSMIC_FORMS_GLSL

#include "CosmicCommon.glsl"

float cosmic_life(vec3 identity)
{
    return 12.0 + identity.x * 14.0;
}

float cosmic_band(float cohort, ChibiParameters u)
{
    if (cohort < 2.0) return mix(u.Bass, u.Mid, cohort * 0.5);
    return mix(u.Mid, u.Treble, (cohort - 2.0) * 0.5);
}

// Five filled populations share the same flow but occupy different moving
// volumes. A volume confines only overflow; no grain has a prescribed path.
void cosmic_volume(float cohort, ChibiParameters u, out vec3 center, out vec3 extent, out vec3 axis)
{
    vec4 weights = cosmic_stage_weights(u.Style);
    vec3 seed = cosmic_random(cohort + 71.0, 359.0, u.Variation);
    float phase = seed.x * COSMIC_TAU + u.Flow * (0.045 + seed.y * 0.035);
    vec3 knot = vec3(cos(phase), sin(phase * 1.3 + seed.z * 4.0), sin(phase * 0.7 + seed.y * 5.0));
    vec3 offset = vec3(340.0, 230.0, 180.0) * weights.x
        + vec3(640.0, 310.0, 260.0) * weights.y
        + vec3(550.0, 330.0, 190.0) * weights.z
        + vec3(780.0, 520.0, 380.0) * weights.w;
    center = vec3(0.0, 20.0, -1780.0) + knot * offset;
    extent = vec3(940.0, 700.0, 520.0) * weights.x
        + vec3(1480.0, 430.0, 470.0) * weights.y
        + vec3(1460.0, 760.0, 310.0) * weights.z
        + vec3(930.0, 790.0, 630.0) * weights.w;
    vec3 character = clamp(u.Character, 0.0, 1.0);
    extent *= vec3(1.0 + character.y * 0.30,
        1.0 - character.y * 0.22 + character.z * 0.16,
        1.0 + character.x * 0.28 - character.z * 0.24);
    float section = clamp(u.Section, 0.0, 1.0);
    extent *= 1.0 + section * 0.22;
    extent.x *= 1.0 + u.Duet * 0.55;
    axis = normalize(vec3((seed.x - 0.5) * 0.65, (seed.y - 0.5) * 0.55, 1.0));
}

ChibiParticleState cosmic_birth(ChibiParticleInput p, ChibiParameters u, vec3 identity)
{
    float cohort = floor(identity.z * 5.0);
    vec3 center, extent, axis;
    cosmic_volume(cohort, u, center, extent, axis);
    vec4 weights = cosmic_stage_weights(u.Style);
    vec3 random = cosmic_random(p.index + 31.0, p.seed + 19.0, u.Variation + 51.0);
    vec3 ball = cosmic_ball(random);
    // Short core routes coexist with long outer routes. Their density comes
    // from volume and residence time, never a sampled curve's endpoints.
    float outer = smoothstep(0.64, 0.94, identity.y);
    vec3 pulse = ball * vec3(0.73, 0.73, 0.78);
    vec3 stream = ball * vec3(0.44 + outer * 0.40, 0.24 + outer * 0.33, 0.42 + outer * 0.36);
    vec3 sheet = ball * vec3(0.84, 0.64, 0.22);
    sheet.z += sin(ball.x * 3.4 + cohort * 1.7) * 0.34 + sin(ball.y * 4.1 + cohort) * 0.20;
    vec3 cloud = ball * (0.30 + outer * 0.58);
    vec3 shape = pulse * weights.x + stream * weights.y + sheet * weights.z + cloud * weights.w;
    ChibiParticleState state;
    state.position = center + shape * extent;
    state.velocity = cosmic_curl(state.position * 0.0017, u.Flow) * 45.0;
    state.age = 0.0;
    if (p.reset) {
        state.age = cosmic_random(p.index, p.seed + 613.0, u.Variation + 997.0).y * cosmic_life(identity);
    }
    return state;
}

ChibiParticleState cosmic_advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    vec3 identity = cosmic_random(p.index, p.seed, u.Variation);
    if (p.reset || state.age >= cosmic_life(identity)) return cosmic_birth(p, u, identity);
    vec4 weights = cosmic_stage_weights(u.Style);
    vec3 character = clamp(u.Character, 0.0, 1.0);
    float cohort = floor(identity.z * 5.0);
    vec3 center, extent, axis;
    cosmic_volume(cohort, u, center, extent, axis);
    vec3 relative = state.position - center;
    vec3 local = relative / extent;
    float band = cosmic_band(cohort, u);
    float music = clamp(u.Energy * 0.45 + band * 0.85, 0.0, 1.0);
    float rhythm = sin(u.Pulse - cohort * 0.45) * music;
    float section = clamp(u.Section, 0.0, 1.0);
    float phase = u.Flow + u.Wind * 0.24;
    vec3 broad = cosmic_curl(state.position * (0.0015 + section * 0.00035), phase);
    vec3 fine = cosmic_curl(state.position * (0.0054 + character.z * 0.0025), phase * 1.23 + 9.0);
    vec3 orbit = cross(axis, local) * 125.0 + broad * 65.0;
    // Anisotropic circulation carries filled knots into long streams. The
    // shared curl bends and separates their edges without locking a centerline.
    vec3 stream = vec3(-local.y * 175.0, local.x * 34.0, local.y * 26.0)
        + broad * vec3(100.0, 65.0, 55.0) + fine * 8.0;
    vec3 sheet = vec3(-local.y * 90.0, local.x * 48.0, 0.0)
        + broad * vec3(95.0, 130.0, 38.0) + fine * vec3(32.0, 48.0, 23.0);
    vec3 cloud = broad * 135.0 + fine * 125.0 + cross(axis, local) * 55.0;
    vec3 target = orbit * weights.x + stream * weights.y + sheet * weights.z + cloud * weights.w;
    target *= (0.45 + music * 0.85 + u.Reaction * 0.30) * (1.0 + rhythm * 0.30);
    target += fine * section * 30.0;

    float radius = length(local);
    vec3 normal = local / max(radius, 0.001);
    // Smooth pressure at the boundary keeps the depth volume bounded. Inside,
    // momentum and shared flow remain free to make uneven knots and openings.
    vec3 force = -normal * max(0.0, radius - 0.88) * 420.0;
    force += cosmic_event_force(state.position, u) * 5.2;
    force += normal * u.Beat * (120.0 + character.x * 340.0) * (1.0 - cohort * 0.10);
    force.z += sin(u.Pulse + cohort) * band * character.x * 120.0;
    force.z -= max(0.0, state.position.z + 650.0) * 1.4;
    force.z += max(0.0, -3400.0 - state.position.z) * 0.7;
    float response = 1.0 - exp(-p.delta * (0.85 + music * 1.8));
    state.velocity += (target - state.velocity) * response + force * p.delta;
    state.velocity *= min(1.0, 640.0 / max(length(state.velocity), 1.0));
    state.position += state.velocity * p.delta;
    state.age += p.delta;
    return state;
}

// Population brightness varies independently of position, leaving granular
// edges. The volume births and flow create the large dark openings.
float cosmic_density(vec3 identity, vec4 weights)
{
    float core = 1.0 - smoothstep(0.45, 0.94, identity.y);
    return mix(0.36, 1.0, core) * dot(weights, vec4(0.84, 0.92, 0.75, 1.0));
}

#endif
