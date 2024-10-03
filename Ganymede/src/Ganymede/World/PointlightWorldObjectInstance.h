#pragma once
#include "WorldObjectInstance.h"
#include "PointlightWorldObject.h"
#include "glm/glm.hpp"
#include "Ganymede/Runtime/GMTime.h"

#include "Ganymede/Graphics/LightsManager.h"

class PointlightWorldObjectInstance : public WorldObjectInstance
{
public:
	PointlightWorldObjectInstance(const PointlightWorldObject* pointLightWorldObject) :
		WorldObjectInstance(pointLightWorldObject)
	{
		if (pointLightWorldObject == nullptr)
		{
			SetBrightness(100.f);
			SetColor(1,1,1);
			SetImportance(0);

			return;
		}

		SetBrightness(pointLightWorldObject->GetBrightness());
		SetColor(pointLightWorldObject->GetColor().x, pointLightWorldObject->GetColor().y, pointLightWorldObject->GetColor().z);
		SetImportance(pointLightWorldObject->GetImportance());
	};

	glm::vec3 GetColor() const { return m_Color; }
	void SetColor(float r, float g, float b)
	{
		m_Color.r = r; m_Color.g = g; m_Color.b = b;
		m_ColorOriginal = m_Color;
	}

	float GetBrightness() const { return m_Brightness; }
	void SetBrightness(float brightness);

	void SetImportance(int importance) { m_Importance = importance; }
	int GetImportance() const { return m_Importance; }

	int GetLightID() const { return m_LightID; }

	LightsManager::LightingState GetLightingState() const { return m_LightingState; }

	void SetLightingState(LightsManager::LightingState lightingState);

	bool DoUpdateShadowMap() const
	{
		return (m_LightingState == LightsManager::LightingState::DynamicShadow) &&
			m_LightID > -1 &&
			GMTime::s_FrameNumber - m_LightingStateUpdateFrame >=0;
	}

private:
	friend class LightsManager;

	LightsManager::LightingState m_LightingState = LightsManager::LightingState::Initialize;
	unsigned int m_LightingStateUpdateFrame = 0;
	int m_LightID = -1;
	glm::vec3 m_Color = glm::vec3(1.0f);
	glm::vec3 m_ColorOriginal = glm::vec3(1.0f);
	float m_Brightness = 10.f;
	float m_BrightnessOriginal = 1;

	// Lights get culled automatically by distance. Lights with higher importance will not be culled although the distance to camera might be bigger.
	// The other light will be culled with lower importance.
	int m_Importance = 0;
};