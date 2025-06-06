#include "RenderPipeline.h"

#include "Ganymede/Player/FPSCamera.h"
#include "GPUDebugHandler.h"
#include "GPUResourceSystem.h"
#include "RenderContext.h"
#include "Ganymede/Common/Helpers.h"

namespace Ganymede
{
	RenderPipeline::RenderPipeline(RenderContext& renderContext) :
		m_RenderContext(renderContext),
		m_IsInitialized(false)
	{}
	
	RenderPipeline::~RenderPipeline()
	{
#ifndef GM_RETAIL
		GPUDebugHandler::Disable();
#endif // GM_RETAIL
	}

	bool RenderPipeline::Initialize()
	{
		if (m_IsInitialized)
		{
			GM_CORE_ASSERT(false, "Pipeline already initialized. Not doing anything.");
			return true;
		}

		for (auto& renderPass : m_RenderPasses)
		{
			if (!renderPass->Initialize(m_RenderContext))
			{
				GM_CORE_ASSERT(false, "Failed to initialize renderpass.");
				return false;
			}
		}

#ifndef GM_RETAIL
		GPUDebugHandler::Enable();
#endif // GM_RETAIL

		m_IsInitialized = true;
		return true;
	}

	void RenderPipeline::Execute()
	{
		if (!m_IsInitialized)
		{
			GM_CORE_ASSERT(false, "Pipeline not initialized.");
			return;
		}

		for (auto& renderPass : m_RenderPasses)
		{
			if (renderPass->IsBypassed())
			{
				continue;
			}
			renderPass->Execute(m_RenderContext);
		}
	}
}