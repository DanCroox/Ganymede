#pragma once
#include "WorldObjectInstance.h"
#include <algorithm>

#include "Ganymede/Graphics/Material.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/Graphics/LightsManager.h"


class MeshWorldObjectInstance : public WorldObjectInstance
{
public:
	struct OcclusionQueryInfo
	{
		unsigned int m_CollisionQueryID;
		bool m_CollsionQueryInProgress = false;
		std::vector<bool> m_IsOccluded;
	};

	MeshWorldObjectInstance(const MeshWorldObject* meshWorldObject) :
		WorldObjectInstance(meshWorldObject),
		m_MeshWorldObject(meshWorldObject),
		m_CastShadows(meshWorldObject->GetCastShadows())
	{
		for (const MeshWorldObject::Mesh* mesh : meshWorldObject->m_Meshes)
		{
			AddMaterial(mesh->m_Material);
		}
	};

	~MeshWorldObjectInstance() = default;

	const std::vector<Material>& GetMaterials() const { return m_Materials; }

	void AddMaterial(const Material& material)
	{
		m_Materials.push_back(material);
	}

	void SetMaterialToSlot(const Material& material, unsigned char slotIndex)
	{
		m_Materials[slotIndex] = material;
	}

	void SetCastShadows(bool enable) { m_CastShadows = enable; }
	bool GetCastShadows() const { return m_CastShadows; }

	const MeshWorldObject* GetMeshWorldObject() const { return m_MeshWorldObject; }

	void MakeRigidBody(float mass) override;

	bool rotate = false;

protected:
	std::vector<Material> m_Materials;
	const MeshWorldObject* m_MeshWorldObject;
	bool m_CastShadows;
};