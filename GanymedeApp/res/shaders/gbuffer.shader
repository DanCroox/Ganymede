#shader vertex
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;
layout(location = 7) in uint GBufferInstanceDataIndex;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;
out mat3 v_TBN;
out vec3 v_SSAOPos;
out vec3 v_SSAONormal;

uniform mat4 u_Projection;

struct CommonShaderData
{
	mat4 m_View;
	mat4 m_Projection;
	float m_NearClip;
	float m_FarClip;
	float m_GameTime;
	float m_DeltaTime;
};

layout(std140, binding = 4) buffer CommonShaderDataBlock
{
	CommonShaderData CommonData;
};

struct GBufferInstanceData
{
	mat4 m_M;
	uvec4 m_AnimationDataOffset;
};

layout(std140, binding = 3) buffer InstanceDataBlock
{
	GBufferInstanceData InstanceDatas[];
};

struct NewInstanceData
{
	mat4 m_Transform;
	uint m_ViewID;
	uint m_MeshID;
	uint m_Pad1;
	uint m_Pad2;
};

layout(std140, binding = 25) buffer NewInstanceDataBlock
{
	NewInstanceData NewInstanceDatas[];
};

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

void main()
{
	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 ppos = vec4(Position, 1);

	NewInstanceData ssboInstanceData = NewInstanceDatas[gl_BaseInstance + gl_InstanceID];

	mat4 ss_MV = CommonData.m_View * ssboInstanceData.m_Transform;
	mat4 ss_M = ssboInstanceData.m_Transform;

	isAnimated = false;
	if (isAnimated)
	{
		//int boneDataOffset = int(ssboInstanceData.m_AnimationDataOffset);
		int boneDataOffset = 0;

		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		ppos = BoneTransform * ppos;
	}

	v_SSAOPos = (ss_MV * vec4(Position, 1)).xyz;
	v_SSAONormal = mat3(transpose(inverse(ss_MV))) * Normal;

	gl_Position = (CommonData.m_Projection * ss_MV) * ppos;
	v_Normal = mat3(transpose(inverse(ss_M))) * Normal;
	v_TexCoords = TexCoords;
	v_FragPos = (ss_M * ppos).xyz;

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
//layout(location = 4) out vec4 ssaoPosition;
//layout(location = 5) out vec4 ssaoNormal;

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_Normal;
in mat3 v_TBN;
in vec3 v_SSAOPos;
in vec3 v_SSAONormal;

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
	//ssaoPosition = vec4(v_SSAOPos, 1);
	//ssaoNormal = vec4(v_SSAONormal, 1);

	// store the fragment position vector in the first gbuffer texture
	gPosition = vec4(v_FragPos, 1);

	// also store the per-fragment normals into the gbuffer
	vec3 normal = texture(u_Texture1, v_TexCoords).xyz;

	normal = normal * 2.0 - 1.0;
	gNormal = vec4(normalize(v_TBN * normal), 1);
	
	// and the diffuse per-fragment color
		gAlbedo = texture(u_Texture0, v_TexCoords) * vec4(u_BaseColor, 1);
	if (gAlbedo.a < 1.0)
		discard;

	int count = 0;
	for (int i = 0; i < 4; ++i)
	{
		if ((gl_SampleMaskIn[0] & (1 << i)) != 0)
		{
			++count;
		}
	}

	if (count != 4)
		gComplexFragment = vec4(1);
	else
		gComplexFragment = vec4(0, 0, 0, 1);


	//TODO: optimize and merge roughness and metalness during asset loading into one texture
	// Metal and roughness
	gMetalRough.r = texture(u_Texture3, v_TexCoords).r * u_Metalness;
	
	//Roughness
	gMetalRough.g = texture(u_Texture2, v_TexCoords).g * u_Roughness;
	gMetalRough.a = 1;


	//Emission
	gEmission = texture(u_Texture4, v_TexCoords) * vec4(u_EmissiveColor, 1);

}