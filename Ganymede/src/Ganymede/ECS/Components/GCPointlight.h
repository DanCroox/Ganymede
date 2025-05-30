#pragma once
#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"

namespace Ganymede
{
	struct RenderView;

	struct GCPointlight
	{
		glm::vec3 m_Color = glm::vec3(1.0f);
		float m_Brightness = 10.0f;
		std::array<RenderView*, 6> m_CubemapViews;
	};
}