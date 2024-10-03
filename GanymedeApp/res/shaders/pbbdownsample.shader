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

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
uniform sampler2D srcTexture;
uniform int srcTextureMipLevel;
uniform ivec2 srcResolution;

in vec2 TexCoords;
layout(location = 0) out vec4 downsample;

void main()
{
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    
    vec3 a = textureLod(srcTexture, vec2(TexCoords.x - 2 * x, TexCoords.y + 2 * y), srcTextureMipLevel).rgb;
    vec3 b = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y + 2 * y), srcTextureMipLevel).rgb;
    vec3 c = textureLod(srcTexture, vec2(TexCoords.x + 2 * x, TexCoords.y + 2 * y), srcTextureMipLevel).rgb;
    vec3 d = textureLod(srcTexture, vec2(TexCoords.x - 2 * x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 e = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 f = textureLod(srcTexture, vec2(TexCoords.x + 2 * x, TexCoords.y), srcTextureMipLevel).rgb;
    vec3 g = textureLod(srcTexture, vec2(TexCoords.x - 2 * x, TexCoords.y - 2 * y), srcTextureMipLevel).rgb;
    vec3 h = textureLod(srcTexture, vec2(TexCoords.x, TexCoords.y - 2 * y), srcTextureMipLevel).rgb;
    vec3 i = textureLod(srcTexture, vec2(TexCoords.x + 2 * x, TexCoords.y - 2 * y), srcTextureMipLevel).rgb;
    vec3 j = textureLod(srcTexture, vec2(TexCoords.x - x, TexCoords.y + y), srcTextureMipLevel).rgb;
    vec3 k = textureLod(srcTexture, vec2(TexCoords.x + x, TexCoords.y + y), srcTextureMipLevel).rgb;
    vec3 l = textureLod(srcTexture, vec2(TexCoords.x - x, TexCoords.y - y), srcTextureMipLevel).rgb;
    vec3 m = textureLod(srcTexture, vec2(TexCoords.x + x, TexCoords.y - y), srcTextureMipLevel).rgb;
    

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    vec3 oucolor = vec3(0);
    oucolor = e * 0.125;
    oucolor += (a + c + g + i) * 0.03125;
    oucolor += (b + d + f + h) * 0.0625;
    oucolor += (j + k + l + m) * 0.125;

    //oucolor = clamp(oucolor, vec3(0), vec3(1));

    downsample = vec4(oucolor, 1);
}