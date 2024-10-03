const float Pi = 3.1415926535897932384626433832795;
const float Pi2 = Pi * 2;
const highp float NOISE_GRANULARITY = .5 / 255.0;

float saturate(float value)
{
    return clamp(value, 0.0, 1.0);
}

vec3 saturate(vec3 value)
{
    return clamp(value, 0.0, 1.0);
}

float CheapContrast(float value, float contrast)
{
    return saturate(mix(0.5, value, contrast));
}

vec3 CheapContrast(vec3 value, float contrast)
{
    return vec3(
        CheapContrast(value.r, contrast),
        CheapContrast(value.g, contrast),
        CheapContrast(value.b, contrast)
    );
}

float remap(float x, float a, float b, float c, float d) {
    return (x - a) / (b - a) * (d - c) + c;
}

float linearDepth(float depthSample, float zNear, float zFar)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

highp float randomFloat(highp vec2 coords) {
    return fract(sin(dot(coords.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 randomNormalizedVector(vec2 seed) {
    vec3 randomVec = vec3(
        fract(sin(dot(seed, vec2(127.1, 311.7))) * 43758.5453),
        fract(sin(dot(seed, vec2(269.5, 183.3))) * 43758.5453),
        fract(sin(dot(seed, vec2(419.2, 371.9))) * 43758.5453)
    );
    return normalize(randomVec);
}

float colorDifference(vec3 color1, vec3 color2) {
    // Calculate the difference between each component of the two colors
    vec3 delta = color1 - color2;

    float distance = length(color1 - color2);

    return distance / sqrt(3);

}