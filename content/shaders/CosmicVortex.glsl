#include "CosmicCommon.glsl"

// Four force regimes share live particles. A transition changes their future
// acceleration and births; it does not interpolate one geometric shape into another.
float cloud_life(vec3 random)
{
    return 12.0 + random.x * 14.0;
}

vec3 cloud_center(vec3 random, vec4 weights, float variation)
{
    float cluster = floor(random.y * 9.0);
    vec3 knot = cosmic_random(cluster + 91.0, 17.0, variation + 23.0);
    vec3 swarm = vec3((knot.x - 0.5) * 3900.0, (knot.y - 0.5) * 2100.0, -1800.0 + (knot.z - 0.5) * 1050.0);
    return swarm * weights.x + vec3(430.0, -40.0, -1720.0) * weights.y
        + vec3(-230.0, 80.0, -1780.0) * weights.z + vec3(-1250.0, 0.0, -1850.0) * weights.w;
}

ChibiParticleState cloud_birth(ChibiParticleInput p, ChibiParameters u, vec3 identity, vec4 weights)
{
    // Reset seeds a filled volume. Later births enter transparently and follow
    // the same population's dynamics, with no synchronized reset or visible pop.
    float cycle = floor(p.time / cloud_life(identity));
    vec3 random = cosmic_random(p.index + cycle * 31.7, p.seed + 19.0, u.Variation + 51.0);
    float layer = floor(identity.z * 5.0);
    vec3 ball = cosmic_ball(random);
    vec3 center = cloud_center(identity, weights, u.Variation);
    vec3 spread = vec3(210.0 + layer * 58.0) * weights.x
        + vec3(130.0 + layer * 210.0) * weights.y
        + vec3(200.0 + layer * 280.0) * weights.z
        + vec3(950.0, 1060.0, 570.0) * weights.w;
    ChibiParticleState state;
    state.position = center + ball * spread;
    state.position += cosmic_curl(ball * 2.7 + identity * 3.0, u.Flow) * (100.0 + layer * 35.0);
    state.velocity = cosmic_curl(state.position * 0.0021, u.Flow) * (70.0 + layer * 14.0)
        + vec3(130.0 * weights.w, 0.0, 0.0);
    state.age = 0.0;
    if (p.reset) {
        float phase = cosmic_random(p.index, p.seed + 613.0, u.Variation + 997.0).y;
        state.age = phase * cloud_life(identity);
    }
    return state;
}

ChibiParticleState advect(ChibiParticleInput p, ChibiParameters u, ChibiParticleState state)
{
    vec3 identity = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    float life = cloud_life(identity);
    if (p.reset || state.age >= life) return cloud_birth(p, u, identity, weights);
    float layer = floor(identity.z * 5.0);
    vec3 center = cloud_center(identity, weights, u.Variation);
    vec3 relative = state.position - center;
    float radius = length(relative);
    vec3 radial = relative / max(radius, 1.0);
    float band = mix(u.Bass, u.Treble, layer * 0.25);
    float frequency = dot(weights, vec4(0.0024, 0.0034, 0.0018, 0.0020));
    frequency *= 0.82 + layer * 0.12 + u.Mid * 0.22;
    vec3 flow = cosmic_curl(state.position * frequency, u.Flow + u.Wind * 0.24);
    float speed = dot(weights, vec4(160.0, 125.0, 100.0, 170.0)) *
        (0.65 + layer * 0.14 + band * 0.7 + u.Reaction * 0.55);
    vec3 target = flow * speed + vec3(150.0, 12.0, 0.0) * weights.w;
    // Rotation is an acceleration through the interior, never a ring constraint.
    vec3 orbit = vec3(-relative.y, relative.x, relative.x * 0.12);
    target += orbit * (0.075 * weights.y + 0.025 * weights.z);
    float limit = dot(weights, vec4(540.0 + layer * 105.0, 360.0 + layer * 300.0,
        440.0 + layer * 290.0, 2800.0));
    vec3 confinement = -radial * max(0.0, radius - limit) * 0.42;
    vec3 force = confinement + cosmic_event_force(state.position, u) * 5.2;
    force.z -= max(0.0, state.position.z + 650.0) * 1.4;
    force.z += max(0.0, -3400.0 - state.position.z) * 0.7;
    force += radial * u.Beat * (180.0 + u.Bass * 320.0) * (1.0 - layer * 0.13);
    force += cosmic_curl(state.position * 0.004, u.Flow + layer) * u.Beat *
        (u.Mid * 110.0 + u.Treble * 150.0) * (0.35 + layer * 0.16);
    // Exact exponential velocity relaxation, then semi-implicit position update.
    float response = 1.0 - exp(-p.delta * 1.6);
    state.velocity += (target - state.velocity) * response + force * p.delta;
    state.velocity *= min(1.0, 640.0 / max(length(state.velocity), 1.0));
    state.position += state.velocity * p.delta;
    state.age += p.delta;
    return state;
}

ChibiParticle vortex(ChibiParticleInput p, ChibiParameters u)
{
    vec3 random = cosmic_random(p.index, p.seed, u.Variation);
    vec4 weights = cosmic_stage_weights(u.Style);
    float life = cloud_life(random);
    float envelope = smoothstep(0.0, 0.65, p.age) * (1.0 - smoothstep(life - 2.2, life, p.age));
    float quiet = cosmic_board_quiet(cosmic_board_projection(p.position, u.Eye), u.Duet);
    quiet *= 1.0 - smoothstep(-420.0, -180.0, p.position.z);
    float foreground = 1.0 - smoothstep(1000.0, 3100.0, -p.position.z);
    float size = 0.42 + pow(random.z, 4.0) * 0.50;
    float accent = pow(random.x, 12.0);
    float speed = length(p.velocity);
    float wake = cosmic_event_light(p.position, u);
    float activity = clamp(u.Energy * 0.25 + u.Beat * 0.42 + wake * 0.85, 0.0, 1.0);
    float light = 0.30 + foreground * 0.12 + activity * 1.2 + u.Treble * accent * 0.35;
    ChibiParticle result = chibi_particle();
    result.position = p.position;
    result.size = size * (0.86 + u.Treble * accent * 0.45) * sqrt(envelope);
    vec3 tint = cosmic_tint(random, p.position, weights, u);
    tint = mix(tint, vec3(0.50, 0.95, 1.12), wake * 0.32);
    result.color = vec4(tint * light,
        envelope * quiet * (0.10 + 0.30 * random.y) * (0.5 + activity * 1.5));
    // Most grains are round solids. Only rare fast foreground grains stretch,
    // in their actual direction of travel, retaining a sharp visible center.
    float streak = accent * foreground * clamp(speed / 220.0, 0.0, 2.0);
    result.scale = vec3(1.0 + streak * 2.0, 0.9, 0.9);
    result.rotation = vec3(0.0, 0.0, atan(p.velocity.y, p.velocity.x));
    result.lighting = 0.18;
    return result;
}
