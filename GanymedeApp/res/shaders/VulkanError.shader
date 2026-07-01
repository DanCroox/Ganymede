#shader vertex
#version 450

// MVP-Matrix (SSBO)
layout(set = 0, binding = 0) buffer SSBO {
    mat4 mvp;
} ssbo;

// Eingabe aus Vertex-Buffer
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inColor;
layout(location = 6) in ivec4 inBoneIndex;
layout(location = 7) in vec4 inBoneWeight;

// Ausgabe an Fragment-Shader
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

layout(push_constant) uniform PushBlock {
    mat4 model;
} pc;

void main()
{
    gl_Position = pc.model * vec4(inPosition, 1.0);
    fragColor = inNormal;
    fragUV = inUV;
}

#shader fragment
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(.5, 0.0, 0.0, 1.0);
}