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

uniform sampler2D u_SSRTexture;
uniform sampler2D u_GNormals;
uniform sampler2DMS u_MSGNormals;
uniform sampler2DMS u_MSGMetalRough;
uniform sampler2DMS u_ComplexFragments;
uniform sampler2D u_HalfResSSRTexture;

uniform int u_Iteration;
uniform vec2 u_BlurDirection;
uniform ivec2 u_ViewportResolution;

layout(location = 0) out vec4 BlurTextureOut;

void main()
{
    if (u_Iteration == -1)
    {
        BlurTextureOut = texture(u_HalfResSSRTexture, v_TexCoords);
        return;
    }

    const ivec2 texel = ivec2(v_TexCoords * u_ViewportResolution);
    float isc = texelFetch(u_ComplexFragments, texel, 0).r;
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 1).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 2).r);
    isc = max(isc, texelFetch(u_ComplexFragments, texel, 3).r);
    const bool isComplex = isc == 1;

    int msSampleCount = 0;
    vec3 outColor = vec3(0);

    for (int i = 0; i < 1; ++i)
    {
        vec3 normal = texelFetch(u_MSGNormals, texel, i).xyz;
        const float roughness = texelFetch(u_MSGMetalRough, texel, i).g;
        const float kernel = remap(roughness, 0.0, 1.0, 5.0, 20.0);
        
        vec3 color = vec3(0);
        int its = 0;
        for (float o = -kernel; o <= kernel; o += 1.0)
        {
            const float off = o * (u_ViewportResolution.x / 256) * roughness;
            const vec2 offset = u_BlurDirection * off;
            const vec3 normalOffset = texelFetch(u_MSGNormals, texel + ivec2(offset), 3).xyz;
            const float angle = acos(dot(normal, normalOffset)) * 180.0 / Pi;
            const bool normalPass = angle < 3;

            if (normalPass)
            {
                color += texelFetch(u_SSRTexture, texel + ivec2(offset), 0).rgb;
                ++its;
            }
        }

        if (its == 0)
        {
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    color += textureOffset(u_SSRTexture, v_TexCoords, ivec2(x, y)).rgb;
                    ++its;
                }
            }
        }

        color /= its;

        outColor += color;
        ++msSampleCount;
        if (!isComplex)
            break;
    }

    BlurTextureOut = vec4(outColor / msSampleCount, 1);
}