#pragma once

#include "Ganymede/Core/Core.h"

#include "RenderPass.h"
#include <memory>

namespace Ganymede
{
	class VertexObject;
	class Shader;
	class FrameBuffer;

	class GANYMEDE_API CompositeRenderPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_FrameBuffer;
		Shader* m_ViewportShader;
		VertexObject* m_ScreenVO;
	};
}