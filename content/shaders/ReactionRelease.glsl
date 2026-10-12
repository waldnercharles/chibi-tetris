#include "ReactionCommon.glsl"

vec3 release_source(float index, vec3 random, ChibiParameters u)
{
    return vec3((mod(index, 10.0) - 4.5) * 24.0 + (random.x - 0.5) * 18.0,
        (random.y - 0.5) * 18.0, u.Point0.z + 0.65 + (random.z - 0.5) * 0.6);
}

float release_birth(float source_x, float quantile, ChibiParameters u, float seed)
{
    if (u.Timing.z > 0.5) return 0.0;
    vec3 sweep = reaction_profile(seed, 1847u);
    float origin = (sweep.x - 0.5) * 210.0;
    float front = clamp((source_x + 118.0) / 236.0, 0.0, 1.0);
    if (sweep.z > 0.3333) front = 1.0 - front;
    if (sweep.z > 0.6667) front = abs(source_x - origin) / 236.0;
    float birth = front * (0.07 + sweep.y * 0.10) + quantile * (0.02 + sweep.x * 0.04);
    float ember = reaction_pattern(u.Pattern, 10.0);
    return min(u.Life * 0.4, mix(birth, quantile * (0.018 + ember * 0.085), u.Loss));
}

vec3 release_velocity(vec3 source, vec3 random, ChibiParameters u, float seed)
{
    vec3 event = reaction_event(seed);
    vec3 shape = reaction_profile(seed, 211u);
    float origin = (shape.x - 0.5) * 160.0;
    float side = source.x < origin ? -1.0 : 1.0;
    float reach = min(1.0, abs(source.x - origin) / 170.0);
    vec2 fan = vec2(side * (100.0 + reach * 150.0 + random.x * 70.0),
        45.0 + random.y * 95.0 + (shape.y - 0.5) * 130.0);
    if (shape.z > 0.3333) {
        float drift = shape.x < 0.5 ? -1.0 : 1.0;
        fan = vec2(drift * (125.0 + random.x * 145.0),
            (source.x - origin) * (0.4 + shape.y * 0.8) + 35.0 + random.y * 80.0);
    }
    if (shape.z > 0.6667) {
        fan = vec2(side * (130.0 + random.x * 150.0),
            side * (shape.y - 0.5) * 260.0 + 20.0 + random.y * 95.0);
    }
    float forward = step(0.58 + shape.y * 0.23, random.z);
    float depth = mix(-120.0 - random.y * 230.0, 170.0 + random.y * 190.0, forward);
    vec3 velocity = vec3(fan, depth) * (0.75 + event.y * 0.65);
    if (u.Pattern == 11.0) velocity.xy *= 1.18;
    velocity = reaction_launch(velocity, fan, random, event, clamp(u.Character, 0.0, 1.0));
    // Large near grains keep a visible center beside the well for longer.
    velocity.xy *= 1.0 - smoothstep(0.94, 0.995, random.z) * 0.34;
    return velocity;
}

float release_distance(float elapsed, float gather, float expansion)
{
    float phase = clamp((elapsed - gather) / expansion, 0.0, 1.0);
    return expansion * (phase * phase * phase - 0.5 * phase * phase * phase * phase)
        + max(0.0, elapsed - gather - expansion);
}

// The row first gathers a visible school, then smoothly gives it outward
// momentum. Old grains continue through the same shared current after launch.
ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    if (u.Pattern != 10.0 && u.Pattern != 11.0) {
        state.position = vec3(0.0);
        state.velocity = vec3(0.0);
        state.age = p.time;
        return state;
    }
    vec3 random = reaction_random(p.index, p.seed);
    vec3 event = reaction_event(p.seed);
    vec3 shape = reaction_profile(p.seed, 211u);
    vec3 source = release_source(p.index, random, u);
    float birth = u.Timing.x + release_birth(source.x, reaction_quantile(p.index, p.seed), u, p.seed);
    float elapsed = max(0.0, p.time - birth) * (1.0 - u.Timing.z);
    float seconds = min(p.delta, elapsed);
    float gather = 0.13 + event.y * 0.10;
    float expansion = 0.22 + shape.z * 0.18;
    vec3 velocity = release_velocity(source, random, u, p.seed);
    vec3 focus = vec3((shape.x - 0.5) * 160.0, (shape.y - 0.5) * 14.0, source.z + 3.0);
    vec3 gathering = (focus - source) * 0.38 + vec3(0.0, 6.0, 0.0);
    if (p.reset) {
        state.position = source;
        state.velocity = gathering + velocity * smoothstep(gather, gather + expansion, elapsed);
        state.position += gathering * min(elapsed, gather)
            + velocity * reaction_drag(0.45, release_distance(elapsed, gather, expansion));
        if (u.Loss > 0.5) {
            state.velocity = velocity * 0.28;
            state.position = source + state.velocity * reaction_drag(0.45, elapsed);
        }
        state.age = p.time;
        return state;
    }
    if (u.Loss < 0.5) {
        float impulse = smoothstep(gather, gather + expansion, elapsed)
            - smoothstep(gather, gather + expansion, max(0.0, elapsed - seconds));
        state.velocity += velocity * impulse;
    }
    return reaction_advect(state, p, u, birth, 0.16 + shape.y * 0.08);
}

// Fracture, opaque debris, microembers and energy share a captured row front.
ChibiParticle release(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = reaction_random(p.index, p.seed);
    vec3 event = reaction_event(p.seed);
    float quantile = reaction_quantile(p.index, p.seed);
    vec4 layers = vec4(
        reaction_pattern(u.Pattern, 5.0), reaction_pattern(u.Pattern, 9.0),
        reaction_pattern(u.Pattern, 10.0), reaction_pattern(u.Pattern, 11.0)
    );
    vec3 source = release_source(p.index, random, u);
    float birth = release_birth(source.x, quantile, u, p.seed);
    float duration = u.Life * (0.56 + random.z * 0.16 +
        (layers.z + layers.w) * (0.16 + random.z * 0.02));
    vec3 character = clamp(u.Character, 0.0, 1.0);
    duration *= (0.82 + event.z * 0.18) * reaction_cooling(character);
    // Loss adds a sparse ember tail behind the departing original block meshes.
    duration = mix(duration, u.Life * (0.78 + random.z * 0.14), u.Loss);
    duration = min(duration, max(0.001, u.Life - birth - 0.015));
    float elapsed = max(0.0, p.time - u.Timing.x - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float tail = layers.z + layers.w;
    float attack = mix(0.018 + event.x * 0.022, 0.075 + event.x * 0.045, tail);
    float envelope = smoothstep(0.0, min(attack, duration * 0.2), elapsed) *
        (1.0 - smoothstep(duration * (0.24 + tail * 0.30), duration, elapsed)) * u.Timing.y;
    float loss_envelope = smoothstep(0.0, 0.045, elapsed) *
        (1.0 - smoothstep(duration * 0.42, duration, elapsed)) * u.Timing.y;
    envelope = mix(envelope, loss_envelope, u.Loss);
    int cell = int(mod(p.index, 10.0));
    float occupied = float((int(u.RowMask) >> cell) & 1);
    envelope *= occupied * reaction_population(p.index, p.seed, event);
    float motion = elapsed * (1.0 - u.Timing.z);
    float spread = (0.7 + event.y * 0.75) * (1.0 + character.x * 0.5 + character.z * 0.25);

    // Heavy fragments use exact exponential drag and gravity.
    float drag_rate = (4.5 - layers.y * 2.7) * (0.72 + event.z * 0.62);
    float drag_time = reaction_drag(drag_rate, motion);
    float gravity_time = (motion - drag_time) / drag_rate;
    vec3 direction = release_velocity(source, random, u, p.seed);
    vec3 fracture = source + direction * vec3(0.18, 0.22, 0.10) * drag_time * spread;
    vec3 debris = source + (direction * vec3(0.75, 0.55, 0.16) + vec3(0.0, 55.0, 0.0)) *
        drag_time * spread - vec3(0.0, (180.0 + event.x * 140.0) * gravity_time, 0.0);

    float fleck = step(0.78, random.y);
    float coarse = smoothstep(0.94, 0.995, random.z);
    float wake = step(0.48, random.y) * (1.0 - fleck);
    float clear_size = (
        layers.x * (1.25 + random.z * 1.5) + layers.y * (2.2 + random.z * 3.6) +
        layers.z * (1.25 + random.z * 1.7 + coarse * 7.0 + fleck * 0.7) +
        layers.w * (1.0 + random.z * 1.4 + coarse * 3.0)
    ) * (1.0 - phase * 0.24) * mix(envelope, sqrt(max(envelope, 0.0)), tail);
    float loss_size = (0.55 + random.z * 0.7) * (1.0 - phase * 0.3) * envelope;
    clear_size = mix(clear_size, loss_size, u.Loss);
    // The brief highlight rises during expansion; it never flashes the whole
    // captured row white at birth. Colored grains then cool for several seconds.
    float gather = 0.13 + event.y * 0.10;
    float highlight = smoothstep(gather - 0.04, gather + 0.10, elapsed)
        * (1.0 - smoothstep(gather + 0.10, gather + 0.58, elapsed));
    vec3 cool = mix(vec3(0.18, 0.82, 1.1), vec3(1.1, 0.48, 0.16), step(0.84, random.y));
    vec3 color = mix(cool, vec3(1.45, 1.85, 2.15), highlight * 0.48)
        * (1.0 + layers.x * 0.25 + layers.w * 0.15);
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
    result.size = mix(clear_size * spread, envelope, u.Timing.z);
    result.color = mix(clear_color, vec4(0.4, 0.8, 1.0, envelope * 0.4), u.Timing.z);
    result.scale = mix(vec3(1.0), vec3(2.2, 0.38, 0.6), layers.x * (1.0 - u.Timing.z)) *
        mix(vec3(1.0), vec3(
            1.2 + random.x * 0.8, 0.5 + random.y * 0.55, 0.65 + random.z * 0.55
        ), layers.y * (1.0 - u.Timing.z));
    result.rotation = vec3(random.x - 0.5, random.y - 0.5, random.z - 0.5) *
        (2.2 + motion * 8.0) * layers.y * (1.0 - u.Timing.z);
    if (u.Pattern == 10.0 || u.Pattern == 11.0) {
        float streak = (1.0 - coarse) * step(0.80, random.x);
        result.scale = vec3(1.0 + streak * 2.2, 0.9, 0.9);
        result.rotation = vec3(0.0, 0.0, reaction_heading(p.velocity));
    }
    result.lighting = (layers.y * 0.85 + layers.z * 0.35) * (1.0 - u.Timing.z);
    return reaction_illuminate(result, u, random, p.seed);
}
