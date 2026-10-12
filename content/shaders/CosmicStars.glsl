#include "CosmicCommon.glsl"

ChibiParticle stars(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    float phase = cosmic_phase(u);
    vec3 position = vec3(
        (random.x - 0.5) * 6500.0 + sin(phase * 0.04 + random.z * COSMIC_TAU) * 95.0,
        (random.y - 0.5) * 3950.0 + cos(phase * 0.035 + random.x * COSMIC_TAU) * 70.0,
        -950.0 - random.z * 2900.0 + sin(phase * 0.025 + random.y * COSMIC_TAU) * 90.0);
    position = cosmic_music_motion(position, u, 0.35);
    position = cosmic_event_motion(position, u, 0.35);
    float size = (0.22 + 1.8 * pow(random.z, 9.0)) * max(0.75, (u.Eye.z - position.z) / 2300.0);
    ChibiParticle result = chibi_particle();
    result.position = position;
    result.size = cosmic_music_size(size, random, u);
    result.color = cosmic_color(random, position, weights, 1.0, u);
    result.scale = vec3(1.0 + 1.8 * pow(random.z, 8.0), 0.7, 0.8)
        * vec3(1.0 + u.Treble * 0.45, 1.0 + u.Beat * 0.10, 1.0);
    result.rotation = random * COSMIC_TAU;
    result.lighting = 0.12;
    return result;
}
