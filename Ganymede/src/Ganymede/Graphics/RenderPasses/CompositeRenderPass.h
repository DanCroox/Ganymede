#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/RenderPass.h"
#include <memory>

namespace Ganymede
{
	class FrameBuffer;
	class GraphicsShader;
	class VertexObject;

	class GANYMEDE_API CompositeRenderPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_FrameBuffer;
		GraphicsShader* m_ViewportShader;
		VertexObject* m_ScreenVO;
	};
}