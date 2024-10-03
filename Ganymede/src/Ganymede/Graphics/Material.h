#pragma once

#include <array>
#include <unordered_map>
#include "Shader.h"

class Texture;

class Material
{
public:
	Material();

	void SetShader(Shader* shader) { m_Shader = shader; }
	const Shader* GetShader() const { return m_Shader; }

	void Bind() const;
	void Unbind() const;

	void AddMaterialFloatProperty(const std::string& name, float value)
	{
		MaterialPropertyData propertyData;
		propertyData.m_Scalar = value;
		m_MaterialProperties[name] = { MaterialProperty::Type::Float, propertyData };
	}

	void AddMaterialVector3fProperty(const std::string& name, glm::vec3 value)
	{
		MaterialPropertyData propertyData;
		propertyData.m_Vector3f = value;
		m_MaterialProperties[name] = { MaterialProperty::Type::Vector3f, propertyData };
	}

	void AddMaterialTextureSamplerProperty(const std::string& name, const Texture* value)
	{
		MaterialPropertyData propertyData;
		propertyData.m_Texture = value;
		m_MaterialProperties[name] = { MaterialProperty::Type::TextureSampler, propertyData };
	}

	bool operator==(const Material& other) const
	{
		return m_Shader == other.m_Shader &&
			m_MaterialProperties == other.m_MaterialProperties;
	}

	bool operator!=(const Material& other) const
	{
		return !(other == *this);
	}

private:
	union MaterialPropertyData
	{
		float m_Scalar;
		glm::vec3 m_Vector3f;
		const Texture* m_Texture;
	};

	struct MaterialProperty
	{
		enum class Type
		{
			Float,
			Vector3f,
			TextureSampler
		};

		bool operator==(const MaterialProperty& other) const
		{
			bool dataEqual = false;

			switch (m_Type)
			{
			case Type::Float:
				return m_Data.m_Scalar == other.m_Data.m_Scalar;
			case Type::Vector3f:
				return m_Data.m_Vector3f == other.m_Data.m_Vector3f;
			case Type::TextureSampler:
				return m_Data.m_Texture == other.m_Data.m_Texture;
			default:
				// TODO Add assert to not fail compare if other type is not reflected here
				return false;
				break;
			}
		}

		bool operator!=(const MaterialProperty& other) const
		{
			return !(other == *this);
		}

		Type m_Type = Type::Float;
		MaterialPropertyData m_Data;
	};

	mutable std::unordered_map<std::string, MaterialProperty> m_MaterialProperties;

	// TODO: Implement proper shader binding!
public:
	Shader* m_Shader;
};