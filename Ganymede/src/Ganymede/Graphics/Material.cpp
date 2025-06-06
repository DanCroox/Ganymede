#include "Material.h"
#include "Shader.h"
#include "Texture.h"

namespace Ganymede
{
	void Material::SetShader(Shader* shader) { m_Shader = shader; }
	const Shader* Material::GetShader() const { return m_Shader; }

	bool Material::operator==(const Material& other) const
	{
		return m_Shader == other.m_Shader &&
			m_MaterialProperties == other.m_MaterialProperties;
	}

	void Material::Bind() const
	{
		//TODO: Do proper material binding. Currently following this quickndirty approach
		//m_Shader->Bind();

		std::unordered_map<std::string, MaterialProperty>::iterator it = m_MaterialProperties.begin();

		int nextTextureSlot = 0;

		while (it != m_MaterialProperties.end())
		{
			const std::string& propertyName = it->first;
			const MaterialPropertyData& propertyValue = it->second.m_Data;

			if (std::holds_alternative<float>(propertyValue))
			{
				m_Shader->SetUniform1f(propertyName.c_str(), std::get<float>(propertyValue));
			}
			else if (std::holds_alternative<glm::vec3>(propertyValue))
			{
				m_Shader->SetUniform3f(propertyName.c_str(), std::get<glm::vec3>(propertyValue));
			}
			else if (std::holds_alternative<Handle<Texture>>(propertyValue))
			{
				std::get<Handle<Texture>>(propertyValue).GetData().Bind(nextTextureSlot);
				m_Shader->SetUniform1i(propertyName, nextTextureSlot);
				++nextTextureSlot;
			}
			it++;
		}
	}

	void Material::Unbind() const
	{
		m_Shader->Unbind();
	}
}