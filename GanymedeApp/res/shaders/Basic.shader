#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec2 v_TexCoords;
out vec3 v_Normals;
out vec3 v_FragPos;
out mat3 v_TBN;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

uniform mat4 u_MVP;
uniform mat4 u_V;
uniform mat4 u_M;

void main()
{
	gl_Position = u_MVP * position;
	v_Normals = mat3(transpose(inverse(u_M))) * vertexNormal;
	v_TexCoords = texCoords;
	v_FragPos = (u_M * position).xyz;

	vec3 T = normalize(vec3(u_M * vec4(tangent, 0.0)));
	vec3 B = normalize(vec3(u_M * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(u_M * vec4(vertexNormal, 0.0)));
	v_TBN = mat3(T, B, N);
};

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoords;
in vec3 v_Normals;
in vec3 v_FragPos;
in mat3 v_TBN;

uniform vec3 u_ViewPos;

uniform vec3 u_PointLighWorldLocation[10];
uniform float u_PointlightActive[10];

uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;

void main()
{
	vec3 normal = texture(u_Texture1, v_TexCoords).rgb;
	normal = normal * 2.0 - 1.0;
	normal = normalize(v_TBN * normal);

	vec4 objectColor = texture(u_Texture0, v_TexCoords);

	vec3 lightColor = vec3(1);
	


	float ambientStrength = .25;
	vec3 ambient = vec3(ambientStrength);

	vec3 norm = normal;
	vec3 lightDir = normalize(u_PointLighWorldLocation[0] - v_FragPos);
	float diff = max(dot(norm, lightDir), 0.0);

	float lightDistance = distance(u_PointLighWorldLocation[0], v_FragPos);
	float energy = 50 * (1 / (lightDistance));
	diff *= energy;

	// Specular
	float specularStrength = 2;

	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 200);
	vec3 specular = specularStrength * spec * lightColor;

	float fresnel = dot(norm, viewDir);
	fresnel = clamp(1 - fresnel, 0.0, 1.0);
	fresnel = pow(fresnel, 2);

	//ambient *= (fresnel*.5);
	
	vec3 result = (ambient + diff + specular) * objectColor.rgb;

	color = vec4(result, objectColor.a);
};




































