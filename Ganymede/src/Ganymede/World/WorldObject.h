#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Data/SerializerTraits.h"
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

namespace Ganymede
{
	class GANYMEDE_API WorldObject
	{
	public:
		enum class Type
		{
			Invalid,

			Mesh,
			SkeletalMesh,
			PointLight,
		};

		WorldObject(const std::string& name) : m_Name(name) {};

		virtual Type GetType() const { return Type::Invalid; }

		const std::string& GetName() const { return m_Name; }

		const glm::mat4& GetTransform() const { return m_Transform; }
		void SetTransform(const glm::mat4& transform) { m_Transform = transform; }

	private:
		GM_SERIALIZABLE(WorldObject);

		std::string m_Name = "none";

		// Will hold the imported transformation.
		// e.g.: Object in blender is differently transformed compared to identiy
		// -> this will be stored here and reflect actual world pos as in blender
		glm::mat4 m_Transform = glm::mat4(1.0f);
	};
}