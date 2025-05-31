#shader vertex
#version 460 core
#extension GL_ARB_shader_viewport_layer_array : require

#include "Compute/commoncompute.h"

layout(location = 0) in vec3 Position;
layout(location = 5) in ivec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

layout(std140, binding = 2) buffer BonesDataBlock
{
	mat4 bones[];
};

out vec3 v_FragPos;
out vec3 v_PointlightWorldPos;

void main()
{
	InstanceData ssboInstanceData = ssbo_InstanceData[gl_BaseInstance + gl_InstanceID];

	RenderView renderView = ssbo_RenderViews[ssboInstanceData.m_ViewID];

	mat4 modelTransform = ssboInstanceData.m_Transform;
	mat4 viewTransform = renderView.m_Transform;
	mat4 projectionTransform = renderView.m_Projection;

	mat4 ss_MVP = projectionTransform * viewTransform * modelTransform;
	mat4 ss_M = modelTransform;

	bool isAnimated = BoneWeights[0] + BoneWeights[1] + BoneWeights[2] + BoneWeights[3] > 0;
	vec4 vertexPosition = vec4(Position, 1);

	if (isAnimated)
	{
		uint boneDataOffset = ssbo_EntityData[ssboInstanceData.m_EntityDataIndex].m_AnimationDataOffset;

		mat4 BoneTransform = bones[BoneIndices[0] + boneDataOffset] * BoneWeights[0];
		BoneTransform += bones[BoneIndices[1] + boneDataOffset] * BoneWeights[1];
		BoneTransform += bones[BoneIndices[2] + boneDataOffset] * BoneWeights[2];
		BoneTransform += bones[BoneIndices[3] + boneDataOffset] * BoneWeights[3];

		vertexPosition = BoneTransform * vertexPosition;
	}

	gl_Position = ss_MVP * vertexPosition;
	v_FragPos = (ss_M * vertexPosition).xyz;

	// The ssbo_RenderViews share the exact same world location like the pointlight it is related to.
	// A pointlight has 6 views (one for each face of the cube shadowmap) - TODO: Replace by dual parabolic shadowmapping
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