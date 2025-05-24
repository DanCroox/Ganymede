#pragma once
#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"

namespace Ganymede
{
	struct GCSkeletal
	{
		// TODO: Dynamic sized components should be avoided. Find a cache-friendly approach later.
		std::vector<glm::mat4> m_AnimationBoneData;
	};
}