#shader vertex
#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    TexCoords = texCoords;
}  

#shader fragment
#version 460 core

#include "common.h"

in vec2 TexCoords;

uniform sampler2D m_LightingPass;
uniform sampler2D m_GNormals;
uniform sampler2D m_SSRPass;
uniform sampler2D u_VolumetricLight;
uniform sampler2D u_GDepth;
uniform sampler2D u_GEmission;
uniform sampler2D u_PBBTexture;
uniform sampler2D m_GMetalRough;
uniform sampler2D u_GHalfDepthMax;
uniform sampler2D u_SSRNormals;
uniform sampler2D u_OcclusionQueryDebugTexture;

uniform sampler2D u_GAlbedo;
uniform sampler2DMS u_ComplexFragments;
uniform sampler2DMS u_MSGDepth;

uniform ivec2 u_RenderResolution;
uniform ivec2 u_ViewportResolution;

out vec4 FragColor;


uniform float sigmaS;
uniform float sigmaR;

uniform float u_GameTime;

float zNear = 0.01;
float zFar = 1000;

float spatialSigma = 30.0;
float rangeSigma = .6;

float gaussian(int x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (sqrt(2.0 * 3.14159265358979323846) * sigma);
}

vec3 GausBlur(sampler2D inputTexture)
{
    vec2 textSize = textureSize(inputTexture, 0);

    // Calculate the Gaussian kernel weights
    const int kernelSize = 20;
    float sigma = .1;
    float weights[kernelSize];
    float sum = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        weights[i] = gaussian(i - kernelSize / 2, sigma);
        sum += weights[i];
    }
    for (int i = 0; i < kernelSize; ++i)
    {
        weights[i] /= sum;
    }

    // Apply horizontal blur
    vec3 outColor = texture(inputTexture, TexCoords).rgb * weights[0];
    for (int i = 1; i < kernelSize; ++i)
    {
        vec2 offset = vec2(float(i), 0.0) / textSize;
        outColor += texture(inputTexture, TexCoords + offset).rgb * weights[i];
        outColor += texture(inputTexture, TexCoords - offset).rgb * weights[i];
    }

    // Apply vertical blur
    for (int i = 1; i < kernelSize; ++i)
    {
        vec2 offset = vec2(0.0, float(i)) / textSize;
        outColor += texture(inputTexture, TexCoords + offset).rgb * weights[i];
        outColor += texture(inputTexture, TexCoords - offset).rgb * weights[i];
    }

    return outColor;
}

void main()
{
    ivec2 texel = ivec2(TexCoords * u_RenderResolution);

    float isc = texelFetch(u_ComplexFragments, texel, 0).r;
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 1).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 2).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 3).r);

    bool isComplex = isc == 1;

    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 8; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 4; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = 8; // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}

    vec2 Radius = vec2(Size) / u_ViewportResolution;
    vec3 upsampledVolumetric = vec3(0);
    int numIterations = 0;
    int sampleIterations = 0;
    for (int ss = 0; ss < 4; ++ss)
    {
        vec3 upsampledVolumetricTMP = vec3(0);
        float depth = texelFetch(u_MSGDepth, texel, ss).x;
        depth = linearDepth(depth, zNear, zFar);
        int numIterationsTMP = 0;

        for (float d = 0.0; d < Pi2; d += Pi2 / Directions)
        {
            for (float i = 1.0 / Quality; i <= 1.0; i += 1.0 / Quality)
            {
                vec2 uvOffset = TexCoords + vec2(cos(d), sin(d)) * Radius * i;
                float depthOffset = texelFetch(u_MSGDepth, ivec2(uvOffset * u_RenderResolution), ss).x;
                depthOffset = linearDepth(depthOffset, zNear, zFar);
                if ((depthOffset - depth) < (depth * .05) && (depthOffset - depth) >= -(depth * .05))
                {
                    upsampledVolumetricTMP += texture(u_VolumetricLight, uvOffset).rgb;
                    ++numIterationsTMP;
                }
            }
        }

        if (numIterationsTMP > 0)
            upsampledVolumetricTMP /= numIterationsTMP;

        upsampledVolumetric += upsampledVolumetricTMP;
        numIterations += numIterationsTMP;

        ++sampleIterations;
        if (!isComplex)
            break;
    }

    upsampledVolumetric /= sampleIterations;
    upsampledVolumetric *= 50;
   
    highp vec2 coordinates = gl_FragCoord.xy / u_ViewportResolution;
    upsampledVolumetric += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, randomFloat(coordinates));

    vec3 lighting = texture(m_LightingPass, TexCoords).rgb;
    vec3 emission = texture(u_PBBTexture, TexCoords).rgb;
    vec3 ssr = texture(m_SSRPass, TexCoords).rgb;

    vec3 colors = lighting + upsampledVolumetric + (ssr) + (emission * .04);
    
    colors = colors / (colors + vec3(1.0));
    colors = pow(colors, vec3(1.0 / 1.0));

    colors = colors + (texture(u_OcclusionQueryDebugTexture, TexCoords).rgb * texture(u_OcclusionQueryDebugTexture, TexCoords).a);
    FragColor = vec4(colors, 1);
}