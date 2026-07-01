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

#define DEFINE_HANDLE_FOR_TYPE(TYPE, MEMBER)			\
	template<>											\
	Handle<TYPE>::Handle(size_t index)					\
		: m_Index(index)								\
		  HANDLE_DATA_INIT(MEMBER) {}					\
														\
	template<>											\
	const TYPE& Handle<TYPE>::GetData() const {			\
		return StaticData::Instance->MEMBER[m_Index];	\
	}

#ifndef RETAIL
#define HANDLE_DATA_INIT(MEMBER) , m_Data(&StaticData::Instance->MEMBER[m_Index])
#else
#define HANDLE_DATA_INIT(MEMBER)
#endif //RETAIL

	DEFINE_HANDLE_FOR_TYPE(MeshWorldObject, m_MeshWorldObjects);
	DEFINE_HANDLE_FOR_TYPE(SkeletalMeshWorldObject, m_SkeletalMeshWorldObjects);
	DEFINE_HANDLE_FOR_TYPE(PointlightWorldObject, m_PointlightWorldObjects);
	DEFINE_HANDLE_FOR_TYPE(Texture, m_Textures);
	DEFINE_HANDLE_FOR_TYPE(Animation, m_SceneAnimations);
	DEFINE_HANDLE_FOR_TYPE(MeshWorldObject::Mesh, m_Meshes);
	DEFINE_HANDLE_FOR_TYPE(Material, m_Materials);
	DEFINE_HANDLE_FOR_TYPE(ShaderBinary, m_ShaderBinaries);
}