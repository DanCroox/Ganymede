#shader vertex
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(position.x, position.y, 0.0, 1.0);
	TexCoords = texCoords;
}

#shader fragment
#version 460 core

in vec2 TexCoords;

uniform sampler2D u_GDepth;

float order(float a, float b)
{
	return max(a, b);
}


void main()
{
	vec4 gDepths = textureGather(u_GDepth, TexCoords);

	gl_FragDepth =
	order(gDepths.x,
	order(gDepths.y,
	order(gDepths.z,
		gDepths.w)));

	//gl_FragDepth = texture(u_GDepth, TexCoords).r;
}