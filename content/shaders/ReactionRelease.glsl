#include "ReactionCommon.glsl"

vec3 release_source(float index, vec3 random, ChibiParameters u)
{
    return vec3((mod(index, 10.0) - 4.5) * 24.0 + (random.x - 0.5) * 18.0,
        (random.y - 0.5) * 18.0, u.Point0.z + 0.65 + (random.z - 0.5) * 0.6);
}

float release_birth(float source_x, float quantile, ChibiParameters u)
{
    float fracture = reaction_pattern(u.Pattern, 5.0);
    float ember = reaction_pattern(u.Pattern, 10.0);
    float tail = ember + reaction_pattern(u.Pattern, 11.0);
    float birth = mix(reaction_birth(u.Life * (0.26 - tail * 0.16), quantile),
        (abs(source_x) / 118.0 * 0.04 + quantile * 0.008) * u.Life / 0.24, fracture);
    return mix(birth, quantile * (0.018 + ember * 0.085), u.Loss);
}

// Fine clear grains and their outer fan join the same live current as drops.
ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    if (u.Pattern != 10.0 && u.Pattern != 11.0) {
        state.position = vec3(0.0);
        state.velocity = vec3(0.0);
        state.age = p.time;
        return state;
    }
    vec3 random = reaction_random(p.index, p.seed);
    vec3 source = release_source(p.index, random, u);
    float birth = u.Delay + release_birth(source.x, reaction_quantile(p.index, p.seed), u);
    if (p.reset) {
        state.position = source;
        float depth = mix(-100.0 - random.x * 120.0, 155.0 + random.x * 130.0, step(0.65, random.z));
        state.velocity = vec3((random.x - 0.5) * 340.0,
            22.0 + (random.y - 0.5) * 175.0, depth);
        if (u.Pattern == 11.0) state.velocity.xy *= 1.45;
        if (u.Loss > 0.5) state.velocity *= 0.28;
        float elapsed = max(0.0, p.time - birth) * (1.0 - u.Quiet);
        state.position += state.velocity * reaction_drag(0.45, elapsed);
        state.age = p.time;
        return state;
    }
    return reaction_advect(state, p, u, birth, 0.3);
}

// Fracture, opaque debris, microembers and energy share a captured row front.
ChibiParticle release(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = reaction_random(p.index, p.seed);
    float quantile = reaction_quantile(p.index, p.seed);
    vec4 layers = vec4(
        reaction_pattern(u.Pattern, 5.0), reaction_pattern(u.Pattern, 9.0),
        reaction_pattern(u.Pattern, 10.0), reaction_pattern(u.Pattern, 11.0)
    );
    vec3 source = release_source(p.index, random, u);
    float birth = release_birth(source.x, quantile, u);
    float duration = u.Life * (0.56 + random.z * 0.16 +
        (layers.z + layers.w) * (0.16 + random.z * 0.02));
    // Loss adds a sparse ember tail behind the departing original block meshes.
    duration = mix(duration, u.Life * (0.78 + random.z * 0.14), u.Loss);
    float elapsed = max(0.0, p.time - u.Delay - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float envelope = smoothstep(0.0, min(0.016, duration * 0.16), elapsed) *
        (1.0 - smoothstep(duration * 0.28, duration, elapsed)) * u.Fade;
    float loss_envelope = smoothstep(0.0, 0.045, elapsed) *
        (1.0 - smoothstep(duration * 0.42, duration, elapsed)) * u.Fade;
    envelope = mix(envelope, loss_envelope, u.Loss);
    int cell = int(mod(p.index, 10.0));
    float occupied = float((int(u.RowMask) >> cell) & 1);
    envelope *= occupied;
    float motion = elapsed * (1.0 - u.Quiet);

    // Heavy fragments use exact exponential drag and gravity.
    float drag_rate = 4.5 - layers.y * 2.7;
    float drag_time = reaction_drag(drag_rate, motion);
    float gravity_time = (motion - drag_time) / drag_rate;
    vec3 fracture = source + vec3(
        (random.x - 0.5) * 60.0, (random.y - 0.5) * 45.0, 16.0 + random.z * 24.0
    ) * drag_time;
    vec3 debris = source + vec3(
        (random.x - 0.5) * 440.0, 50.0 + random.y * 100.0, 20.0 + random.z * 45.0
    ) * drag_time - vec3(0.0, 250.0 * gravity_time, 0.0);

    float fleck = step(0.78, random.y);
    float wake = step(0.48, random.y) * (1.0 - fleck);
    float clear_size = (
        layers.x * (1.25 + random.z * 1.5) + layers.y * (2.2 + random.z * 3.6) +
        layers.z * (0.52 + random.z * 0.9 + fleck * 1.1 + wake * 0.35) +
        layers.w * (0.55 + random.z * 0.7)
    ) * (1.0 - phase * 0.24) * envelope;
    float loss_size = (0.55 + random.z * 0.7) * (1.0 - phase * 0.3) * envelope;
    clear_size = mix(clear_size, loss_size, u.Loss);
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
                layers.w * 0.58) * (0.7 + 0.3 * exp(-2.8 * elapsed)))
    );

    vec3 loss_color = mix(vec3(0.15, 0.4, 0.52), vec3(0.35, 0.55, 0.7), random.x);
    loss_color = mix(loss_color, vec3(0.4, 0.9, 1.2), layers.z * exp(-elapsed * 10.0));
    clear_color.rgb = mix(clear_color.rgb, loss_color, u.Loss);
    clear_color.a = mix(clear_color.a, envelope * (0.12 + 0.12 * exp(-elapsed * 3.0)), u.Loss);

    ChibiParticle result;
    result.position = fracture * layers.x + debris * layers.y;
    if (u.Pattern == 10.0 || u.Pattern == 11.0) result.position = p.position;
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
