#include "ReactionCommon.glsl"

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
    float quantile = reaction_quantile(p.index, p.seed);
    int cell = int(mod(p.index, 4.0));
    vec3 point = u.Point0;
    if (cell == 1) point = u.Point1;
    if (cell == 2) point = u.Point2;
    if (cell == 3) point = u.Point3;
    vec3 center = (u.Point0 + u.Point1 + u.Point2 + u.Point3) * 0.25;
    int pattern = int(u.Pattern + 0.5);
    float age = max(0.0, p.time - u.Delay);
    float birth = reaction_birth(min(u.Life * 0.12, 0.045), quantile);
    float duration = u.Life * (0.70 + random.z * 0.16);
    if (pattern == 4 || pattern == 8) birth = quantile * 0.006;
    float elapsed = max(0.0, age - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float envelope = smoothstep(0.0, min(0.012, duration * 0.12), elapsed) *
        (1.0 - smoothstep(duration * 0.12, duration, elapsed)) * u.Fade;
    float motion = elapsed * (1.0 - u.Quiet);
    float drag = reaction_drag(3.6, motion);
    float gravity_time = (motion - drag) / 3.6;
    float fleck = step(0.76, random.z);
    float mist = 1.0 - step(0.28, random.z);
    float side = floor(random.y * 4.0);
    vec2 normal = vec2(1.0, 0.0);
    if (side == 1.0) normal = vec2(0.0, 1.0);
    if (side == 2.0) normal = vec2(-1.0, 0.0);
    if (side == 3.0) normal = vec2(0.0, -1.0);
    vec2 tangent = vec2(-normal.y, normal.x);
    vec3 edge = vec3(normal * 10.7 + tangent * (random.x - 0.5) * 21.0, 0.6);
    vec3 position = point + edge;
    float exposed = cue_exposed(point, normal, u);
    float stretch = 1.0;
    float orientation = random.x * 6.283185;
    float lighting = 0.18;

    if (pattern == 0) {
        position += vec3((random.x - 0.5) * 100.0, 25.0 + random.y * 55.0,
                         (random.z - 0.5) * 12.0) * drag;
        position.y -= motion * motion * 110.0;
    }
    if (pattern == 1) {
        normal = vec2(-u.Direction, 0.0);
        exposed = cue_exposed(point, normal, u);
        // The record is the destination. Distribute the wake over the vacated cell.
        float trail = u.Travel * 24.0 * quantile;
        position = point + vec3(-u.Direction * (10.7 + trail),
            (random.y - 0.5) * 20.0, 0.6);
        position += vec3(-u.Direction * (12.0 + random.z * 22.0),
            (random.y - 0.5) * 14.0, 3.0 + random.x * 8.0) * drag;
        stretch = 1.6 + fleck * 1.8;
        orientation = 0.0;
    }
    if (pattern == 2) {
        vec3 radial = point + edge - u.Pivot;
        float angle = -u.Direction * (0.28 + random.z * 0.52) *
            (1.0 - exp(-5.0 * motion));
        position = u.Pivot + vec3(
            radial.x * cos(angle) - radial.y * sin(angle),
            radial.x * sin(angle) + radial.y * cos(angle), radial.z);
        position += vec3(normal * (5.0 + random.z * 10.0) * drag,
            (2.0 + random.x * 7.0) * drag);
        orientation = atan(-u.Direction * radial.x, u.Direction * radial.y) + angle;
        stretch = 1.4 + fleck * 2.0;
    }
    if (pattern == 3) {
        normal = vec2(0.0, 1.0);
        exposed = cue_exposed(point, normal, u);
        // A soft-step record keeps the source row; its wake reaches the new row.
        float travel = u.Travel * 24.0;
        position = point + vec3((random.x - 0.5) * 19.0,
            10.7 - travel * (1.0 - quantile), 0.6);
        position += vec3((random.y - 0.5) * 10.0, -18.0 - random.z * 12.0,
            2.0 + random.x * 6.0) * drag;
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
        position += velocity * drag - vec3(0.0, gravity * gravity_time, 0.0);
        orientation = atan(velocity.y - gravity * drag, velocity.x);
        stretch = 1.0 + fleck * 1.4;
    }
    if (pattern == 6) {
        position += (center - point) * drag * 0.65 + vec3(0.0, 12.0, 8.0) * drag;
    }
    if (pattern == 7) {
        // Travel carries the rejected action kind: wall, down, or perimeter.
        if (u.Travel == 1.0) normal = vec2(u.Direction, 0.0);
        if (u.Travel == 3.0) normal = vec2(0.0, -1.0);
        tangent = vec2(-normal.y, normal.x);
        exposed = cue_exposed(point, normal, u);
        position = point + vec3(normal * 10.7 + tangent * (random.x - 0.5) * 20.0, 0.7);
        position += vec3(-normal * 12.0 * drag, 3.0 * drag);
        orientation = atan(tangent.y, tangent.x);
        stretch = 1.4;
    }
    if (pattern == 8) {
        // One brief granular wash occupies the traveled cells as contact begins.
        // Horizontal scatter and unequal lengths prevent four parallel rails.
        float progress = smoothstep(0.0, 0.06, age) * (1.0 - u.Quiet);
        position = point + vec3((random.x - 0.5) * 19.0,
            -u.Travel * 24.0 * (1.0 - quantile * (1.0 - progress)),
            0.7 + random.y * 2.0);
        exposed = 1.0;
        envelope *= 1.0 - smoothstep(0.02, 0.075, age);
        stretch = 1.4 + random.y * 2.0;
        orientation = 1.5707963;
        lighting = 0.0;
    }

    envelope *= exposed;
    float hot = exp(-elapsed * 20.0);
    vec3 cool = mix(vec3(0.14, 0.48, 0.88), vec3(0.40, 0.63, 0.86), random.y);
    cool = mix(cool, vec3(0.78, 0.36, 0.13), clamp(u.Hue - 1.0, 0.0, 1.0));
    float brightness = 0.52 + u.Strength * 0.5 + fleck * 0.35;
    if (pattern == 4) brightness += hot * 0.8;
    if (pattern == 12) brightness *= 0.62;
    ChibiParticle result;
    result.position = position;
    result.size = (0.65 + random.z * 0.95 + fleck * 0.45) *
        (0.88 + u.Strength * 0.28) * (1.0 - phase * 0.20) *
        sqrt(max(envelope, 0.0));
    result.color = vec4(mix(cool * brightness, vec3(1.6, 1.85, 2.1), hot),
        envelope * (0.42 + fleck * 0.32 - mist * 0.17) * (1.0 - u.Quiet * 0.3));
    result.scale = vec3(stretch, 0.65 + random.x * 0.3, 0.65 + random.y * 0.3);
    result.rotation = vec3((random.x - 0.5) * 0.7, (random.y - 0.5) * 0.7, orientation);
    result.lighting = lighting * (1.0 - u.Quiet);
    return result;
}
