#include "Material.h"
#include "Renderer.h"

// REWORK: set default mesh shader!
Material::Material()
{
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
		const MaterialProperty& propertyValue = it->second;
		const MaterialProperty::Type propertyType = propertyValue.m_Type;

		switch (propertyType)
		{
		case MaterialProperty::Type::Float:
			m_Shader->SetUniform1f(propertyName.c_str(), propertyValue.m_Data.m_Scalar);
			break;
		case MaterialProperty::Type::Vector3f:
			m_Shader->SetUniform3f(propertyName.c_str(), propertyValue.m_Data.m_Vector3f.x, propertyValue.m_Data.m_Vector3f.y, propertyValue.m_Data.m_Vector3f.z);
			break;
		case MaterialProperty::Type::TextureSampler:
			propertyValue.m_Data.m_Texture->Bind(nextTextureSlot);
			m_Shader->SetUniform1i(propertyName, nextTextureSlot);
			++nextTextureSlot;
			// TODO limit number of texture slots
			break;
		default:
			break;
		}

		it++;
	}
}

void Material::Unbind() const
{
	m_Shader->Unbind();
}