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

struct PointLight
{
	mat4 u_ShadowMatrices[6];
	vec4 lightColor;
	vec3 lightPos;
	int u_LightIDs;
	vec4 updateShadowMap;
};

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;
out mat3 v_TBN;
out vec3 v_SSAOPos;
out vec3 v_SSAONormal;

out float testa;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	int bone = 5;
	testa = 0;

	for (int cb = 0; cb < 4; ++cb)
	{
		if (BoneIndices[cb] == bone)
			testa += BoneWeights[cb];
	}

	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 ppos = vec4(Position, 1);

	if (isAnimated)
	{
		int boneDataOffset = int(instanceData.z);

		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		ppos = BoneTransform * ppos;
	}

	v_SSAOPos = (u_MV * vec4(Position, 1)).xyz;
	v_SSAONormal = mat3(transpose(inverse(u_MV))) * Normal;

	gl_Position = (u_Projection * u_MV) * ppos;
	v_Normal = mat3(transpose(inverse(u_M))) * Normal;
	v_TexCoords = TexCoords;
	v_FragPos = (u_M * ppos).xyz;

	mat3 m = mat3(u_M);
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

in float testa;

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