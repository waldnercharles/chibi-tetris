#include "ReactionCommon.glsl"

// Fracture, opaque debris, microembers and energy share a captured row front.
ChibiParticle release(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = reaction_random(p.index, p.seed);
    float quantile = reaction_quantile(p.index, p.seed);
    vec4 layers = vec4(
        reaction_pattern(u.Pattern, 5.0), reaction_pattern(u.Pattern, 9.0),
        reaction_pattern(u.Pattern, 10.0), reaction_pattern(u.Pattern, 11.0)
    );
    vec3 source = vec3(
        (fract(p.index * 0.1) * 10.0 - 4.5) * 24.0 + (random.x - 0.5) * 18.0,
        (random.y - 0.5) * 18.0,
        u.Point0.z + 0.65 + (random.z - 0.5) * 0.6
    );
    float birth = mix(
        reaction_birth(u.Life * (0.26 - (layers.z + layers.w) * 0.16), quantile),
        (abs(source.x) / 118.0 * 0.04 + quantile * 0.008) * u.Life / 0.24,
        layers.x
    );
    float duration = u.Life * (0.56 + random.z * 0.16 +
        (layers.z + layers.w) * (0.16 + random.z * 0.02));
    float elapsed = max(0.0, p.time - u.Delay - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float envelope = smoothstep(0.0, min(0.016, duration * 0.16), elapsed) *
        (1.0 - smoothstep(duration * 0.28, duration, elapsed)) * u.Fade;
    float motion = elapsed * (1.0 - u.Quiet);

    // Exact exponential drag and gravity, plus bounded analytic wind.
    float drag_rate = 4.5 - layers.y * 2.7 - layers.z * 3.6 - layers.w * 3.4;
    float drag_time = reaction_drag(drag_rate, motion);
    float gravity_time = (motion - drag_time) / drag_rate;
    vec3 curl = vec3(
        sin(random.z * 6.283185 + motion * 1.3) - sin(random.z * 6.283185),
        cos(random.x * 6.283185 + motion) - cos(random.x * 6.283185),
        sin(random.y * 6.283185 + motion * 0.8) - sin(random.y * 6.283185)
    ) * (1.0 - exp(-1.6 * motion)) * 18.0;
    vec3 fracture = source + vec3(
        (random.x - 0.5) * 60.0, (random.y - 0.5) * 45.0, 16.0 + random.z * 24.0
    ) * drag_time;
    vec3 debris = source + vec3(
        (random.x - 0.5) * 440.0, 50.0 + random.y * 100.0, 20.0 + random.z * 45.0
    ) * drag_time - vec3(0.0, 250.0 * gravity_time, 0.0);
    vec3 embers = source + vec3(
        (random.x - 0.5) * 105.0, 12.0 + (random.y - 0.5) * 70.0,
        10.0 + random.z * 32.0
    ) * drag_time - vec3(0.0, 7.0 * gravity_time, 0.0) + curl;
    float angle = p.index * 6.283185 / max(p.count, 1.0) + (random.z - 0.5) * 0.018;
    vec3 energy = vec3(
        cos(angle) * (118.0 + 180.0 * drag_time),
        sin(angle) * (10.0 + 90.0 * drag_time),
        u.Point0.z + 0.8 + 30.0 * drag_time + (random.y - 0.5) * 2.0
    ) + curl * 0.35;

    float fleck = step(0.78, random.y);
    float wake = step(0.48, random.y) * (1.0 - fleck);
    float clear_size = (
        layers.x * (1.25 + random.z * 1.5) + layers.y * (2.2 + random.z * 3.6) +
        layers.z * (0.45 + random.z * 0.72 + wake * 0.35) +
        layers.w * (0.55 + random.z * 0.7)
    ) * (1.0 - phase * 0.24) * envelope;
    // Cooling uses seconds, so a long Life does not prolong the white front.
    vec3 color = mix(
        vec3(1.8, 2.25, 2.65),
        mix(vec3(0.18, 0.82, 1.1), vec3(1.4, 0.42, 0.1), step(0.78, random.y)),
        smoothstep(0.0, 0.55, elapsed)
    ) * (1.0 + layers.x * 0.5 + layers.w * 0.15);
    vec4 clear_color = vec4(
        mix(color, mix(vec3(0.12, 0.48, 0.62), vec3(0.55, 0.18, 0.08), random.x), layers.y),
        envelope * (layers.x * 0.8 + layers.y +
            (layers.z * (0.34 + random.z * 0.26) * (1.0 - wake * 0.72) +
                layers.w * 0.58) * (0.3 + 0.7 * exp(-2.8 * elapsed)))
    );

    ChibiParticle result;
    result.position = fracture * layers.x + debris * layers.y +
        embers * layers.z + energy * layers.w;
    result.size = mix(clear_size, envelope, u.Quiet);
    result.color = mix(clear_color, vec4(0.4, 0.8, 1.0, envelope * 0.4), u.Quiet);
    result.scale = mix(vec3(1.0), vec3(2.2, 0.38, 0.6), layers.x * (1.0 - u.Quiet)) *
        mix(vec3(1.0), vec3(
            1.2 + random.x * 0.8, 0.5 + random.y * 0.55, 0.65 + random.z * 0.55
        ), layers.y * (1.0 - u.Quiet));
    result.rotation = vec3(random.x - 0.5, random.y - 0.5, random.z - 0.5) *
        (2.2 + motion * 8.0) * layers.y * (1.0 - u.Quiet);
    result.lighting = (layers.y * 0.85 + layers.z * 0.35) * (1.0 - u.Quiet);
    return result;
}
