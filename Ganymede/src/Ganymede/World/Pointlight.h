#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Pointlight
{
public:

	void Translate(float x, float y, float z);
	void Rotate(float angle, float x, float y, float z);
	void Scale(float x, float y, float z);
	const glm::mat4& GetTransform() const { return m_Transform; }

	void SetBrightness(float brightness) { m_Brightness = brightness; }
	float GetBrightness() const { return m_Brightness; }

private:
	glm::mat4 m_Transform;
	float m_Brightness = 10.f;
};