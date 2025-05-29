#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderPass.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include "Ganymede/World/MeshWorldObject.h"


namespace Ganymede
{
	class RenderContext;
	class SinglesampleRenderTarget;
	class MultisampleRenderTarget;
	class VertexObject;
	class SSBO;

	class GANYMEDE_API GeometryRenderPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderrContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_FrameBuffer;
		FrameBuffer* m_FrameBufferMS;

		MultisampleRenderTarget* m_PositionsRTMS;
		MultisampleRenderTarget* m_NormalsRTMS;
		MultisampleRenderTarget* m_AlbedoRTMS;
		MultisampleRenderTarget* m_MetalRoughnessRTMS;
		MultisampleRenderTarget* m_EmissionRTMS;
		MultisampleRenderTarget* m_DepthRTMS;
		MultisampleRenderTarget* m_ComplexFragmentMS;

		SinglesampleRenderTarget* m_PositionsRT;
		SinglesampleRenderTarget* m_NormalsRT;
		SinglesampleRenderTarget* m_AlbedoRT;
		SinglesampleRenderTarget* m_MetalRoughnessRT;
		SinglesampleRenderTarget* m_EmissionRT;
		SinglesampleRenderTarget* m_DepthRT;

		FrameBuffer::BlitFrameBufferConfig m_MultiToSingleSampleBlitFBConfig;
	};
}