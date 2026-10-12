#include "ReactionCommon.glsl"

// A lower occupied cell suppresses the upper cell's landing grains.
float cue_cell_below(vec3 point, vec3 cell)
{
    return (1.0 - step(0.5, abs(point.x - cell.x))) *
        (1.0 - step(0.5, abs(point.y - cell.y - 24.0)));
}

ChibiParticle cue(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = reaction_random(p.index, p.seed);
    float cell = fract(p.index * 0.25) * 4.0;
    vec3 point = u.Point0 * (1.0 - step(0.5, cell)) +
        u.Point1 * (step(0.5, cell) - step(1.5, cell)) +
        u.Point2 * (step(1.5, cell) - step(2.5, cell)) +
        u.Point3 * step(2.5, cell);
    vec3 center = (u.Point0 + u.Point1 + u.Point2 + u.Point3) * 0.25;

    float quantile = reaction_quantile(p.index, p.seed);
    float birth = reaction_birth(u.Life * 0.2, quantile);
    float duration = u.Life * (0.62 + random.z * 0.14);
    float elapsed = max(0.0, p.time - u.Delay - birth);
    float phase = clamp(elapsed / max(duration, 0.001), 0.0, 1.0);
    float envelope = smoothstep(0.0, min(0.05, duration * 0.22), elapsed) *
        (1.0 - smoothstep(duration * 0.18, duration, elapsed)) * u.Fade;

    float motion = elapsed * (1.0 - u.Quiet);
    float drag = reaction_drag(4.2, motion);
    float gravity_time = (motion - drag) / 4.2;
    vec3 edge = vec3(
        mix((random.x - 0.5) * 21.0,
            (step(0.5, random.z) * 2.0 - 1.0) * 10.7, step(0.5, random.y)),
        mix((step(0.5, random.z) * 2.0 - 1.0) * 10.7,
            (random.x - 0.5) * 21.0, step(0.5, random.y)),
        0.35
    );
    vec3 radial = point - center + vec3(edge.xy, 0.0);
    float angle = -u.Direction * (0.35 + random.z * 0.25) *
        (1.0 - exp(-4.2 * motion));
    float fleck = step(0.78, random.y);
    float wake = step(0.48, random.y) * (1.0 - fleck);

    // Each position is in the captured piece's coordinate frame.
    vec3 fracture = point + edge + vec3(
        (random.x - 0.5) * 100.0, 25.0 + random.y * 55.0,
        (random.z - 0.5) * 12.0
    ) * drag - vec3(0.0, motion * motion * 110.0, 0.0);
    vec3 move_edge = point + vec3(
        -u.Direction * 10.7, (random.y - 0.5) * 19.0, 0.35
    ) + vec3(
        -u.Direction * (12.0 + random.z * 18.0),
        (random.y - 0.5) * 8.0, (random.z - 0.5) * 6.0
    ) * drag;
    vec3 turn = center + vec3(
        radial.x * cos(angle) - radial.y * sin(angle),
        radial.x * sin(angle) + radial.y * cos(angle), radial.z
    ) + vec3(
        (random.x - 0.5) * 4.0 * drag, -12.0 * gravity_time,
        0.35 + (random.z - 0.5) * 5.0 * drag
    );
    vec3 soft_fall = point + vec3(
        (random.x - 0.5) * 16.0, -10.7 - 18.0 * drag,
        0.35 + (random.z - 0.5) * drag * 8.0
    );
    vec3 contact = point + vec3((random.x - 0.5) * 20.0, -10.7, 0.35) + vec3(
        (random.x - 0.5) * (25.0 + fleck * 24.0),
        fleck * (18.0 + random.z * 22.0),
        (random.z - 0.5) * (12.0 + fleck * 18.0)
    ) * drag - vec3(0.0, 50.0 * gravity_time, 0.0);
    vec3 hold = point + edge + (center - point) * drag * 0.65 +
        vec3(0.0, 12.0 * drag, 8.0 * drag);
    vec3 blocked = point + vec3(
        u.Direction * 10.7, (random.y - 0.5) * 18.0, 0.35
    ) + vec3(
        -u.Direction * 9.0 * drag, -8.0 * gravity_time,
        (random.z - 0.5) * drag * 3.0
    );
    float progress = smoothstep(0.0, 0.16, max(0.0, p.time - u.Delay)) *
        (1.0 - u.Quiet);
    float tail = min(u.Travel * 24.0 * progress, 12.0) *
        (1.0 - smoothstep(0.16, 0.32, max(0.0, p.time - u.Delay)));
    vec3 packet = point + vec3(
        (random.y - 0.5) * 2.0,
        -u.Travel * 24.0 * progress + tail * random.z * (0.2 + wake * 0.6),
        0.35 + (random.x - 0.5) * 1.2
    );
    vec3 settling = point + vec3((random.x - 0.5) * 20.0, -10.7, 0.6) + vec3(
        (random.x - 0.5) * 32.0, 6.0 + random.y * 12.0, 5.0 + random.z * 10.0
    ) * drag - vec3(0.0, 18.0 * gravity_time, 0.0) + vec3(
        sin(random.z * 6.283185 + motion * 3.0) - sin(random.z * 6.283185),
        sin(random.x * 6.283185 + motion * 2.0) - sin(random.x * 6.283185),
        sin(random.y * 6.283185 + motion * 2.5) - sin(random.y * 6.283185)
    ) * (1.0 - exp(-2.4 * motion)) * 2.5;

    float move_weight = reaction_pattern(u.Pattern, 1.0);
    float contact_weight = reaction_pattern(u.Pattern, 4.0);
    float settling_weight = reaction_pattern(u.Pattern, 12.0);
    float exposed = 1.0 - max(
        max(cue_cell_below(point, u.Point0), cue_cell_below(point, u.Point1)),
        max(cue_cell_below(point, u.Point2), cue_cell_below(point, u.Point3))
    );
    float visibility = mix(1.0, exposed, max(contact_weight, settling_weight));
    envelope = envelope * visibility;
    vec3 palette = mix(
        mix(vec3(0.17, 0.65, 0.88), vec3(0.66, 0.3, 0.9), clamp(u.Hue, 0.0, 1.0)),
        vec3(1.0, 0.34, 0.065), clamp(u.Hue - 1.0, 0.0, 1.0)
    );

    ChibiParticle result;
    result.position = fracture * reaction_pattern(u.Pattern, 0.0) +
        move_edge * move_weight + turn * reaction_pattern(u.Pattern, 2.0) +
        soft_fall * reaction_pattern(u.Pattern, 3.0) + contact * contact_weight +
        hold * reaction_pattern(u.Pattern, 6.0) + blocked * reaction_pattern(u.Pattern, 7.0) +
        packet * reaction_pattern(u.Pattern, 8.0) + settling * settling_weight;
    result.size = (0.9 + random.z * 0.65 + wake * 0.35) *
        (0.78 + u.Strength * 0.35) * (1.0 - phase * 0.24) * envelope *
        (2.0 - envelope) * (1.0 + contact_weight * 0.3) *
        (1.0 - settling_weight * 0.25);
    result.color = vec4(
        mix(vec3(0.85, 1.1, 1.25), palette, smoothstep(0.0, 0.42, phase)) *
            (0.62 + u.Strength * 0.32),
        envelope * (0.42 + random.z * 0.18) * (1.0 - wake * 0.45) *
            (1.0 - u.Quiet * 0.25) * (1.0 - settling_weight * 0.3)
    );
    result.scale = mix(vec3(1.0), vec3(1.7, 0.65, 0.65), move_weight) *
        mix(vec3(1.0), vec3(0.8, 0.8, 0.65), settling_weight) *
        vec3(0.8 + random.x * 0.35, 0.85 + random.y * 0.3, 0.8 + random.z * 0.3);
    result.rotation = vec3(random.x - 0.5, random.y - 0.5, random.z - 0.5) *
        (1.4 + motion * 1.6);
    result.lighting = (0.18 + contact_weight * 0.3 + settling_weight * 0.28) *
        (1.0 - u.Quiet);
    return result;
}
