#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform mat4 u_M;
uniform mat4 u_MNoScale;

void main()
{
	vec3 worldNormal = normalize(mat3(transpose(inverse(u_M))) * Normal);
	vec3 worldPosition = (u_M * vec4(Position, 1)).xyz;
	worldPosition += (worldNormal * .2);

	gl_Position = vec4(worldPosition, 1);
};

#shader geometry
#version 460 core


struct PointLight
{
	mat4 u_ShadowMatrices[6];
	vec4 lightColor;
	vec3 lightPos;
	int u_LightIDs;
	vec4 updateShadowMap;
};

layout(std140, binding = 1) buffer PointLightDataBlock
{
	PointLight pointLights[];
};

uniform int u_PointlightCount;

out vec4 FragPos; // FragPos from GS (output per emitvertex)
out vec3 llpos;

layout(triangles, invocations = 10) in;
layout(triangle_strip, max_vertices = 18) out;

void main()
{
	if (gl_InvocationID >= u_PointlightCount)
	{
		return;
	}

	llpos = pointLights[gl_InvocationID].lightPos;
	for (int face = 0; face < 6; ++face)
	{
		for (int i = 0; i < 3; ++i)
		{
			FragPos = gl_in[i].gl_Position;

			gl_Position = pointLights[gl_InvocationID].u_ShadowMatrices[face] * FragPos;
			gl_Layer = (pointLights[gl_InvocationID].u_LightIDs * 6) + face;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#shader fragment
#version 460 core

in vec4 FragPos;
in vec3 llpos;

uniform float far_plane;
uniform vec3 lightPoss[32];
void main()
{
	// get distance between fragment and light source
	float lightDistance = length(FragPos.xyz - llpos);

	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / far_plane;

	// write this as modified depth
	gl_FragDepth = lightDistance;
}

