#shader vertex
// core = does not allow to use deprecated functions
#version 460 core
#extension GL_NV_viewport_array2 : require

layout(location = 0) in vec3 Position;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;
layout(location = 7) in uint InstanceDataIndex;

struct RenderView
{
	mat4 m_Transform;
	mat4 m_Perspective;
	vec4 m_WorldPosition;
	float m_NearClip;
	float m_FarClip;
	uint m_ViewID;
	uint m_FaceIndex;
	uint m_RenderViewGroup;
	uint m_Pad1;
	uint m_Pad2;
	uint m_Pad3;
};

layout(std430, binding = 22) buffer RenderViewBuffer { RenderView renderViews[]; };

struct NewInstanceData
{
	mat4 m_Transform;
	uint m_ViewID;
	uint m_MeshID;
	uint m_NumMeshIndices;
	uint m_FaceIndex;
	uint m_RenderViewGroup;
	uint m_Pad1;
	uint m_Pad2;
	uint m_Pad3;
};
layout(std140, binding = 23) buffer NewInstanceDataBlock
{
	NewInstanceData NewInstanceDatas[];
};

struct PointLightg
{
	vec3 lightPos;
	int u_LightIDs;
};

layout(std140, binding = 1) buffer PointLightDataBlockg
{
	PointLightg pointLightss[];
};

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

struct PointLight
{
    vec4 u_PointlightColor;
    vec3 u_PointlighWorldLocation;
    int u_LightID;
};

layout(std140, binding = 0) buffer PointLightDataBlock
{
    PointLight pointLights[];
};


uniform mat4 u_Projection;

out vec3 v_FragPos;
out vec3 v_PointlightWorldPos;

void main()
{
	NewInstanceData ssboInstanceData = NewInstanceDatas[gl_BaseInstance + gl_InstanceID];

	RenderView renderView = renderViews[ssboInstanceData.m_ViewID];

	mat4 modelTransform = ssboInstanceData.m_Transform;
	mat4 viewTransform = renderView.m_Transform;
	mat4 projectionTransform = renderView.m_Perspective;

	mat4 ss_MVP = projectionTransform * viewTransform * modelTransform;
	mat4 ss_M = modelTransform;

	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 ppos = vec4(Position, 1);

	isAnimated = false;
	if (isAnimated)
	{
		//uint boneDataOffset = ssboInstanceData.m_Attribs.z;
		uint boneDataOffset = 0u;
		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		ppos = BoneTransform * ppos;
	}

	gl_Position = ss_MVP * ppos;
	v_FragPos = (ss_M * ppos).xyz;

	v_PointlightWorldPos = renderView.m_WorldPosition.xyz;

	gl_Layer = int(ssboInstanceData.m_FaceIndex);
};


#shader fragment
#version 460 core

in vec3 v_FragPos;
in vec3 v_PointlightWorldPos;

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(v_FragPos.xyz - v_PointlightWorldPos);

	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / 1000.0;

	// write this as modified depth
	gl_FragDepth = lightDistance;
}