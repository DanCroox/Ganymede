#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class PointlightWorldObjectInstance;

	class GANYMEDE_API LightsManager
	{
	public:
		enum class LightingState
		{
			Initialize,
			DynamicShadow,
			StaticShadow,
			NoShadow
		};

		static unsigned int MAX_POINTLIGHTS_STATICS_SHADOWS;
		static unsigned int MAX_POINTLIGHTS_DYNAMIC_SHADOWS;

		LightsManager();
		~LightsManager() {};

		void Update(const std::vector<PointlightWorldObjectInstance*>& pointlightsSortedByDistanceToCamera);

	private:
		std::vector<int> m_LightIDStorage;
	};
}