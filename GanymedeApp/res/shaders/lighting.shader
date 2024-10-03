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

uniform mat4 u_Projection;
uniform mat4 u_ProjectionInverse;

struct PointLight
{
    mat4 u_ShadowMatrices[6];
    vec4 u_PointlightColor;
    vec3 u_PointlighWorldLocation;
    int u_LightID;
    vec4 updateShadowMap;
};

layout(std140, binding = 0) buffer PointLightDataBlock
{
    PointLight pointLights[];
};

uniform int u_PointlightCount;

uniform samplerCubeArray u_DepthCubemapTexture;

uniform sampler2DMS u_GPositions;
uniform sampler2DMS u_GNormals;
uniform sampler2DMS u_GDepths;
uniform sampler2DMS u_GAlbedo;
uniform sampler2DMS u_GMetalRough;
uniform sampler2DMS u_ComplexFragment;
uniform sampler2DMS u_GEmission;
//uniform sampler2D u_SSAOTexture;

uniform float u_ClipNear;
uniform float u_ClipFar;

#define SIGMA 30.0
#define BSIGMA 1.0
#define MSIZE 30

float normpdf(in float x, in float sigma)
{
    return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
    return 0.39894 * exp(-0.5 * dot(v, v) / (sigma * sigma)) / sigma;
}

float ShadowCalculation(int lightID, vec3 viewPos, vec3 fragPos, vec3 pointlightPos, vec3 fragNormal)
{
    float ditherPattern2[4][4] = { { 0.0f, 0.5f, 0.125f, 0.625f},
    { 0.75f, 0.22f, 0.875f, 0.375f},
    { 0.1875f, 0.6875f, 0.0625f, 0.5625},
    { 0.9375f, 0.4375f, 0.8125f, 0.3125} };

    float ditherOffset = (ditherPattern2[int(mod(v_TexCoords.x * u_ViewportResolution.x+1, 4))][int(mod(v_TexCoords.y * u_ViewportResolution.y+1, 4))]);
    float ditherOffset1 = (ditherPattern2[int(mod(v_TexCoords.x * u_ViewportResolution.x+2, 4))][int(mod(v_TexCoords.y * u_ViewportResolution.y+2, 4))]);
    float ditherOffset2 = (ditherPattern2[int(mod(v_TexCoords.x * u_ViewportResolution.x, 4))][int(mod(v_TexCoords.y * u_ViewportResolution.y, 4))]);

    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
        vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
        );

    float shadow = 0.0;
    //float bias = (1 - dot(fragNormal, normalize(pointlightPos - fragPos))) * .5;
    float bias = .2;

    vec3 fragToLight = fragPos - pointlightPos;
    float currentDepth = length(fragToLight);
    
    float offsetStrength = 0.03;

    float closestDepth = texture(u_DepthCubemapTexture, vec4(fragToLight, lightID)).r;
    closestDepth *= 1000;   // undo mapping [0;1]
    bool inShadow = currentDepth - bias > closestDepth;

    int i = 0;
    for (int numSample = 0; numSample < 10; ++numSample)
    {
        vec3 randSampleOffset = randomNormalizedVector(vec2(fragPos.xy + numSample));

        float lightDepth = texture(u_DepthCubemapTexture, vec4(fragToLight + ((sampleOffsetDirections[numSample]) * offsetStrength), lightID)).r * 1000;
        bool isFragmentInShadow = currentDepth - bias > lightDepth;

        if (isFragmentInShadow)
        {
            shadow += 1;
            ++i;
        }
    }

    if (i == 0)
    {
        i = 1;
    }

    return shadow / 10;
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

uniform samplerCubeArray u_ColorCube;

void main()
{
    ivec2 texel = ivec2(v_TexCoords * u_RenderResolution);

    float isc = texelFetch(u_ComplexFragment, texel, 0).r;
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 1).r);
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 2).r);
    isc = max(isc, texelFetch(u_ComplexFragment, texel, 3).r);

    bool isComplex = isc == 1;

    int sampleIdx = 0;
    vec4 colorout = vec4(0);
    while (sampleIdx < 4)
    {
        vec3 albedo = texelFetch(u_GAlbedo, texel, sampleIdx).xyz;
        float albedoAlpha = texelFetch(u_GAlbedo, texel, sampleIdx).a;
        vec3 position = texelFetch(u_GPositions, texel, sampleIdx).xyz;
        vec3 normal = texelFetch(u_GNormals, texel, sampleIdx).xyz;

        vec2 metalRough = texelFetch(u_GMetalRough, texel, sampleIdx).xy;
        //float metallic = metalRough.x;
        float metallic = 0;
        float roughness = metalRough.y;
        float spec = 0.5;

        vec3 N = normalize(normal);
        vec3 V = normalize(u_ViewPos - position);

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        // reflectance equation
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

            float ss = 1;
            
            if (pointLights[i].u_LightID > -1)
            {
                ss = 1 - ShadowCalculation(pointLights[i].u_LightID, u_ViewPos, position, pointLights[i].u_PointlighWorldLocation, normal);
                if (ss == 0)
                    continue;
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

            specular *= ss;

            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);
            Lo += (((kD * albedo / Pi + specular) * radiance * NdotL) * ss);
        }

        vec3 ambient = vec3(0.0) * albedo;
        vec3 finalColor = ambient + Lo;
   
        vec3 emission = texelFetch(u_GEmission, texel, sampleIdx).rgb * 100;
        finalColor += emission;

        //finalColor *= pow(ssa,3);

        // Get rid of banding in color and volumetric
        

        colorout += vec4(finalColor.rgb, 1);
        ++sampleIdx;
        if (!isComplex)
            break;
    }

    //finalColor = finalColor / (finalColor + vec3(1.0));
    //finalColor = pow(finalColor, vec3(1.0 / 1.0));

    FragColor = colorout / sampleIdx;
    highp vec2 coordinates = gl_FragCoord.xy / u_ViewportResolution;
    FragColor += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, randomFloat(coordinates));
}