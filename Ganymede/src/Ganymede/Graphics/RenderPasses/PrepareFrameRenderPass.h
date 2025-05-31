#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/RenderPass.h"
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
			float m_GameTime;
			float m_DeltaTime;
			glm::uint m_FrameNumber;
			glm::uint m_Pad1;
		};

		CommonRenderData m_CommonRenderData;
		SSBO* m_CommonRenderDataSSBO;
	};
}