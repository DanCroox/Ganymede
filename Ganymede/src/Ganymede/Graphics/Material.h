#pragma once

#include "Ganymede/Data/Handle.h"
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>
#include <variant>

namespace Ganymede
{
	class Shader;
	class Texture;

	class Material
	{
	public:
		using MaterialPropertyData = std::variant<float, glm::vec3, Handle<Texture>>;

		struct MaterialProperty
		{
			bool operator==(const MaterialProperty& other) const { return m_Data == other.m_Data; }
			bool operator!=(const MaterialProperty& other) const { return !(other == *this); }

			MaterialPropertyData m_Data;
		};

		void AddMaterialFloatProperty(const std::string& name, float value) { m_MaterialProperties.emplace(name, value); }
		void AddMaterialVector3fProperty(const std::string& name, glm::vec3 value) { m_MaterialProperties.emplace(name, value);	}
		void AddMaterialTextureSamplerProperty(const std::string& name, const Handle<Texture>& value) {	m_MaterialProperties.emplace(name, value); }

		bool operator==(const Material& other) const { return m_Shader == other.m_Shader && m_MaterialProperties == other.m_MaterialProperties;	}
		bool operator!=(const Material& other) const { return !(other == *this); }

		void SetShader(Shader* shader) { m_Shader = shader; }
		const Shader* GetShader() const { return m_Shader; }

		const std::unordered_map<std::string, MaterialProperty>& GetMaterialProperties() const { return m_MaterialProperties; }

	private:
		mutable std::unordered_map<std::string, MaterialProperty> m_MaterialProperties;
		// TODO: Implement proper shader binding!
		Shader* m_Shader = nullptr;
	};
}