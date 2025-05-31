#shader vertex
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    v_TexCoords = texCoords;
}

#shader fragment
#version 460 core

#include "common.h"

out vec4 FragColor;

in vec2 v_TexCoords;

uniform vec3 u_ViewPos;
uniform ivec2 u_ViewportResolution;
uniform ivec2 u_RenderResolution;

struct PointLight
{
    vec4 u_PointlightColor;
    vec3 u_PointlighWorldLocation;
    int u_LightID;
};
layout(std140, binding = 0) buffer PointLightDataBlock { PointLight pointLights[]; };

uniform int u_PointlightCount;

uniform samplerCubeArray u_DepthCubemapTexture;

uniform sampler2DMS u_GPositions;
uniform sampler2DMS u_GNormals;
uniform sampler2DMS u_GDepths;
uniform sampler2DMS u_GAlbedo;
uniform sampler2DMS u_GMetalRough;
uniform sampler2DMS u_ComplexFragment;
uniform sampler2DMS u_GEmission;

#define SIGMA 30.0
#define BSIGMA 1.0
#define MSIZE 30

bool IsInShadow(int lightID, vec3 fragPos, vec3 pointlightPos, vec3 fragNormal)
{
    float bias = 0.2;

    vec3 fragToLight = fragPos - pointlightPos;
    float currentDepth = length(fragToLight);

    float closestDepth = texture(u_DepthCubemapTexture, vec4(normalize(fragToLight), lightID)).r;
    closestDepth *= 1000.0;   // undo mapping [0;1]
    return (currentDepth - bias) > closestDepth;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = Pi * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    ivec2 texel = ivec2(v_TexCoords * u_RenderResolution);

    float isc = texelFetch(u_ComplexFragment, texel, 0).r;
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 1).r);
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 2).r);
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 3).r);

    bool isComplex = isc == 1;

    int sampleIdx = 0;
    vec3 colorout = vec3(0);
    while (sampleIdx < 4)
    {
        vec3 albedo = texelFetch(u_GAlbedo, texel, sampleIdx).xyz;
        float albedoAlpha = texelFetch(u_GAlbedo, texel, sampleIdx).a;
        vec3 position = texelFetch(u_GPositions, texel, sampleIdx).xyz;
        vec3 normal = texelFetch(u_GNormals, texel, sampleIdx).xyz;

        vec2 metalRough = texelFetch(u_GMetalRough, texel, sampleIdx).xy;
        float metallic = metalRough.x;
        float roughness = metalRough.y;
        float spec = 0.5;

        vec3 N = normalize(normal);
        vec3 V = normalize(u_ViewPos - position);

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        vec3 Lo = vec3(0.0);
        for (int i = 0; i < u_PointlightCount; ++i)
        {
            PointLight pLight = pointLights[i];
            vec4 pLightColor = pLight.u_PointlightColor;

            float pLightDistance = length(pLight.u_PointlighWorldLocation - position);
            float kl = 0.09;
            float kq = 0.032;
            float attenuation = 1.0 / (1.0 + (kl * pLightDistance) + (kq * pLightDistance * pLightDistance));

            vec3 radiance = (pointLights[i].u_PointlightColor.rgb) * attenuation;
            float energy = radiance.r + radiance.g + radiance.b;
            
            if (pointLights[i].u_LightID > -1)
            {
                if (IsInShadow(pointLights[i].u_LightID, position, pointLights[i].u_PointlighWorldLocation, normal))
                {
                    continue;
                }
            }

            // calculate per-light radiance
            vec3 L = normalize(pointLights[i].u_PointlighWorldLocation - position);
            vec3 H = normalize(V + L);

            // cook-torrance brdf
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);
            Lo += (((kD * albedo / Pi + specular) * radiance * NdotL));
        }

        vec3 emission = texelFetch(u_GEmission, texel, sampleIdx).rgb * 100;

        colorout += Lo + emission;

        ++sampleIdx;
        if (!isComplex)
            break;
    }

    vec3 finalLinearColor = colorout / sampleIdx;

    // Simple filmic tonemapping
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    vec3 x = max(vec3(0.0), finalLinearColor - E);
    vec3 filmic = (x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F) - E / F;
    vec3 tonemapped = clamp(filmic, 0.0, 1.0);

    vec3 finalsRGBColor = pow(tonemapped, vec3(1.0 / 2.2));

    // Eliminate banding by using sublte noise
    highp vec2 coordinates = gl_FragCoord.xy / u_ViewportResolution;
    FragColor = vec4(finalsRGBColor + mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, randomFloat(coordinates)), 1);
}