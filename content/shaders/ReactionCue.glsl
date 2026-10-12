#include "ReactionCommon.glsl"

float cue_birth(int pattern, float life, float quantile, vec3 event)
{
    if (pattern == 13) return quantile * (0.03 + event.z * 0.11);
    if (pattern == 4 || pattern == 8) return quantile * (0.002 + event.z * 0.014);
    return reaction_birth(min(life * 0.18, 0.02 + event.y * 0.07), quantile);
}

vec3 cue_point(float index, ChibiParameters u)
{
    int cell = int(mod(index, 4.0));
    if (cell == 1) return u.Point1;
    if (cell == 2) return u.Point2;
    if (cell == 3) return u.Point3;
    return u.Point0;
}

vec2 cue_normal(vec3 random)
{
    int side = int(floor(random.y * 4.0));
    if (side == 1) return vec2(0.0, 1.0);
    if (side == 2) return vec2(-1.0, 0.0);
    if (side == 3) return vec2(0.0, -1.0);
    return vec2(1.0, 0.0);
}

// Move leaves a feather from the vacated edge. Rotation launches tangentially
// around the recorded pivot. Drop starts a broad rising school along its path.
ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    int pattern = int(u.Pattern + 0.5);
    if (pattern != 1 && pattern != 2 && pattern != 13) {
        state.position = vec3(0.0);
        state.velocity = vec3(0.0);
        state.age = p.time;
        return state;
    }
    vec3 random = reaction_random(p.index, p.seed);
    vec3 event = reaction_event(p.seed);
    float quantile = reaction_quantile(p.index, p.seed);
    float birth = u.Timing.x + cue_birth(pattern, u.Life, quantile, event);
    if (p.reset) {
        vec3 point = cue_point(p.index, u);
        vec2 normal = cue_normal(random);
        vec2 tangent = vec2(-normal.y, normal.x);
        state.position = point + vec3(normal * 10.7 + tangent * ((random.x - 0.5) * 14.0 + (event.y - 0.5) * 7.0), 0.6);
        float depth = mix(-150.0 - random.y * 190.0, 240.0 + random.y * 180.0, step(0.72, random.z));
        if (pattern == 1) {
            state.position = point + vec3(-u.Action.x * (10.7 + u.Action.y * 24.0 * quantile),
                (random.y - 0.5) * 20.0, 0.6);
            state.velocity = vec3(-u.Action.x * (75.0 + event.y * 100.0 + random.z * 50.0),
                (event.x - 0.5) * 120.0 + (random.y - 0.5) * 45.0, depth * 0.45);
        }
        if (pattern == 2) {
            vec2 radial = state.position.xy - u.Pivot.xy;
            vec2 turn = vec2(radial.y, -radial.x) * u.Action.x;
            state.velocity = vec3(turn * (5.5 + event.y * 3.5) + normal * 45.0, depth * 0.70);
        }
        if (pattern == 13) {
            state.position = point + vec3((random.x - 0.5) * 21.0,
                -u.Action.y * 24.0 * quantile, (random.y - 0.5) * 8.0 + 4.0);
            float side = event.x < 0.5 ? -1.0 : 1.0;
            float ribbon = sin(quantile * 4.2 + event.z * 6.283185);
            state.velocity = vec3(side * (190.0 + event.y * 130.0 + random.x * 90.0) + ribbon * 65.0,
                140.0 + event.z * 95.0 + random.y * 125.0, depth) * vec3(0.65, 0.52, 0.60);
            // Near grains grow through depth while lingering beside the well.
            state.velocity.xy *= 1.0 - smoothstep(0.92, 0.99, random.z) * 0.28;
        }
        vec2 direction = vec2(-u.Action.x, (event.x - 0.5) * 0.35);
        if (pattern == 2) {
            vec2 radial = state.position.xy - u.Pivot.xy;
            direction = vec2(radial.y, -radial.x) * u.Action.x;
        }
        if (pattern == 13) direction = vec2(event.x < 0.5 ? -1.0 : 1.0, 0.8);
        state.velocity = reaction_launch(state.velocity, direction, random, event, clamp(u.Character, 0.0, 1.0));
        // A late first draw preserves the event's age without replaying frames.
        float elapsed = max(0.0, p.time - birth) * (1.0 - u.Timing.z);
        state.position += state.velocity * reaction_drag(0.55, elapsed);
        state.age = p.time;
        return state;
    }
    return reaction_advect(state, p, u, birth, pattern == 13 ? 0.23 : 0.48);
}

// A few forward grains grow through perspective; receding grains remain fine.
// The visible tail cools slowly while the short beam has already disappeared.
ChibiParticle drop_wake(ChibiParticleInput p, ChibiParameters u, vec3 random, float quantile)
{
    vec3 event = reaction_event(p.seed);
    float birth = cue_birth(13, u.Life, quantile, event);
    float elapsed = max(0.0, p.time - u.Timing.x - birth);
    float duration = reaction_duration(u.Life, random, event, clamp(u.Character, 0.0, 1.0), birth);
    float envelope = smoothstep(0.0, 0.065, elapsed) *
        (1.0 - smoothstep(duration * 0.52, duration, elapsed)) * u.Timing.y * reaction_population(p.index, p.seed, event);
    float hot = exp(-elapsed * 7.0);
    float coarse = smoothstep(0.92, 0.99, random.z);
    float grain = (1.65 + random.y * 1.8 + coarse * 6.5) * (0.8 + event.y * 0.6);
    float light = 1.65 + random.y * 1.2 + u.Current.z * 0.75;
    ChibiParticle result = chibi_particle();
    result.position = p.position;
    result.size = grain * sqrt(max(envelope, 0.0));
    result.color = vec4(mix(vec3(0.08, 0.82, 0.91) * light, vec3(0.5, 1.15, 1.45), hot),
        envelope * (0.44 + random.z * 0.34));
    float streak = (1.0 - coarse) * step(0.62, random.x);
    result.scale = vec3(1.0 + streak * (0.7 + hot * 2.0), 0.90, 0.90);
    result.rotation = vec3(0.0, 0.0, reaction_heading(p.velocity));
    result.lighting = 0.2;
    return reaction_illuminate(result, u, random, p.seed);
}

float cue_neighbor(vec3 point, vec3 cell, vec2 normal)
{
    return 1.0 - step(0.5, length(cell.xy - point.xy - normal * 24.0));
}

// Emit from the piece silhouette, never from the seam between occupied cells.
float cue_exposed(vec3 point, vec2 normal, ChibiParameters u)
{
    return 1.0 - max(
        max(cue_neighbor(point, u.Point0, normal), cue_neighbor(point, u.Point1, normal)),
        max(cue_neighbor(point, u.Point2, normal), cue_neighbor(point, u.Point3, normal))
    );
}

ChibiParticle cue(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = reaction_random(p.index, p.seed);
    vec3 event = reaction_event(p.seed);
    float quantile = reaction_quantile(p.index, p.seed);
    int cell = int(mod(p.index, 4.0));
    vec3 point = u.Point0;
    if (cell == 1) point = u.Point1;
    if (cell == 2) point = u.Point2;
    if (cell == 3) point = u.Point3;
    vec3 center = (u.Point0 + u.Point1 + u.Point2 + u.Point3) * 0.25;
    int pattern = int(u.Pattern + 0.5);
    if (pattern == 13) return drop_wake(p, u, random, quantile);
    float age = max(0.0, p.time - u.Timing.x);
    float birth = cue_birth(pattern, u.Life, quantile, event);
    float duration = reaction_duration(u.Life, random, event, clamp(u.Character, 0.0, 1.0), birth);
    float elapsed = max(0.0, age - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float envelope = smoothstep(0.0, min(0.012, duration * 0.12), elapsed) *
        (1.0 - smoothstep(duration * 0.12, duration, elapsed)) * u.Timing.y;
    float motion = elapsed * (1.0 - u.Timing.z);
    vec3 character = clamp(u.Character, 0.0, 1.0);
    float spread = (0.65 + event.y * 0.8) * (1.0 + character.x * 0.45 + character.z * 0.25);
    float drag_rate = 2.5 + event.z * 2.2;
    float drag = reaction_drag(drag_rate, motion);
    float gravity_time = (motion - drag) / drag_rate;
    float fleck = step(0.76, random.z);
    float mist = 1.0 - step(0.28, random.z);
    float side = floor(random.y * 4.0);
    vec2 normal = vec2(1.0, 0.0);
    if (side == 1.0) normal = vec2(0.0, 1.0);
    if (side == 2.0) normal = vec2(-1.0, 0.0);
    if (side == 3.0) normal = vec2(0.0, -1.0);
    vec2 tangent = vec2(-normal.y, normal.x);
    vec3 edge = vec3(normal * 10.7 + tangent * ((random.x - 0.5) * 14.0 + (event.y - 0.5) * 7.0), 0.6);
    vec3 position = point + edge;
    float exposed = cue_exposed(point, normal, u);
    float stretch = 1.0;
    float orientation = random.x * 6.283185;
    float lighting = 0.18;

    if (pattern == 0) {
        position += vec3((random.x - 0.5) * 100.0 + (event.x - 0.5) * 70.0, 15.0 + random.y * 55.0 + event.z * 25.0,
                         (random.z - 0.5) * 12.0) * drag * spread;
        position.y -= motion * motion * 110.0;
    }
    if (pattern == 1) {
        normal = vec2(-u.Action.x, 0.0);
        exposed = cue_exposed(point, normal, u);
    }
    if (pattern == 3) {
        normal = vec2(0.0, 1.0);
        exposed = cue_exposed(point, normal, u);
        // A soft-step record keeps the source row; its wake reaches the new row.
        float travel = u.Action.y * 24.0;
        position = point + vec3((random.x - 0.5) * 19.0,
            10.7 - travel * (1.0 - quantile), 0.6);
        position += vec3((random.y - 0.5) * 10.0 + (event.x - 0.5) * 20.0, -14.0 - random.z * 12.0 - event.y * 10.0,
            2.0 + random.x * 6.0) * drag * spread;
        stretch = 1.3 + fleck * 1.6;
        orientation = 1.5707963;
    }
    if (pattern == 4 || pattern == 12) {
        normal = vec2(0.0, -1.0);
        exposed = cue_exposed(point, normal, u);
        position = point + vec3((random.x - 0.5) * 21.0, -10.7, 0.7);
        vec3 velocity = vec3((random.x - 0.5) * (100.0 + fleck * 95.0),
            14.0 + random.y * 48.0, 12.0 + random.z * 22.0);
        float gravity = 125.0;
        lighting = 0.35;
        if (pattern == 12) {
            velocity = vec3((random.x - 0.5) * 55.0, 5.0 + random.y * 13.0,
                7.0 + random.z * 12.0);
            gravity = 20.0;
            lighting = 0.48;
        }
        velocity = reaction_launch(velocity, vec2(random.x < 0.5 ? -1.0 : 1.0, 0.35),
            random, event, character);
        position += velocity * drag * spread - vec3(0.0, gravity * gravity_time, 0.0);
        orientation = atan(velocity.y - gravity * drag, velocity.x);
        stretch = 1.0 + fleck * 1.4;
    }
    if (pattern == 6) {
        position += (center - point) * drag * (0.35 + event.y * 0.75)
            + vec3((event.x - 0.5) * 50.0, 4.0 + event.z * 20.0, 2.0 + event.y * 10.0) * drag * spread;
    }
    if (pattern == 7) {
        // Travel carries the rejected action kind: wall, down, or perimeter.
        if (u.Action.y == 1.0) normal = vec2(u.Action.x, 0.0);
        if (u.Action.y == 3.0) normal = vec2(0.0, -1.0);
        tangent = vec2(-normal.y, normal.x);
        exposed = cue_exposed(point, normal, u);
        position = point + vec3(normal * 10.7 + tangent * (random.x - 0.5) * 20.0, 0.7);
        position += vec3((-normal * (8.0 + event.y * 9.0) + tangent * (event.x - 0.5) * 16.0)
            * drag * spread, (2.0 + event.z * 4.0) * drag);
        orientation = atan(tangent.y, tangent.x);
        stretch = 1.4;
    }
    if (pattern == 8) {
        // Thin blue shafts fill the traveled volume. Different depths and
        // lengths make a column of light, without a flat screen-space strip.
        position = point + vec3((random.x - 0.5) * (14.0 + event.y * 14.0) * (1.0 + character.x * 0.65 - character.y * 0.25),
            -u.Action.y * 24.0 * quantile + (1.0 - quantile) * 42.0,
            2.0 + (random.y - 0.5) * (12.0 + character.x * 18.0));
        exposed = 1.0;
        envelope = smoothstep(0.0, 0.025 + event.x * 0.015, age) *
            (1.0 - smoothstep(0.07, u.Life * (0.8 + event.z * 0.2), age)) * u.Timing.y;
        stretch = 7.0 + random.y * 20.0;
        orientation = 1.5707963;
        lighting = 0.0;
    }

    if (pattern == 1 || pattern == 2) {
        position = p.position;
        orientation = reaction_heading(p.velocity);
        stretch = 1.2 + fleck * 1.4;
        envelope = smoothstep(0.0, 0.025, elapsed) *
            (1.0 - smoothstep(duration * 0.28, duration, elapsed)) * u.Timing.y;
    }
    envelope *= exposed * reaction_population(p.index, p.seed, event);
    float hot = exp(-elapsed * 20.0);
    vec3 cool = mix(vec3(0.14, 0.48, 0.88), vec3(0.40, 0.63, 0.86), random.y);
    cool = mix(cool, vec3(0.78, 0.36, 0.13), clamp(u.Hue - 1.0, 0.0, 1.0));
    float brightness = 0.52 + u.Action.z * 0.5 + fleck * 0.35;
    if (pattern == 4) brightness += hot * 0.8;
    if (pattern == 12) brightness *= 0.62;
    if (pattern == 1 || pattern == 2) brightness += 1.15;
    ChibiParticle result;
    result.position = position;
    result.size = (1.0 + random.z * 1.45 + fleck * 2.0) * (0.8 + event.z * 0.55) *
        (0.88 + u.Action.z * 0.28) * (1.0 - phase * 0.20) *
        sqrt(max(envelope, 0.0));
    result.color = vec4(mix(cool * brightness, vec3(1.6, 1.85, 2.1), hot),
        envelope * (0.42 + fleck * 0.32 - mist * 0.17) * (1.0 - u.Timing.z * 0.3));
    result.scale = vec3(stretch, 0.65 + random.x * 0.3, 0.65 + random.y * 0.3);
    result.rotation = vec3((random.x - 0.5) * 0.7, (random.y - 0.5) * 0.7, orientation);
    if (pattern == 8) {
        result.color = vec4(0.16, 0.7, 1.6, envelope * 0.14);
        result.scale = vec3(stretch, 1.3 + random.x, 1.8 + random.y * 1.8);
    }
    result.lighting = lighting * (1.0 - u.Timing.z);
    return reaction_illuminate(result, u, random, p.seed);
}
