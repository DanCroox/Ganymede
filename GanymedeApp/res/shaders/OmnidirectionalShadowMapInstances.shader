#shader vertex
// core = does not allow to use deprecated functions
#version 460 core
#extension GL_NV_viewport_array2 : require

layout(location = 0) in vec3 Position;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;
layout(location = 7) in uint InstanceDataIndex;

struct InstanceDataCubemapShadowMapping
{
	mat4 m_M;
	mat4 m_MVP;
	uvec4 m_Attribs;
};

layout(std140, binding = 4) buffer InstanceDataBlock
{
	InstanceDataCubemapShadowMapping InstanceDatas[];
};

struct PointLight
{
	vec3 lightPos;
	int u_LightIDs;
};

layout(std140, binding = 1) buffer PointLightDataBlock
{
	PointLight pointLights[];
};

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

uniform mat4 u_Projection;

out vec3 v_FragPos;
out vec3 v_PointlightWorldPos;

void main()
{
	InstanceDataCubemapShadowMapping ssboInstanceData = InstanceDatas[InstanceDataIndex];
	mat4 ss_MVP = ssboInstanceData.m_MVP;
	mat4 ss_M = ssboInstanceData.m_M;

	// x = LightIndex, y = FaceIndex we will render to. Data are passed as vec2 to have more vertex attributes
	uint indx = ssboInstanceData.m_Attribs.x;
	uint findx = ssboInstanceData.m_Attribs.y;

	PointLight pl = pointLights[indx];

	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 ppos = vec4(Position, 1);

	if (isAnimated)
	{
		uint boneDataOffset = ssboInstanceData.m_Attribs.z;
		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		ppos = BoneTransform * ppos;
	}

	gl_Position = ss_MVP * ppos;
	v_FragPos = (ss_M * ppos).xyz;

	v_PointlightWorldPos = pl.lightPos;
	gl_Layer = (pl.u_LightIDs * 6) + int(findx);
};


#shader fragment
#version 460 core

in vec3 v_FragPos;
in vec3 v_PointlightWorldPos;

uniform float far_plane;

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(v_FragPos.xyz - v_PointlightWorldPos);

	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / 1000.0;

	// write this as modified depth
	gl_FragDepth = lightDistance;
}