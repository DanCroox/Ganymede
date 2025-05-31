#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/RenderPass.h"

namespace Ganymede
{
	class SSBO;

	class GANYMEDE_API UpdateDrawDataPass : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		SSBO* m_AnimationDataSSBO;
		SSBO* m_EntityDataSSBO;
	};
}