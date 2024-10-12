#pragma once

#include "Ganymede/Core/Core.h"

#include "WorldObjectInstance.h"
#include <algorithm>

#include "Ganymede/Graphics/Material.h"

namespace Ganymede
{
	class MeshWorldObject;

	class GANYMEDE_API MeshWorldObjectInstance : public WorldObjectInstance
	{
	public:
		GM_GENERATE_CLASSTYPEINFO(MeshWorldObjectInstance, WorldObjectInstance);

		struct OcclusionQueryInfo
		{
			unsigned int m_CollisionQueryID;
			bool m_CollsionQueryInProgress = false;
			std::vector<bool> m_IsOccluded;
		};

		MeshWorldObjectInstance(const MeshWorldObject* meshWorldObject);

		const MeshWorldObject* GetMeshWorldObject() const;

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


		void MakeRigidBody(float mass) override;

		bool rotate = false;

	protected:
		std::vector<Material> m_Materials;
		const MeshWorldObject* m_MeshWorldObject;
		bool m_CastShadows;
	};
}