#shader vertex
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    v_TexCoords = texCoords;
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 v_TexCoords;

uniform vec3 u_ViewPos;

uniform vec3 u_PointlighWorldLocations[10];
uniform vec3 u_PointlightColors[10];
uniform int u_PointlightCount;
uniform samplerCubeArray u_DepthCubemapTexture;

uniform sampler2D u_GPositions;
uniform sampler2D u_GNormals;
uniform sampler2D u_GDepths;
uniform sampler2D u_GAlbedo;
uniform sampler2D u_GMetalRough;

const float PI = 3.14159265359;

vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

float ShadowCalculation(int lightIndex, vec3 viewPos, vec3 fragPos, vec3 pointlightPos)
{
    float shadow = 0.0;
    float bias = .1;
    int samples = 1;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = .008;
    vec3 fragToLight = fragPos - pointlightPos;
    float currentDepth = length(fragToLight);
    for (int i = 0; i < samples; ++i)
    {
        //float closestDepth = texture(u_DepthCubemapTexture, vec4(fragToLight + (sampleOffsetDirections[i] * diskRadius), lightIndex)).r;
        float closestDepth = texture(u_DepthCubemapTexture, vec4(fragToLight, lightIndex)).r;
        closestDepth *= 1000;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow = shadow / float(samples);

    return shadow;
}

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

void main()
{
    vec3 albedo = texture(u_GAlbedo, v_TexCoords).xyz;
    vec3 position = texture(u_GPositions, v_TexCoords).xyz;
    vec3 normal = texture(u_GNormals, v_TexCoords).xyz;

    vec2 metalRough = texture(u_GMetalRough, v_TexCoords).xy;
    float metallic = metalRough.x;
    float roughness = metalRough.y;
    float spec = 0.5;
    float ao = 1.0;

    vec3 N = normal;
    vec3 V = normalize(u_ViewPos - position);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_PointlightCount; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(u_PointlighWorldLocations[i] - position);
        vec3 H = normalize(V + L);
        float distance = length(u_PointlighWorldLocations[i] - position);
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

        float ss = 1 - ShadowCalculation(i, u_ViewPos, position, u_PointlighWorldLocations[i]);

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (((kD * albedo / PI + specular) * radiance * NdotL) * ss);
    }

    vec3 ambient = vec3(0) * albedo * ao;
    vec3 finalColor = ambient + Lo;

    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    vec3 viewDir = normalize(u_ViewPos - position);
    float fresnel = dot(normal, viewDir);
    fresnel = clamp(1 - fresnel, 0.0, 1.0);
    fresnel = pow(fresnel, 1.5);

    FragColor = vec4(finalColor, 1);


    //float gamma = 1.;
    //color.rgb = pow(color.rgb, vec3(1.0 / gamma));
    //color = vec4(1, 0, 0, 1);
}



















