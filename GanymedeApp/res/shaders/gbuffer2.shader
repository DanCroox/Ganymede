#shader vertex
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;
layout(location = 7) in mat4 u_MV;
layout(location = 11) in mat4 u_M;
layout(location = 15) in vec3 instanceData;

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

uniform mat4 u_Projection;

out vec3 v_WorldPosition;
out vec3 v_WorldNormal;
out vec2 v_TexCoords;

void main()
{
	bool isAnimated = (BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3]) > 0;
	vec4 vertexPosition = vec4(Position, 1);
	if (isAnimated)
	{
		int boneDataOffset = int(instanceData.z);
		
		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		vertexPosition = BoneTransform * vertexPosition;
	}

	gl_Position = (u_Projection * u_MV) * vertexPosition;

	v_WorldPosition = (u_M * vertexPosition).xyz;
	v_WorldNormal = mat3(transpose(inverse(u_M))) * Normal;
	v_TexCoords = TexCoords;
};


#shader fragment
#version 460 core

in vec3 v_WorldPosition;
in vec3 v_WorldNormal;
in vec2 v_TexCoords;

layout(location = 0) out vec4 gWorldPosition;
layout(location = 1) out vec4 gWorldNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gMetalRough;
layout(location = 4) out vec4 gEmission;

void main()
{
	gWorldPosition = vec4(v_WorldPosition,1);
	gWorldNormal = vec4(v_WorldNormal,1);
}