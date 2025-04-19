#pragma once

#include "Ganymede/Core/Core.h"
#include "RenderPass.h"
#include "DataBuffer.h"
#include "VertexDataTypes.h"

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
		FrameBuffer* m_FrameBuffer;
		SinglesampleRenderTarget* m_LightingRT;
		SSBO* m_PointLightSortedToCamDistanceSSBO;
		Shader* m_LightingShader;
		VertexObject* m_ScreenVO;
		DataBuffer<Vec3VertexData>* m_ScreenVBPos;
		DataBuffer<Vec2VertexData>* m_ScreenVBUV;
	};
}