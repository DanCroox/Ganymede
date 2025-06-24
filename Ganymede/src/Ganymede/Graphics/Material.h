#pragma once

#include "Ganymede/Data/Handle.h"
#include "Ganymede/Data/SerializerTraits.h"
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>
#include <variant>

namespace Ganymede
{
	class ShaderBinary;
	class Texture;

	class Material
	{
	public:
		using MaterialPropertyData = std::variant<float, glm::vec3, Handle<Texture>>;

		struct MaterialProperty
		{
			GM_SERIALIZABLE(MaterialProperty);

			MaterialProperty() = default;
			MaterialProperty(float value) : m_Data(value) {}
			MaterialProperty(glm::vec3 value) : m_Data(value) {}
			MaterialProperty(Handle<Texture> value) : m_Data(value) {}

			bool operator==(const MaterialProperty& other) const { return m_Data == other.m_Data; }
			bool operator!=(const MaterialProperty& other) const { return !(other == *this); }

			MaterialPropertyData m_Data;
		};

		Material(const Handle<ShaderBinary>& shaderHandle) : m_ShaderBinaryHandle(shaderHandle) {};

		void AddMaterialFloatProperty(const std::string& name, float value) { m_MaterialProperties.emplace(name, value); }
		void AddMaterialVector3fProperty(const std::string& name, glm::vec3 value) { m_MaterialProperties.emplace(name, value);	}
		void AddMaterialTextureSamplerProperty(const std::string& name, const Handle<Texture>& value) {	m_MaterialProperties.emplace(name, value); }

		bool operator==(const Material& other) const { return m_ShaderBinaryHandle == other.m_ShaderBinaryHandle && m_MaterialProperties == other.m_MaterialProperties;	}
		bool operator!=(const Material& other) const { return !(other == *this); }

		void SetShader(const Handle<ShaderBinary>& shaderHandle) { m_ShaderBinaryHandle = shaderHandle; }
		const Handle<ShaderBinary>& GetShaderBinary() const { return m_ShaderBinaryHandle; }

		const std::unordered_map<std::string, MaterialProperty>& GetMaterialProperties() const { return m_MaterialProperties; }

	private:
		GM_SERIALIZABLE(Material);
		Material() : m_ShaderBinaryHandle(Handle<ShaderBinary>(0)) {}

		mutable std::unordered_map<std::string, MaterialProperty> m_MaterialProperties;
		Handle<ShaderBinary> m_ShaderBinaryHandle;
	};
}