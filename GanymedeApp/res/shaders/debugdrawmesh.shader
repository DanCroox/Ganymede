#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 2) in vec3 Normal;
layout(location = 9) in mat4 u_M;

uniform mat4 u_VP;

void main()
{
	vec3 worldNormal = normalize(mat3(transpose(inverse(u_M))) * Normal);
	vec3 worldPosition = (u_M * vec4(Position, 1)).xyz;
	//worldPosition += (worldNormal * .2);

	gl_Position = u_VP * vec4(worldPosition + (0,0, 0), 1);
};

#shader fragment
#version 460 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(.25,0,0,.4);
}