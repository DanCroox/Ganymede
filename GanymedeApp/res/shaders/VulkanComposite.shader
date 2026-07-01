#shader vertex
#version 450

layout(location = 0) in vec3 inPosition;
vec2 positions[3] = vec2[](
	vec2(-1.0, -3.0),
	vec2(3.0, 1.0),
	vec2(-1.0, 1.0)
);

layout(location = 0) out vec2 uv;

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	uv = gl_Position.xy * 0.5 + 0.5;
		uv.y = 1.0 - uv.y; // Flip Y

}

#shader fragment
#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

struct CompositeMaterialData
{
	uint PositionIdx;
	uint NormalIdx;
	uint AlbedoIdx;
	uint RoughnessMetalIdx;

	uint EmissionIdx;
	uint ComplexFrgIdx;
	uint DepthIdx;
	uint _PadA;

	vec4 _PadG[14];
};

layout(set = 0, binding = 0) uniform sampler2D textures[];
layout(set = 0, binding = 1) buffer MaterialDataBuffer {
	CompositeMaterialData materialData[];
} ssbos[];

layout(push_constant) uniform PushBlock {
	uint m_BufferIndex;
	uint m_DataIndex;
	uint _PadA;
	uint _PadB;
} pc;

float LinearizeDepth(float z, float near, float far)
{
	return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
	CompositeMaterialData matData = ssbos[nonuniformEXT(pc.m_BufferIndex)].materialData[pc.m_DataIndex];

	float screenPosX = gl_FragCoord.x;

	if (screenPosX < 320)
	{
		outColor = texture(textures[nonuniformEXT(matData.PositionIdx)], uv);
	}
	else if (screenPosX < 640)
	{
		outColor = texture(textures[nonuniformEXT(matData.NormalIdx)], uv);
	}
	else if (screenPosX < 960)
	{
		outColor = texture(textures[nonuniformEXT(matData.AlbedoIdx)], uv);
	}
	else if (screenPosX < 1280)
	{
		outColor = texture(textures[nonuniformEXT(matData.RoughnessMetalIdx)], uv);
	}
	else if (screenPosX < 1600)
	{
		float col = texture(textures[nonuniformEXT(matData.ComplexFrgIdx)], uv).x;
		outColor = vec4(col, col, col, 1.0);
	}
	else
	{
		float depth = texture(textures[nonuniformEXT(matData.DepthIdx)], uv).x;
		float linDepth = LinearizeDepth(depth, 0.1, 1000.0);
		outColor = vec4(linDepth, linDepth, linDepth, 1.0);
	}
}