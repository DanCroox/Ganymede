#pragma once

#include "WorldObject.h"

class PointlightWorldObject: public WorldObject
{
public:
	using WorldObject::WorldObject;

	Type GetType() const { return Type::PointLight; }

	float GetBrightness() const { return m_Brightness; }
	void SetBrightness(float brightness) { m_Brightness = brightness; }

	glm::vec3 GetColor() const { return m_Color; }
	void SetColor(float r, float g, float b) { m_Color.r = r, m_Color.g = g, m_Color.b = b; }

	void SetImportance(int importance) { m_Importance = importance; }
	int GetImportance() const { return m_Importance; }

private:
	glm::vec3 m_Color = glm::vec3(1.0f);
	float m_Brightness = 10.f;
	int m_Importance = 0;
};