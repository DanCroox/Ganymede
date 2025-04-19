#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec3 Position;

layout(location = 9) in mat4 u_M;

void main()
{
	gl_Position = u_M * vec4(Position,1);
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
	if (gl_InvocationID >= u_PointlightCount || pointLights[gl_InvocationID].updateShadowMap.x != 1)
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

layout(location = 0) out vec3 FragColor;

uniform float far_plane;
uniform vec3 lightPoss[32];

//out vec3 FragColor;

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(FragPos.xyz - llpos);

	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / far_plane;

	// write this as modified depth
	gl_FragDepth = lightDistance;

	//gColor = vec4(1);

	//FragColor = vec3(0);
	//return;
	//gColor.xyz = vec3(.5);
	//gColor.w = lightDistance;
}
































