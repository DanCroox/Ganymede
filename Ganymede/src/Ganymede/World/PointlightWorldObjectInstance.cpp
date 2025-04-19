#include "PointlightWorldObjectInstance.h"
#include "glm/glm.hpp"
#include "World.h"
#include "Ganymede/Player/FPSCamera.h"

#include "Ganymede/Runtime/GMTime.h"

namespace Ganymede
{
	void PointlightWorldObjectInstance::SetBrightness(float brightness)
	{
		m_Brightness = glm::max(brightness, 0.f);
	}

	void PointlightWorldObjectInstance::SetLightingState(LightsManager::LightingState lightingState)
	{
		if (lightingState == m_LightingState)
			return;

		m_LightingStateUpdateFrame = GMTime::s_FrameNumber;
		m_LightingState = lightingState;
	}
}