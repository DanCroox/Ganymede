#shader vertex
#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    TexCoords = texCoords;
}  

#shader fragment
#version 460 core

#include "common.h"

in vec2 TexCoords;

uniform sampler2D m_LightingPass;

out vec4 FragColor;

void main()
{
    FragColor = texture(m_LightingPass, TexCoords);
}