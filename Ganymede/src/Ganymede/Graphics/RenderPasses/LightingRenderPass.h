#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/RenderPass.h"
#include "Ganymede/Graphics/VertexDataTypes.h"

namespace Ganymede
{
	class FrameBuffer;
	class SinglesampleRenderTarget;
	class SSBO;
	class Shader;
	class VertexObject;

	class GANYMEDE_API LightingRenderPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		struct PointLight
		{
			glm::vec4 m_LightColor;
			glm::vec3 lightPos;
			int u_LightID = -1;
		};

		FrameBuffer* m_FrameBuffer;
		SinglesampleRenderTarget* m_LightingRT;
		SSBO* m_PointLightSortedToCamDistanceSSBO;
		Shader* m_LightingShader;
		VertexObject* m_ScreenVO;
		DataBuffer<Vec3VertexData>* m_ScreenVBPos;
		DataBuffer<Vec2VertexData>* m_ScreenVBUV;
	};
}