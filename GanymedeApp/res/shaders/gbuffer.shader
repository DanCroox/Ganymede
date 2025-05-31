#shader vertex
#version 460 core

#include "Compute/commoncompute.h"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;
out mat3 v_TBN;

layout(std140, binding = 2) buffer BonesDataBlock { mat4 bones[]; };

void main()
{
	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 vertexPosition = vec4(Position, 1);

	InstanceData ssboInstanceData = ssbo_InstanceData[gl_BaseInstance + gl_InstanceID];
	
	mat4 ss_MV = ssbo_RenderViews[ssboInstanceData.m_ViewID].m_Transform * ssboInstanceData.m_Transform;
	mat4 ss_M = ssboInstanceData.m_Transform;

	if (isAnimated)
	{
		uint boneDataOffset = ssbo_EntityData[ssboInstanceData.m_EntityDataIndex].m_AnimationDataOffset;

		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		vertexPosition = BoneTransform * vertexPosition;
	}

	gl_Position = (ssbo_RenderViews[ssboInstanceData.m_ViewID].m_Projection * ss_MV) * vertexPosition;
	v_Normal = mat3(transpose(inverse(ss_M))) * Normal;
	v_TexCoords = TexCoords;
	v_FragPos = (ss_M * vertexPosition).xyz;

	mat3 m = mat3(ss_M);
	vec3 T = normalize(m * Tangent);
	vec3 N = normalize(m * Normal);
	vec3 B = normalize(m * Bitangent);

	v_TBN = mat3(T, B, N);
};


#shader fragment
#version 460 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gMetalRough;
layout(location = 4) out vec4 gEmission;
layout(location = 5) out vec4 gComplexFragment;

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_Normal;
in mat3 v_TBN;

uniform vec3 u_BaseColor;
uniform vec3 u_EmissiveColor;
uniform float u_Roughness;
uniform float u_Metalness;

uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform sampler2D u_Texture4;

void main()
{
	gAlbedo = texture(u_Texture0, v_TexCoords) * vec4(u_BaseColor, 1);
	if (gAlbedo.a < 1.0)
		discard;

	gPosition = vec4(v_FragPos, 1);
	vec3 normal = texture(u_Texture1, v_TexCoords).xyz;

	normal = normal * 2.0 - 1.0;
	gNormal = vec4(normalize(v_TBN * normal), 1);
	
	// Store "complex" fragments (less than max number of samples) -> Used for MSAA
	if (bitCount(gl_SampleMaskIn[0]) != 4)
		gComplexFragment = vec4(1);
	else
		gComplexFragment = vec4(0, 0, 0, 1);

	//TODO: optimize and merge roughness and metalness during asset loading into one texture
	// Metal and roughness
	gMetalRough.r = texture(u_Texture3, v_TexCoords).r * u_Metalness;
	
	gMetalRough.g = texture(u_Texture2, v_TexCoords).g * u_Roughness;
	gMetalRough.a = 1;

	gEmission = texture(u_Texture4, v_TexCoords) * vec4(u_EmissiveColor, 1);
}