#include "PrepareFrameRenderPass.h"

#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Runtime/GMTime.h"
#include "RenderContext.h"

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
		// TODO: Only update dirty data
		const FPSCamera& cam = renderContext.GetCamera();
		m_CommonRenderData.m_View = cam.GetTransform();
		m_CommonRenderData.m_Projection = cam.GetProjection();
		m_CommonRenderData.m_NearClip = cam.GetNearClip();
		m_CommonRenderData.m_FarClip = cam.GetFarClip();
		m_CommonRenderData.m_GameTime = GMTime::s_Time;
		m_CommonRenderData.m_DeltaTime = GMTime::s_DeltaTime;
		m_CommonRenderDataSSBO->Write(0, sizeof(CommonRenderData), &m_CommonRenderData);
	}
}