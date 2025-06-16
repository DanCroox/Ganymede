#include "CompositeRenderPass.h"

#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/VertexObject.h"

namespace Ganymede
{
	bool CompositeRenderPass::Initialize(RenderContext& renderContext)
	{
		m_FrameBuffer = renderContext.CreateFrameBuffer("Hardware", { 1920, 1080 }, true);
		m_ViewportShader = renderContext.LoadShader("ScreenShader", { "res/shaders/ScreenShader2.shader" });
		m_ScreenVO = renderContext.GetVertexObject("ScreenVertexObject");

		return true;
	}

	void CompositeRenderPass::Execute(RenderContext& renderContext)
	{
		m_ViewportShader->BindTexture(*renderContext.GetSingleSampleRenderTarget("Lighting"), "m_LightingPass");
		renderContext.GetRenderer().ClearFrameBuffer(*m_FrameBuffer, true, true);
		renderContext.GetRenderer().DrawVertexObject(*m_ScreenVO, 1, *m_FrameBuffer, *m_ViewportShader, false);
	}
}