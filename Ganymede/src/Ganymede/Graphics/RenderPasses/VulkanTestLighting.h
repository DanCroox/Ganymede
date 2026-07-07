#pragma once
#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/RenderPass.h"

namespace Ganymede
{
	class GANYMEDE_API VulkanTestLighting : public RenderPass2
	{
	public:
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;
	};
}