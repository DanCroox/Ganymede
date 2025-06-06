#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Data/Handle.h"
#include "glm/glm.hpp"
#include <unordered_map>
#include <variant>

namespace Ganymede
{
	class Shader;
	class Texture;

	class GANYMEDE_API Material
	{
	public:
		void SetShader(Shader* shader);
		const Shader* GetShader() const;

		void Bind() const;
		void Unbind() const;

		void AddMaterialFloatProperty(const std::string& name, float value)
		{
			MaterialPropertyData propertyData = value;
			m_MaterialProperties[name] = { propertyData };
		}

		void AddMaterialVector3fProperty(const std::string& name, glm::vec3 value)
		{
			MaterialPropertyData propertyData = value;
			m_MaterialProperties[name] = { propertyData };
		}

		void AddMaterialTextureSamplerProperty(const std::string& name, const Handle<Texture>& value)
		{
			MaterialPropertyData propertyData = value;
			m_MaterialProperties[name] = { propertyData };
		}

		bool operator==(const Material& other) const;

		bool operator!=(const Material& other) const
		{
			return !(other == *this);
		}

	private:
		using MaterialPropertyData = std::variant<float, glm::vec3,	Handle<Texture>>;

		struct MaterialProperty
		{
			bool operator==(const MaterialProperty& other) const
			{
				return m_Data == other.m_Data;
			}

			bool operator!=(const MaterialProperty& other) const
			{
				return !(other == *this);
			}

			MaterialPropertyData m_Data;
		};

		mutable std::unordered_map<std::string, MaterialProperty> m_MaterialProperties;

		// TODO: Implement proper shader binding!
	public:
		Shader* m_Shader = nullptr;
	};
}