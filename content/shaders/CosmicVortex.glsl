#include "CosmicForms.glsl"

ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    return cosmic_advect(p, u, state);
}

ChibiParticle vortex(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    float life = cosmic_life(random);
    float envelope = smoothstep(0.0, 0.65, p.age) * (1.0 - smoothstep(life - 2.2, life, p.age));
    float quiet = cosmic_board_quiet(cosmic_board_projection(p.position, u.Eye), u.Duet);
    quiet *= 1.0 - smoothstep(-420.0, -180.0, p.position.z);
    float foreground = 1.0 - smoothstep(1000.0, 3100.0, -p.position.z);
    vec3 character = clamp(u.Character, 0.0, 1.0);
    float size = (0.42 + pow(random.z, 4.0) * 0.50) * (1.0 + character.x * 0.25 - character.z * 0.38);
    float accent = pow(random.x, 12.0);
    float speed = length(p.velocity);
    float wake = cosmic_event_light(p.position, u);
    float wave = particle_wave_light(p.position, u) * 0.5;
    float activity = clamp(u.Energy * 0.25 + u.Beat * 0.42 + wake * 0.85, 0.0, 1.0);
    float light = 0.22 + foreground * 0.10 + activity * 1.2 + u.Treble * accent * 0.35;
    ChibiParticle result = chibi_particle();
    result.position = p.position;
    result.size = size * (0.86 + u.Treble * accent * 0.45) * (1.0 + wave * 2.0) * sqrt(envelope);
    vec3 tint = cosmic_tint(random, p.position, weights, u);
    tint = mix(tint, vec3(0.50, 0.95, 1.12), wake * 0.32);
    result.color = vec4(tint * light + vec3(0.18, 1.8, 2.4) * wave,
        envelope * quiet * cosmic_density(random, weights) * (0.08 + 0.22 * random.y) * (0.45 + activity * 1.5 + wave * 2.2));
    // Most grains are round solids. Only rare fast foreground grains stretch,
    // in their actual direction of travel, retaining a sharp visible center.
    float streak = accent * foreground * clamp(speed / 220.0, 0.0, 2.0);
    float filament = (weights.y * (1.1 + character.y * 2.8) + weights.z * character.z * 1.7)
        * smoothstep(0.82, 0.97, random.x) * clamp(speed / 180.0, 0.0, 1.0);
    result.scale = vec3(1.0 + streak * 2.0 + filament, 0.9 / (1.0 + filament * 0.28), 0.9);
    result.rotation = vec3(0.0, 0.0, cosmic_heading(p.velocity));
    result.lighting = 0.18;
    return result;
}
