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

layout(location = 0) out vec4 Color;

out vec4 FragColor;

in vec2 v_TexCoords;

struct PointLight
{
    mat4 u_ShadowMatrices[6];
    vec4 lightColor;
    vec3 lightPos;
    int u_LightID;
    vec4 updateShadowMap;
};

layout(std140, binding = 0) buffer PointLightDataBlock
{
    PointLight pointLights[];
};

uniform int u_PointlightCount;

uniform samplerCubeArray u_DepthCubemapTexture;

uniform sampler2D u_GPositions;
uniform sampler2D u_GDepths;
uniform vec3 u_ViewPos;
uniform ivec2 u_VolumetricRenderResolution;


float G_SCATTERING = .25;

float ComputeScattering(float lightDotView)
{
    float result = 1.0f - G_SCATTERING * G_SCATTERING;
    result /= (4.0f * Pi * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
    return result;
}

float GetBayerFromCoordLevel2(vec2 pixelpos)
{
    float finalBayer = 0.0;
    float finalDivisor = 0.0;
    float layerMult = 1.0;

    for (float bayerLevel = float(4); bayerLevel >= 1.0; bayerLevel--)
    {
        float bayerSize = exp2(bayerLevel) * 0.5;
        vec2 bayercoord = mod(floor(pixelpos.xy / bayerSize), 2.0);
        layerMult *= 4.0;

        float line0202 = bayercoord.x * 2.0;

        finalBayer += mix(line0202, 3.0 - line0202, bayercoord.y) / 3.0 * layerMult;
        finalDivisor += layerMult;
    }

    return finalBayer / finalDivisor;
}

float VolumetricLight(vec3 fragPos, int lightIndex, int lightID)
{
    vec3 startPos = u_ViewPos;
    vec3 endPosition = fragPos;
    vec3 rayVector = endPosition - startPos;

    float rayLength = length(rayVector);
    vec3 rayDirection = normalize(rayVector);

    float maxDistance = 50;
    float numSteps = 20;
    //vec3 step = rayDirection * (rayLength / numSteps);
    vec3 step = rayDirection * (maxDistance / numSteps);

    vec3 lightPos = pointLights[lightIndex].lightPos;

    vec3 currentPos = startPos;

    highp vec2 coordinates = gl_FragCoord.xy / u_VolumetricRenderResolution;
    float ff= randomFloat(coordinates);

    float ditherOffset = GetBayerFromCoordLevel2(vec2(v_TexCoords.x * u_VolumetricRenderResolution.x, v_TexCoords.y * u_VolumetricRenderResolution.y));
    //ditherOffset = ff;

    float volumeAccumulation = 0;
    for (int i = 0; i < numSteps; ++i)
    {
       vec3 _currentPos = currentPos + (step * ditherOffset);
       //vec3 _currentPos = currentPos;

        vec3 currentPosToLight = _currentPos - lightPos;
        float currentDepth = length(currentPosToLight);
        float closestDepth = texture(u_DepthCubemapTexture, vec4(currentPosToLight, lightID)).r;
        closestDepth *= 1000;   // undo mapping [0;1]

        
        if (length(_currentPos - u_ViewPos) >= length(fragPos - u_ViewPos))
        {
            break;
        }
        

        if (currentDepth < closestDepth)
        {
            float lightStrength = length(pointLights[lightIndex].lightColor);
            float distance = length(_currentPos - lightPos);

            float attenuation = 1 / mix((distance * distance), distance, .5);
            //float attenuation = 1 / (distance * distance);
            //float attenuation = 1 / (distance);
 
            vec3 lightToPos = normalize(_currentPos - lightPos);

            float contrib = (attenuation) * lightStrength;
            volumeAccumulation += contrib * .12;
        }
        currentPos += step;
    }
    return max(0,volumeAccumulation / (numSteps));
}

void main()
{
    vec3 position = texture(u_GPositions, v_TexCoords).xyz;

    vec4 Lo = vec4(0.0);
    Lo.a = 1;

    for (int i = 0; i < u_PointlightCount; ++i)
    {
        if (pointLights[i].u_LightID == -1)
            continue;

        float volume = VolumetricLight(position, i, pointLights[i].u_LightID);
        if (length(pointLights[i].lightColor.xyz) > 0)
        {
            //Correct color gamma
            vec3 lightColor = pow(pointLights[i].lightColor.xyz, vec3(1.0 / 2.2));
            Lo.rgb += (normalize(lightColor) * volume);

            //Lo.a += volume;
        }

    }

    Color = Lo;
}