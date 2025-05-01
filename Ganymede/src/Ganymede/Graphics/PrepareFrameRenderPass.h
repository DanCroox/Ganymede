#pragma once
#include "Ganymede/Core/Core.h"

#include "RenderPass.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class SSBO;

	class GANYMEDE_API PrepareFrameRenderPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		struct CommonRenderData
		{
			glm::mat4 m_View;
			glm::mat4 m_Projection;
			float m_NearClip;
			float m_FarClip;
			float m_GameTime;
			float m_DeltaTime;
		};

		CommonRenderData m_CommonRenderData;
		SSBO* m_CommonRenderDataSSBO;
	};
}