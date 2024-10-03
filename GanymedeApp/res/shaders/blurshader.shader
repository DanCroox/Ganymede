#shader vertex
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
	// Full screen quad
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    v_TexCoords = texCoords;
}

#shader fragment
#version 460 core

#include "common.h"

in vec2 v_TexCoords;

uniform sampler2D u_Depth;
uniform sampler2D u_InputTexture;
uniform sampler2D u_gNormal;
uniform sampler2D u_gMetalRoughness;
uniform float u_IsSSRBlur;
uniform int u_Horizontal;
layout(location = 0) out vec4 BlurTextureOut;

uniform sampler2DMS u_ComplexFragments;
uniform sampler2DMS u_MSGDepth;
uniform sampler2DMS u_MSGNormal;

uniform ivec2 u_RenderResolution;
uniform ivec2 u_ViewportResolution;

uniform float u_ClipNear;
uniform float u_ClipFar;

void main()
{
    ivec2 texel = ivec2(v_TexCoords * u_ViewportResolution);
    float isc = texelFetch(u_ComplexFragments, texel, 0).r;
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 1).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 2).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 3).r);
    bool isComplex = isc == 1;

    int msSampleCount = 0;
    vec4 outColor = vec4(0);
    for (int i = 0; i < 4; ++i)
    {
        int samples = 6;
        int its = 0;

        float depth = texelFetch(u_MSGDepth, texel, i).x;
        depth = linearDepth(depth, u_ClipNear, u_ClipFar);

        vec2 off = vec2(0);
        if (u_Horizontal == 1)
            off = vec2(1, 0);
        else
            off = vec2(0, 1);

        bool normalPass = true;
        vec3 normal = vec3(0);
        float strength = 5;
        float roughness = 0;
        if (u_IsSSRBlur == 1)
        {
            roughness = texture(u_gMetalRoughness, v_TexCoords).g;
            strength = roughness * 30;
            normal = texelFetch(u_MSGNormal, texel, i).xyz;
        }

        vec4 col = vec4(0);
        for (float y = -samples; y <= samples; ++y)
        {
            vec2 offset = (vec2(y, y) * off) * strength;
            offset /= (vec2(u_ViewportResolution.x, u_ViewportResolution.y));
            float depthOffset = texelFetch(u_MSGDepth, ivec2((v_TexCoords + offset) * u_ViewportResolution), i).x;
            depthOffset = linearDepth(depthOffset, u_ClipNear, u_ClipFar);

            bool depthPass;

            if (u_IsSSRBlur == 1)
            {
                vec3 normalOffset = texelFetch(u_MSGNormal, ivec2((v_TexCoords + offset) * u_ViewportResolution), i).xyz;
                float angle = acos(dot(normal, normalOffset)) * 180.0 / Pi;
                normalPass = angle < 5;
                depthPass = normalPass;
            }
            else
            {
                depthPass = ((depthOffset - depth) < (depth * .1) && (depthOffset - depth) >= -(depth * .1));
            }

            if (depthPass)
            {
                col += texture(u_InputTexture, v_TexCoords + offset);
                ++its;
            }
        }

        col /= its;

        outColor += col;
        ++msSampleCount;
        if (!isComplex)
            break;
    }

    BlurTextureOut = outColor / msSampleCount;
}