#include "PointlightWorldObjectInstance.h"
#include "glm/glm.hpp"
#include "World.h"
#include "Ganymede/Player/FPSCamera.h"

#include "Ganymede/Runtime/GMTime.h"


namespace Ganymede
{
	/*
	void PointlightWorldObjectInstance::Tick(float deltaTime)
	{
		return;

		const float distance = 15.f;

		const float lightPlayerDistance = glm::distance(GetPosition(), Globals::fpsCamera->GetPosition());
		float lightPower = glm::clamp(distance - lightPlayerDistance, 0.f, distance);
		lightPower /= distance;

		//m_Brightness = m_BrightnessOriginal * lightPower;

		const std::vector<SkeletalMeshWorldObjectInstance*>* npcsPtr = Globals::world->GetWorldObjectInstancesByType<SkeletalMeshWorldObjectInstance>();
		if (npcsPtr == nullptr)
		{
			return;
		}

		const std::vector<SkeletalMeshWorldObjectInstance*>& npcs = *npcsPtr;


		// Find closest creature
		float lightCreatureDistance = Numbers::MAX_FLOAT;
		for (const SkeletalMeshWorldObjectInstance* npc : npcs)
		{
			if (const CreatureMeshWorldObjectInstance* creatur = dynamic_cast<const CreatureMeshWorldObjectInstance*>(npc))
			{
				lightCreatureDistance = glm::min(lightCreatureDistance, glm::distance(GetPosition(), creatur->GetPosition()));
			}
		}

		float lightPowerCreature = glm::clamp(distance - lightCreatureDistance, 0.f, distance);
		lightPowerCreature /= distance;
		m_Brightness = glm::mix(m_Brightness, m_BrightnessOriginal * 2.f, lightPowerCreature);
		m_Color = glm::mix(m_ColorOriginal, glm::vec3(1, .05, .03), glm::min(1.f, lightPowerCreature * 5.f));

	}

	*/

	void PointlightWorldObjectInstance::SetBrightness(float brightness)
	{
		m_Brightness = glm::max(brightness, 0.f);
		m_BrightnessOriginal = m_Brightness;
	}

	void PointlightWorldObjectInstance::SetLightingState(LightsManager::LightingState lightingState)
	{
		if (lightingState == m_LightingState)
			return;

		m_LightingStateUpdateFrame = GMTime::s_FrameNumber;
		m_LightingState = lightingState;
	}
}