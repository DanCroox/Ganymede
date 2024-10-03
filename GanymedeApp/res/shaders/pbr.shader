#shader vertex
// core = does not allow to use deprecated functions
#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 tangent;

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
	gl_Position = (u_MVP * position);
	v_Normals = mat3(transpose(inverse(u_M))) * vertexNormal;
	v_TexCoords = texCoords;
    v_FragPos = (u_M * position).rgb;

	vec3 T = normalize(vec3(u_M * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(u_M * vec4(vertexNormal, 0.0)));
    vec3 B = normalize(vec3(u_M * vec4(cross(N, T), 0.0)));
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
uniform vec3 bla;

uniform vec3 u_PointLighWorldLocation[10];
uniform float u_PointlightActive[10];
uniform vec3 u_PointlightColors[10];

uniform vec3 u_BaseColor;
uniform float u_Roughness;
uniform float u_Metalness;

uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform samplerCubeArray u_DepthCubemapTexture;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

float ShadowCalculation(int lightIndex)
{
    float shadow = 0.0;
    float bias = .1;
    int samples = 20;
    float viewDistance = length(u_ViewPos - v_FragPos);
    float diskRadius = .008;
    vec3 fragToLight = v_FragPos - u_PointLighWorldLocation[lightIndex];
    float currentDepth = length(fragToLight);
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(u_DepthCubemapTexture, vec4(fragToLight + (sampleOffsetDirections[i] * diskRadius), lightIndex)).r;
        //float closestDepth = texture(u_DepthCubemapTexture, fragToLight ).r;
        closestDepth *= 1000;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow = shadow / float(samples);

    return shadow;
}

void main()
{
    vec3 normal = texture(u_Texture1, v_TexCoords).xyz;
    normal = normal * 2.0 - 1.0;
    normal = normalize(v_TBN * normal);

    vec4  albedoalpha = pow(texture(u_Texture0, v_TexCoords), vec4(2.2));
    vec3  albedo = albedoalpha.rgb * u_BaseColor; 
    float  alpha = albedoalpha.a;
    float metallic = texture(u_Texture3, v_TexCoords).x * u_Metalness;
    float roughness = texture(u_Texture2, v_TexCoords).g * u_Roughness;
    float spec = 0.5;
    float ao = 1.0;

    vec3 N = normal;
    vec3 V = normalize(u_ViewPos - v_FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 10; ++i)
    {
        if (u_PointlightActive[i] == 0)
        {
            continue;
        }
        // calculate per-light radiance
        vec3 L = normalize(u_PointLighWorldLocation[i] - v_FragPos);
        vec3 H = normalize(V + L);
        float distance = length(u_PointLighWorldLocation[i] - v_FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = (u_PointlightColors[i]) * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float ss = 1 - ShadowCalculation(i);
        ss += .01;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (((kD * albedo / PI + specular) * radiance * NdotL) * ss);
    }

    vec3 ambient = vec3(0.0) * albedo * ao;
    vec3 finalColor = ambient + Lo;

    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    float fresnel = dot(normal, viewDir);
    fresnel = clamp(1-fresnel, 0.0, 1.0);
    fresnel = pow(fresnel, 1.5);
    //finalColor = finalColor + (albedo * fresnel);

    //color = vec4(finalColor.r * ss, ss, ss, 1);
    color = vec4(finalColor, alpha);


    float gamma = 1.;
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));
    //color = vec4(1, 0, 0, 1);
}



































