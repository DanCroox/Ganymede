#pragma once

#include "Ganymede/Core/Core.h"

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

		RenderTarget* m_PositionsRTMS;
		RenderTarget* m_NormalsRTMS;
		RenderTarget* m_AlbedoRTMS;
		RenderTarget* m_MetalRoughnessRTMS;
		RenderTarget* m_EmissionRTMS;
		RenderTarget* m_DepthRTMS;
		RenderTarget* m_ComplexFragmentMS;

		RenderTarget* m_PositionsRT;
		RenderTarget* m_NormalsRT;
		RenderTarget* m_AlbedoRT;
		RenderTarget* m_MetalRoughnessRT;
		RenderTarget* m_EmissionRT;
		RenderTarget* m_DepthRT;

		SSBO* ssbo_IndirectDrawCmds;
	};
}