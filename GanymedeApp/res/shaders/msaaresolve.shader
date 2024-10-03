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

in vec2 TexCoords;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gMetalRough;
layout(location = 4) out vec4 gEmissive;

uniform sampler2DMS m_Albedos;
uniform sampler2DMS m_Depth;
uniform sampler2DMS m_MetalRough;
uniform sampler2DMS m_Normals;
uniform sampler2DMS m_Positions;
uniform sampler2DMS m_Emissive;

void main()
{
    ivec2 pos = ivec2(gl_FragCoord.xy);
    // Accumulate depth values from all samples
   
    vec4 albedoAccum = vec4(0.0);
    float depthAccum = 0.0;
    vec4 metalAccum = vec4(0.0);
    vec4 normalAccum = vec4(0.0);
    vec4 posAccum = vec4(0.0);
    vec4 emissiveAccum = vec4(0.0);
    for (int i = 0; i < 4; ++i) {
        albedoAccum += texelFetch(m_Albedos, pos, i);
        //depthAccum += texelFetch(m_Depth, pos, i).r;
        metalAccum += texelFetch(m_MetalRough, pos, i);
        normalAccum += texelFetch(m_Normals, pos, i);
        posAccum += texelFetch(m_Positions, pos, i);
        emissiveAccum += texelFetch(m_Emissive, pos, i);
    }

   gAlbedo = albedoAccum / 4;
   //gl_FragDepth = depthAccum / 4;
   gl_FragDepth = texelFetch(m_Depth, pos, 0).r;
   gMetalRough = metalAccum / 4;
   gNormal = normalAccum / 4;
   gPosition = posAccum / 4;
   gEmissive = emissiveAccum / 4;
}