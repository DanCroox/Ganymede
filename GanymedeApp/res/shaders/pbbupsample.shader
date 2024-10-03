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


// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
uniform sampler2D srcTexture;
uniform int srcTextureMipLevel;
uniform float filterRadius;

in vec2 TexCoords;
layout(location = 0) out vec4 upsample;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = textureLod(srcTexture, vec2(TexCoords.x - x, TexCoords.y + y), srcTextureMipLevel).rgb;
    vec3 b = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y + y), srcTextureMipLevel).rgb;
    vec3 c = textureLod(srcTexture, vec2(TexCoords.x + x, TexCoords.y + y), srcTextureMipLevel).rgb;
    vec3 d = textureLod(srcTexture, vec2(TexCoords.x - x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 e = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 f = textureLod(srcTexture, vec2(TexCoords.x + x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 g = textureLod(srcTexture, vec2(TexCoords.x - x, TexCoords.y - y), srcTextureMipLevel).rgb;
    vec3 h = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y - y), srcTextureMipLevel).rgb;
    vec3 i = textureLod(srcTexture, vec2(TexCoords.x + x, TexCoords.y - y), srcTextureMipLevel).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |

    vec3 oucolor = e * 4.0;
    oucolor += (b + d + f + h) * 2.0;
    oucolor += (a + c + g + i);
    oucolor *= 1.0 / 16.0;

    upsample = vec4(oucolor, 1);
}