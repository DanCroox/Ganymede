#include "LightsManager.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/World.h"
#include "glm/glm.hpp"

#include "Ganymede/Common/Helpers.h"

namespace Ganymede
{
	unsigned int LightsManager::MAX_POINTLIGHTS_STATICS_SHADOWS = 300;
	unsigned int LightsManager::MAX_POINTLIGHTS_DYNAMIC_SHADOWS = 5;

	LightsManager::LightsManager()
	{
		for (unsigned int i = 0; i < MAX_POINTLIGHTS_STATICS_SHADOWS; ++i)
		{
			m_LightIDStorage.push_back(i);
		}
	}

	void LightsManager::Update(const std::vector<PointlightWorldObjectInstance*>& pointlightsSortedByDistanceToCamera)
	{
		SCOPED_TIMER("LightsManager update");
		const unsigned int maxLights = glm::min((unsigned int)pointlightsSortedByDistanceToCamera.size(), MAX_POINTLIGHTS_STATICS_SHADOWS);
		for (unsigned int i = 0; i < maxLights; ++i)
		{
			PointlightWorldObjectInstance* light = pointlightsSortedByDistanceToCamera[i];

			// Light has an id ... all good
			//if (light->m_LightID > -1)
			//	continue;

			// Light does not have an id... lets check if there is one left in id storage
			//if (m_LightIDStorage.size() > 0)
			//{
			//	light->m_LightID = m_LightIDStorage.back();
			//	m_LightIDStorage.pop_back();
			//	continue;
			//}

			// All ids given to other lights... lets get a light less close to cam and use its light id
			// Start reading from a light index which is outside of range to be updated
			for (unsigned int i = maxLights; i < pointlightsSortedByDistanceToCamera.size(); ++i)
			{
				//PointlightWorldObjectInstance* oldLight = pointlightsSortedByDistanceToCamera[i];
				//if (oldLight->m_LightID > -1)
				//{
				//	light->m_LightID = oldLight->m_LightID;
				//	oldLight->m_LightID = -1;
				//	break;
				//}
			}
		}
	}
}