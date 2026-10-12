#include "CosmicForms.glsl"

// The finer field follows the same forces and keeps its own persistent state.
// One quarter stays distant, unstructured dust at every quality level.
ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    vec3 identity = cosmic_random(p.index, p.seed, u.Variation);
    if (identity.z >= 0.25) return cosmic_advect(p, u, state);
    if (p.reset) {
        state.position = vec3((identity.x - 0.5) * 6500.0, (identity.y - 0.5) * 3800.0,
            -1400.0 - identity.z * 8800.0);
        state.velocity = vec3(0.0);
        state.age = 0.0;
        return state;
    }
    vec3 drift = cosmic_curl(state.position * 0.0011, u.Flow * 0.35) * 4.0;
    state.velocity += (drift - state.velocity) * (1.0 - exp(-p.delta * 0.7));
    state.position += state.velocity * p.delta;
    state.age += p.delta;
    return state;
}

ChibiParticle stars(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    vec3 character = clamp(u.Character, 0.0, 1.0);
    float dust = 1.0 - step(0.25, random.z);
    float life = cosmic_life(random);
    float envelope = mix(smoothstep(0.0, 0.65, p.age) *
        (1.0 - smoothstep(life - 2.2, life, p.age)), 1.0, dust);
    float opacity = mix(0.045 + u.Energy * 0.10 + u.Beat * 0.10, 0.1 + random.y * 0.24, dust);
    float brightness = mix(0.36 + u.Energy * 0.55 + u.Beat * 0.65, 0.22 + u.Treble * 0.12, dust);
    float size = mix(0.8 + random.y * 0.2, 0.18 + 0.24 * pow(random.y, 7.0), dust);
    size *= 1.0 + character.x * 0.2 - character.z * 0.35;
    float action = cosmic_event_light(p.position, u);
    float wave = particle_wave_light(p.position, u) * 0.25;
    float quiet = cosmic_board_quiet(cosmic_board_projection(p.position, u.Eye), u.Duet);
    float filament = (weights.y * character.y * 3.2 + weights.z * character.z * 2.0) * (1.0 - dust)
        * smoothstep(0.82, 0.97, random.x) * clamp(length(p.velocity) / 180.0, 0.0, 1.0);
    ChibiParticle result = chibi_particle();
    result.position = p.position;
    result.size = size * (1.0 + action * 0.2 + wave * 1.2) * sqrt(envelope);
    result.color = vec4(cosmic_tint(random, p.position, weights, u) * (brightness + action * 0.8)
        + vec3(0.3, 1.6, 2.2) * wave,
        opacity * quiet * envelope * (1.0 + wave * 2.4) * mix(cosmic_density(random, weights), 1.0, dust));
    result.scale = vec3(1.0 + filament, 1.0 / (1.0 + filament * 0.3), 1.0);
    result.rotation = vec3(0.0, 0.0, cosmic_heading(p.velocity));
    result.lighting = 0.1;
    return result;
}
