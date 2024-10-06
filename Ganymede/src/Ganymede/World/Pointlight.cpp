#include "Pointlight.h"

namespace Ganymede
{
	void Pointlight::Translate(float x, float y, float z)
	{
		m_Transform = glm::translate(m_Transform, glm::vec3(x, y, z));
	}

	void Pointlight::Rotate(float angle, float x, float y, float z)
	{
		m_Transform = glm::rotate(m_Transform, std::fmod(angle, 360.f), glm::vec3(x, y, z));
	}

	void Pointlight::Scale(float x, float y, float z)
	{
		m_Transform = glm::scale(m_Transform, glm::vec3(x, y, z));
	}
}