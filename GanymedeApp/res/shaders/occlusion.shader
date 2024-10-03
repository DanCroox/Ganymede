#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform mat4 u_VP;

void main()
{
	//vec3 worldNormal = normalize(mat3(transpose(inverse(u_M))) * Normal);
	//vec3 worldPosition = (u_M * vec4(Position, 1)).xyz;
	//worldPosition += (worldNormal * .2);

	//gl_Position = u_VP * vec4(worldPosition, 1);
    //gl_Position = u_VP * vec4((u_M * vec4(Position, 1)).xyz, 1);
	
    vec3 worldNormal = Normal;
    vec3 worldPosition = Position;
    //worldPosition += (worldNormal * 0);
    gl_Position = u_VP * vec4(worldPosition, 1);
};

#shader fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

uniform float visibility;

void main()
{
	if (visibility > 0)
    {
        FragColor = vec4(1, 0, 0, .2f);
    }
}