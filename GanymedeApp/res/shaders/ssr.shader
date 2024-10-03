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

uniform sampler2D colorBuffer;
uniform sampler2DMS depthMapMS;
uniform sampler2D u_MetalRough;
uniform sampler2DMS gNormalMS;

uniform mat4 invProjection;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 invView;
 
uniform ivec2 seed;
uniform float u_GameTime;

layout(location = 0) out vec4 ColorOut;
layout(location = 1) out vec4 NormalOut;

uniform ivec2 u_RenderResolution;

in vec2 TexCoords;

bool rayIsOutofScreen(vec2 ray) {
	return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0);
}


float GetBayerFromCoordLevel2(vec2 pixelpos)
{
	float finalBayer = 0.0;
	float finalDivisor = 0.0;
	float layerMult = 1.0;

	for (float bayerLevel = float(4); bayerLevel >= 1.0; bayerLevel--)
	{
		float bayerSize = exp2(bayerLevel) * 0.5;
		vec2 bayercoord = mod(floor(pixelpos.xy / bayerSize), 2.0);
		layerMult *= 4.0;

		float line0202 = bayercoord.x * 2.0;

		finalBayer += mix(line0202, 3.0 - line0202, bayercoord.y) / 3.0 * layerMult;
		finalDivisor += layerMult;
	}

	return finalBayer / finalDivisor;
}

void main()
{
	float maxRayDistance = 5;

	//View Space ray calculation
	int its = 0;

	vec3 finalColor = vec3(0);
	int sampleCount = 2;
    int cc = 0;
    
    for (int i = 0; i < sampleCount; ++i)
    {
        for (int j = 0; j < sampleCount; ++j)
        {
            ++cc;
            vec2 coords = TexCoords + vec2(float(i) / 1920, float(j) / 1080);
            ivec2 texels = ivec2(coords * vec2(1920, 1080));
            vec3 pixelPositionTexture = vec3(coords, 0);
            vec3 normal = texelFetch(gNormalMS, texels, 0).xyz; // TODO: Needs proper multisample for complex pixels in entire shader code!
		
            vec3 normalView = normalize(normal);

            normalView = (vec4(normalView, 1) * invView).xyz;

            float pixelDepth = texelFetch(depthMapMS, ivec2(pixelPositionTexture.xy * vec2(1920, 1080)), 0).r; // 0< <1

            pixelPositionTexture.z = pixelDepth;
            vec4 positionView = invProjection * vec4(pixelPositionTexture * 2 - 1, 1);
            positionView /= positionView.w;
            vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));

            reflectionView = normalize(reflectionView);
            if (reflectionView.z > 0)
            {
                continue;
            }
            vec3 rayEndPositionView = positionView.xyz + reflectionView * maxRayDistance;

		//Texture Space ray calculation
            vec4 rayEndPositionTexture = projection * vec4(rayEndPositionView, 1);
            rayEndPositionTexture /= rayEndPositionTexture.w;
            rayEndPositionTexture.xyz = (rayEndPositionTexture.xyz + vec3(1)) / 2.0f;

            float sampleDepth;
            float depthDif;

            vec3 startViewSpace = pixelPositionTexture;
            vec3 endViewSpace = rayEndPositionTexture.xyz;

            float rnd = GetBayerFromCoordLevel2(mod(gl_FragCoord.xy + ivec2(i * i + 5, i * i + 10), u_RenderResolution));
            //float rnd = randomFloat(coords);
            float step = 1.0 / 5;
            
            for (float interp = step; interp <= 1.0; interp += step)
            {
                vec3 position = mix(startViewSpace, endViewSpace, interp);
                vec3 positionNext = mix(startViewSpace, endViewSpace, interp + step);
                position = mix(position, positionNext, rnd);

                if (rayIsOutofScreen(position.xy))
                {
                    break;
                }

                sampleDepth = texelFetch(depthMapMS, ivec2(position.xy * vec2(1920, 1080)), 0).r;
                depthDif = position.z - sampleDepth;
                if (position.z > sampleDepth && startViewSpace.z < sampleDepth)
                {
				// hit
                    vec3 hdrColor = texture(colorBuffer, position.xy).rgb;
                    finalColor += min(hdrColor, vec3(1));
                    break;
                }
            }
        }
    }

    ColorOut = vec4(finalColor / cc, 1);
}