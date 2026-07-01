#shader vertex
#version 450
#extension GL_EXT_nonuniform_qualifier : require

struct MeshInstanceData
{
	mat4 m_M;
	mat4 m_V;
	mat4 m_P;
	
	uint m_MaterialBufferIndex;
	uint m_MaterialDataIndex;
	uint m_AnimationBufferIndex;
	uint m_AnimationDataIndex;
};

// MVP-Matrix (SSBO)
layout(set = 0, binding = 1) buffer AnimationDataSSBO {
	mat4 data[];
} animationDataSSBO[];

layout(set = 0, binding = 1) buffer MeshInstanceDataSSBO {
	MeshInstanceData data[];
} meshInstanceData[];

layout(push_constant) uniform PushBlock {
	uint m_BufferIndex;
	uint m_DataIndex;
	uint _PadA;
	uint _PadB;
} pc;

// Eingabe aus Vertex-Buffer
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

// Ausgabe an Fragment-Shader
layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out mat3 TBN;

void main()
{
	MeshInstanceData instanceData = meshInstanceData[nonuniformEXT(pc.m_BufferIndex)].data[pc.m_DataIndex];

	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 vertexPosition = vec4(inPosition, 1);
	
	if (isAnimated)
	{
		uint boneDataOffset = instanceData.m_AnimationDataIndex;
		
		mat4 BoneTransform = animationDataSSBO[nonuniformEXT(instanceData.m_AnimationBufferIndex)].data[BoneIndices[0] + instanceData.m_AnimationDataIndex] * BoneWeights[0];
		BoneTransform += animationDataSSBO[nonuniformEXT(instanceData.m_AnimationBufferIndex)].data[BoneIndices[1] + instanceData.m_AnimationDataIndex] * BoneWeights[1];
		BoneTransform += animationDataSSBO[nonuniformEXT(instanceData.m_AnimationBufferIndex)].data[BoneIndices[2] + instanceData.m_AnimationDataIndex] * BoneWeights[2];
		BoneTransform += animationDataSSBO[nonuniformEXT(instanceData.m_AnimationBufferIndex)].data[BoneIndices[3] + instanceData.m_AnimationDataIndex] * BoneWeights[3];

		vertexPosition = BoneTransform * vertexPosition;
	}

	mat4 mvp = instanceData.m_P * instanceData.m_V * instanceData.m_M;

	gl_Position = mvp * vertexPosition;
	fragUV = inUV;
	fragPos = (instanceData.m_M * vertexPosition).xyz;

	mat3 m = mat3(instanceData.m_M);
	vec3 T = normalize(m * inTangent);
	vec3 N = normalize(m * inNormal);
	vec3 B = normalize(m * inBitangent);

	TBN = mat3(T, B, N);
}

#shader fragment
#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in mat3 TBN;

struct MaterialData
{
	uint m_AlbedoIdx;        
	uint m_NormalIdx;        
	uint m_RoughnessIdx;     
	uint m_MetallicIdx;      

	uint m_EmissionIdx;      
	uint _PadA;              
	uint _PadB;              
	uint _PadC;              

	vec4 m_BaseColor;   

	vec3 m_EmissiveColor;
	float _PadD;             

	float m_Roughness;       
	float m_Metalness;       
	float _PadE;             
	float _PadF;             

	vec4 _PadG[11];
};

struct MeshInstanceData
{
	mat4 m_M;
	mat4 m_V;
	mat4 m_P;
	
	uint m_MaterialBufferIndex;
	uint m_MaterialDataIndex;
	uint m_AnimationBufferIndex;
	uint m_AnimationDataIndex;
};

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(set = 0, binding = 1) buffer MaterialDataBuffer {
	MaterialData materialData[];
} materialSSBO[];

layout(set = 0, binding = 1) buffer MeshInstanceDataSSBO {
	MeshInstanceData data[];
} meshInstanceDataSSBO[];


layout(push_constant) uniform PushBlock {
	uint m_BufferIndex;
	uint m_DataIndex;
	uint _PadA;
	uint _PadB;
} pc;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec2 gRoughnessMetal;
layout(location = 4) out vec3 gEmission;
layout(location = 5) out uint gComplexFrag;

void main()
{
	MeshInstanceData meshInstanceData = meshInstanceDataSSBO[nonuniformEXT(pc.m_BufferIndex)].data[pc.m_DataIndex];
	MaterialData matData = materialSSBO[nonuniformEXT(meshInstanceData.m_MaterialBufferIndex)].materialData[meshInstanceData.m_MaterialDataIndex];

	gPosition = fragPos;
	gAlbedo = texture(textures[nonuniformEXT(matData.m_AlbedoIdx)], fragUV) * matData.m_BaseColor;

	vec3 tangendNormal = texture(textures[nonuniformEXT(matData.m_NormalIdx)], fragUV).xyz;
	tangendNormal = tangendNormal * 2.0 - 1.0;
	gNormal = normalize(TBN * tangendNormal);

	vec4 roughness = texture(textures[nonuniformEXT(matData.m_RoughnessIdx)], fragUV);
	vec4 metalness = texture(textures[nonuniformEXT(matData.m_MetallicIdx)], fragUV);
	gRoughnessMetal.r = roughness.r;
	gRoughnessMetal.g = metalness.g;

	gEmission = texture(textures[nonuniformEXT(matData.m_EmissionIdx)], fragUV).rgb;

	if (bitCount(gl_SampleMaskIn[0]) != 4)
	{
		gComplexFrag = 1;
	}
	else
	{
		gComplexFrag = 0;
	}
}