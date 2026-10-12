#include "CosmicCommon.glsl"

vec2 stage_rotate(vec2 point, float angle)
{
    return mat2(cos(angle), sin(angle), -sin(angle), cos(angle)) * point;
}

// Thin concentric traces have different angular speeds. Polygon edges belong
// to the same rings, so the lattice stays coherent.
vec3 stage_orbit(vec3 random, float flow, float lattice)
{
    float lane = floor(random.y * 7.0);
    float angle = random.x * COSMIC_TAU + flow * (0.028 - mod(lane, 2.0) * 0.047);
    float radius = 770.0 + lane * 112.0;
    float sector = COSMIC_TAU / 12.0;
    float polygon = cos(sector * 0.5) / cos(mod(angle + sector * 0.5, sector) - sector * 0.5);
    radius *= mix(1.0, polygon, lattice);
    radius += (random.z - 0.5) * 5.0;
    return vec3(vec2(cos(angle), sin(angle)) * radius, -1580.0 - lane * 22.0);
}

// Original diamond-and-tick marks, repeated around broad orbital bands.
// The marks use line samples rather than textures or borrowed symbol artwork.
vec3 stage_marks(vec3 random, float flow)
{
    float band = floor(random.y * 4.0);
    float mark = floor(random.x * 32.0);
    float progress = fract(random.x * 32.0) * 4.0;
    float edge = floor(progress);
    float a = edge * COSMIC_TAU * 0.25;
    float b = a + COSMIC_TAU * 0.25;
    vec2 diamond = mix(vec2(cos(a), sin(a)), vec2(cos(b), sin(b)), fract(progress));
    vec2 local = diamond * vec2(35.0, 62.0);
    if (mod(mark + band, 3.0) < 1.0) local = vec2((fract(progress) - 0.5) * 86.0, 0.0);
    float angle = mark * COSMIC_TAU / 32.0 + flow * (0.014 - mod(band, 2.0) * 0.031);
    vec2 point = stage_rotate(vec2(970.0 + band * 185.0, 0.0) + local, angle);
    return vec3(point, -1640.0 - band * 35.0);
}

vec3 stage_triangle(vec3 random, float flow, float beat)
{
    float progress = random.x * 3.0;
    float edge = floor(progress);
    vec2 first = vec2(0.0, 1.0);
    vec2 second = vec2(-0.9, -0.8);
    if (edge > 0.5) {
        first = second;
        second = vec2(0.9, -0.8);
    }
    if (edge > 1.5) {
        first = second;
        second = vec2(0.0, 1.0);
    }
    float layer = floor(random.y * 3.0);
    float radius = (1110.0 + layer * 215.0) * (1.0 + beat * 0.12);
    vec2 point = mix(first, second, fract(progress)) * radius;
    point += (random.z - 0.5) * 5.0;
    return vec3(stage_rotate(point, sin(flow * 0.08) * 0.035), -1550.0 - layer * 70.0);
}

// Loose streams flank a single distant rim in the cyan scene. Their widths
// taper continuously, retaining the fine-grain look of the main live cloud.
vec3 stage_school(vec3 random, float flow)
{
    float school = floor(random.y * 6.0);
    float along = random.x * 2.0 - 1.0;
    float side = mod(school, 2.0) * 2.0 - 1.0;
    float phase = school * 1.73 + flow * 0.045;
    float width = (1.0 - along * along) * 90.0;
    float x = side * (1120.0 + sin(phase) * 200.0) + along * 620.0;
    float y = (school - 2.5) * 180.0 + sin(along * 3.2 + phase) * 175.0;
    y += (fract(random.y * 6.0) - 0.5) * width;
    return vec3(x, y, -1430.0 + cos(phase + along) * 270.0 + random.z * 35.0);
}

ChibiParticle stars(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    ChibiParticle result = chibi_particle();
    vec3 position;
    float opacity;
    float brightness;
    float size;
    // Every quality level samples the same stable populations. One quarter
    // remains unstructured distant dust, including the effects-off setting.
    if (random.z < 0.25) {
        position = vec3((random.x - 0.5) * 6500.0, (random.y - 0.5) * 3800.0,
            -1400.0 - random.z * 8800.0);
        position += cosmic_curl(position * 0.0011, u.Flow * 0.35) * 65.0;
        opacity = 0.1 + random.y * 0.24;
        brightness = 0.22 + u.Treble * 0.12;
        size = 0.18 + 0.24 * pow(random.y, 7.0);
    } else {
        random.z = (random.z - 0.25) / 0.75;
        float role = floor(random.z * 4.0);
        vec3 geometry = stage_orbit(random, u.Flow, weights.w);
        vec3 opening = stage_school(random, u.Flow);
        opacity = 0.065 + u.Energy * 0.14 + u.Beat * 0.28;
        brightness = 0.36 + u.Energy * 0.55 + u.Beat * 1.1;
        size = 0.95 + random.y * 0.18;
        if (role < 1.0) {
            float angle = random.x * COSMIC_TAU;
            float rim = 790.0 + random.y * 16.0;
            opening = vec3(cos(angle) * rim + 120.0, sin(angle) * rim + 40.0, -1740.0);
            // A dim far side leaves the rim readable as a crescent in space.
            brightness *= mix(1.0, 0.2 + pow(0.5 + 0.5 * cos(angle - 0.6), 3.0), weights.x);
        }
        if (role > 1.5 && role < 2.5) geometry = stage_marks(random, u.Flow);
        if (role > 2.5) {
            geometry = stage_triangle(random, u.Flow, u.Beat);
            float accent = u.Beat * u.Beat;
            opacity *= 0.13 + accent * 0.95 + u.Reaction * 0.2;
            brightness *= 0.5 + accent * 1.5;
            size *= 1.0 + accent * 0.6;
        }
        position = opening * weights.x + geometry * (weights.y + weights.z + weights.w);
        position.y += 20.0;
        position.x *= 1.0 + u.Duet * 0.55;
    }
    float action = cosmic_event_light(position, u);
    float quiet = cosmic_board_quiet(cosmic_board_projection(position, u.Eye), u.Duet);
    result.position = position;
    result.size = size * (1.0 + action * 0.2);
    result.color = vec4(cosmic_tint(random, position, weights, u) * (brightness + action * 0.8),
        opacity * quiet);
    result.scale = vec3(1.0);
    result.rotation = random * COSMIC_TAU;
    result.lighting = 0.1;
    return result;
}
