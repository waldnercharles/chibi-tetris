// Shared deterministic samples. Both entry points retain the original hashes.
#ifndef REACTION_COMMON_GLSL
#define REACTION_COMMON_GLSL

vec3 reaction_random(float index, float seed)
{
    return fract(sin(vec3(
        index * 127.1 + seed,
        index * 269.5 + seed * 2.0,
        index * 311.7 + seed * 3.0
    )) * 43758.5453);
}

float reaction_quantile(float index, float seed)
{
    return fract(sin(index * 71.37 + seed * 5.0) * 19417.345);
}

// Preserve the authored float selector, including its half-unit boundary.
float reaction_pattern(float pattern, float value)
{
    return 1.0 - step(0.5, abs(pattern - value));
}

float reaction_birth(float span, float quantile)
{
    return span * (exp(log(65.0) * quantile) - 1.0) / 64.0;
}

float reaction_drag(float rate, float seconds)
{
    return (1.0 - exp(-rate * seconds)) / rate;
}

#endif
