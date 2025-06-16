#include "Handle.h"

#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/Texture.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObject.h"
#include "StaticData.h"
#include <vector>

namespace Ganymede
{
	template <>
	const MeshWorldObject& Handle<MeshWorldObject>::GetData() const { return StaticData::Instance->m_MeshWorldObjects[m_Index]; }
	template <>
	const SkeletalMeshWorldObject& Handle<SkeletalMeshWorldObject>::GetData() const { return StaticData::Instance->m_SkeletalMeshWorldObjects[m_Index]; }
	template <>
	const PointlightWorldObject& Handle<PointlightWorldObject>::GetData() const { return StaticData::Instance->m_PointlightWorldObjects[m_Index]; }
	template <>
	const Texture& Handle<Texture>::GetData() const { return StaticData::Instance->m_Textures[m_Index]; }
	template <>
	const Animation& Handle<Animation>::GetData() const { return StaticData::Instance->m_SceneAnimations[m_Index]; }
	template <>
	const MeshWorldObject::Mesh& Handle<MeshWorldObject::Mesh>::GetData() const { return StaticData::Instance->m_Meshes[m_Index]; }
	template <>
	const Material& Handle<Material>::GetData() const { return StaticData::Instance->m_Materials[m_Index]; }
	template <>
	const ShaderBinary& Handle<ShaderBinary>::GetData() const { return StaticData::Instance->m_ShaderBinaries[m_Index]; }
}