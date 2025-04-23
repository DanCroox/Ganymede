#shader vertex

// core = does not allow to use deprecated functions

#version 460 core


layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 7) in uint GBufferInstanceDataIndex;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;
out mat3 v_TBN;
out vec3 v_SSAOPos;
out vec3 v_SSAONormal;

uniform mat4 u_Projection;

struct GBufferInstanceData
{
	mat4 m_M;
	mat4 m_MV;
	uvec4 m_AnimationDataOffset;
};

layout(std140, binding = 3) buffer InstanceDataBlock
{
	GBufferInstanceData InstanceDatas[];
};

void main()
{
	GBufferInstanceData instanceData = InstanceDatas[GBufferInstanceDataIndex];
	mat4 instance_MV = instanceData.m_MV;
	mat4 instance_M = instanceData.m_M;

	v_SSAOPos = (instance_MV * vec4(Position, 1)).xyz;
	v_SSAONormal = mat3(transpose(inverse(instance_MV))) * Normal;
	mat4 u_MVP = u_Projection * instance_MV;
	gl_Position = u_MVP * vec4(Position, 1);
	v_Normal = mat3(transpose(inverse(instance_M))) * Normal;
	v_TexCoords = TexCoords;
	v_FragPos = (instance_M * vec4(Position, 1)).xyz;
	mat3 m = mat3(instance_M);
	vec3 T = m * normalize(Tangent);
	vec3 N = m * normalize(Normal);
	vec3 B = m * normalize(Bitangent);
	v_TBN = mat3(T, B, N);
};

#shader fragment
#version 460 core
#include "common.h"

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
uniform float u_GameTime;

void main()
{
	gPosition = vec4(v_FragPos, 1);
	vec3 triblend = pow(abs(v_Normal), vec3(8));
	triblend /= dot(triblend, vec3(1, 1, 1));
	float scale = .436;

	vec2 uvX = v_FragPos.zy * scale;
	vec2 uvY = v_FragPos.xz * scale;
	vec2 uvZ = v_FragPos.xy * scale;
	float yOffset = -0.0025;
	uvX.y += yOffset;
	uvZ.y += yOffset;
	vec3 axisSign = sign(v_Normal);
	axisSign.x = axisSign.x < 0 ? -1 : 1;
	axisSign.y = axisSign.y < 0 ? -1 : 1;
	axisSign.z = axisSign.z < 0 ? -1 : 1;

	uvX.x *= axisSign.x;
	uvY.x *= axisSign.y;
	uvZ.x *= -axisSign.z;

	vec3 tnormalX = texture(u_Texture1, uvX).rgb * 2.0 - 1.0;
	vec3 tnormalY = texture(u_Texture1, uvY).rgb * 2.0 - 1.0;
	vec3 tnormalZ = texture(u_Texture1, uvZ).rgb * 2.0 - 1.0;

	tnormalX.x *= axisSign.x;
	tnormalY.x *= axisSign.y;
	tnormalZ.x *= -axisSign.z;

	tnormalX = vec3(tnormalX.xy + v_Normal.zy, v_Normal.x);
	tnormalY = vec3(tnormalY.xy + v_Normal.xz, v_Normal.y);
	tnormalZ = vec3(tnormalZ.xy + v_Normal.xy, v_Normal.z);

	vec3 worldNormal = normalize(
		tnormalX.zyx * triblend.x +
		tnormalY.xzy * triblend.y +
		tnormalZ.xyz * triblend.z
	);

	gNormal = vec4(worldNormal, 1);

	// and the diffuse per-fragment color
	gAlbedo = texture(u_Texture0, uvX) * triblend.x;
	gAlbedo += texture(u_Texture0, uvY) * triblend.y;
	gAlbedo += texture(u_Texture0, uvZ) * triblend.z;
	gAlbedo *= vec4(u_BaseColor, 1);

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
	gMetalRough.r = texture(u_Texture3, uvX).r * triblend.x;
	gMetalRough.r += texture(u_Texture3, uvY).r * triblend.y;
	gMetalRough.r += texture(u_Texture3, uvZ).r * triblend.z;
	gMetalRough.r *= u_Metalness;

	gMetalRough.r *= 0;
	//Roughness
	gMetalRough.g = texture(u_Texture2, uvX).g * triblend.x;
	gMetalRough.g += texture(u_Texture2, uvY).g * triblend.y;
	gMetalRough.g += texture(u_Texture2, uvZ).g * triblend.z;
	gMetalRough.g *= u_Roughness * 1;

	//gMetalRough.g = mod(u_GameTime*.2, 1);
	gMetalRough.g = 0.35;
	gMetalRough.a = 1;

	//Emission
	gEmission = texture(u_Texture4, uvX) * triblend.x;
	gEmission += texture(u_Texture4, uvY) * triblend.y;
	gEmission += texture(u_Texture4, uvZ) * triblend.z;
	gEmission.a = 1;
	gEmission *= vec4(u_EmissiveColor, 1);
}