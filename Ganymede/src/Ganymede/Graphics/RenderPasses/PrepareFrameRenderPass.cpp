#include "PrepareFrameRenderPass.h"

#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Runtime/GMTime.h"

namespace Ganymede
{
	bool PrepareFrameRenderPass::Initialize(RenderContext& renderContext)
	{
		// TOOD: Use UBO for small data
		m_CommonRenderDataSSBO = renderContext.CreateSSBO("CommonShaderData", 4, 1 * sizeof(CommonRenderData), false);
		return true;
	}

	void PrepareFrameRenderPass::Execute(RenderContext& renderContext)
	{
		m_CommonRenderData.m_GameTime = GMTime::s_Time;
		m_CommonRenderData.m_DeltaTime = GMTime::s_DeltaTime;
		m_CommonRenderData.m_FrameNumber = GMTime::s_FrameNumber;
		m_CommonRenderDataSSBO->Write(0, sizeof(CommonRenderData), &m_CommonRenderData);
	}
}