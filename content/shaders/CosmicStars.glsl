#include "CosmicCommon.glsl"

// A sparse, quietly drifting volume behind the simulated cloud. These grains
// provide distant depth, without bright blinking stars or repeated ribbon shapes.
ChibiParticle stars(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    vec3 position = vec3((random.x - 0.5) * 6500.0, (random.y - 0.5) * 3800.0,
        -1400.0 - random.z * 2200.0);
    position += cosmic_curl(position * 0.0011, u.Flow * 0.35) * 65.0;
    float quiet = cosmic_board_quiet(cosmic_board_projection(position, u.Eye), u.Duet);
    ChibiParticle result = chibi_particle();
    result.position = position;
    result.size = 0.18 + 0.24 * pow(random.z, 7.0);
    result.color = vec4(cosmic_tint(random, position, weights, u) * (0.18 + u.Treble * 0.06),
        (0.1 + 0.24 * random.y) * quiet);
    result.scale = vec3(1.0);
    result.rotation = random * COSMIC_TAU;
    result.lighting = 0.1;
    return result;
}
