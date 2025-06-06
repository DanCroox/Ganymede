#pragma once

#include "Ganymede/Core/Core.h"

#include "WorldObject.h"
#include <array>
#include "Ganymede/Graphics/Material.h"


namespace Ganymede
{
	class World;

	class GANYMEDE_API MeshWorldObject : public WorldObject
	{
	public:
		enum PreferredPhysicsState
		{
			None, Static, Dynamic
		};

		struct Mesh
		{
			struct Vertex
			{
				Vertex() :
					m_Color(glm::vec4(1))
				{
					for (int i = 0; i < 4; ++i)
					{
						m_BoneIndex[i] = 0;
						m_BoneWeight[i] = 0;
					}
				}

				glm::vec3 m_Position;
				glm::vec2 m_UV;
				glm::vec3 m_Normal;
				glm::vec3 m_Tangent;
				glm::vec3 m_Bitangent;
				glm::vec4 m_Color;
				int m_BoneIndex[4];
				float m_BoneWeight[4];
			};

			struct BoundingBoxVertex
			{
				glm::vec3 m_Position;
				glm::vec3 m_Normal;
			};

			Mesh(size_t meshID) :
				m_MeshID(meshID)
			{
				m_BoundingBoxIndices.resize(12 * 3);

				//Front face
				m_BoundingBoxIndices[0] = 2;
				m_BoundingBoxIndices[1] = 1;
				m_BoundingBoxIndices[2] = 0;

				m_BoundingBoxIndices[3] = 0;
				m_BoundingBoxIndices[4] = 3;
				m_BoundingBoxIndices[5] = 2;

				// Back face
				m_BoundingBoxIndices[6] = 4;
				m_BoundingBoxIndices[7] = 5;
				m_BoundingBoxIndices[8] = 6;

				m_BoundingBoxIndices[9] = 6;
				m_BoundingBoxIndices[10] = 7;
				m_BoundingBoxIndices[11] = 4;

				// Left face
				m_BoundingBoxIndices[12] = 0;
				m_BoundingBoxIndices[13] = 4;
				m_BoundingBoxIndices[14] = 7;

				m_BoundingBoxIndices[15] = 7;
				m_BoundingBoxIndices[16] = 3;
				m_BoundingBoxIndices[17] = 0;

				// Right face
				m_BoundingBoxIndices[18] = 6;
				m_BoundingBoxIndices[19] = 5;
				m_BoundingBoxIndices[20] = 1;

				m_BoundingBoxIndices[21] = 1;
				m_BoundingBoxIndices[22] = 2;
				m_BoundingBoxIndices[23] = 6;

				//Top face
				m_BoundingBoxIndices[24] = 5;
				m_BoundingBoxIndices[25] = 4;
				m_BoundingBoxIndices[26] = 0;

				m_BoundingBoxIndices[27] = 0;
				m_BoundingBoxIndices[28] = 1;
				m_BoundingBoxIndices[29] = 5;

				// Bottom face
				m_BoundingBoxIndices[30] = 3;
				m_BoundingBoxIndices[31] = 7;
				m_BoundingBoxIndices[32] = 6;

				m_BoundingBoxIndices[33] = 6;
				m_BoundingBoxIndices[34] = 2;
				m_BoundingBoxIndices[35] = 3;
			}

			bool IsPointInsideMeshBoundingBox(glm::vec3 point, const glm::mat4& instanceTransform, const glm::vec3& scale) const
			{
				glm::vec3 boundingBoxCenterWorld = instanceTransform * glm::vec4(m_BoundingBoxCenter, 1);
				const glm::vec3 worldCenterToCamera = point - boundingBoxCenterWorld;

				glm::vec3 vecX = m_BoundingBoxCenter + (glm::vec3(1, 0, 0) * m_BoundingBoxHalfSize.x);
				glm::vec3 vecY = m_BoundingBoxCenter + (glm::vec3(0, 1, 0) * m_BoundingBoxHalfSize.y);
				glm::vec3 vecZ = m_BoundingBoxCenter + (glm::vec3(0, 0, 1) * m_BoundingBoxHalfSize.z);

				vecX = instanceTransform * glm::vec4(vecX, 1);
				vecY = instanceTransform * glm::vec4(vecY, 1);
				vecZ = instanceTransform * glm::vec4(vecZ, 1);

				vecX = glm::normalize(vecX - boundingBoxCenterWorld);
				vecY = glm::normalize(vecY - boundingBoxCenterWorld);
				vecZ = glm::normalize(vecZ - boundingBoxCenterWorld);

				glm::vec3 halfSizeScaled = glm::abs(scale * m_BoundingBoxHalfSize);
				halfSizeScaled += 2.f;

				return glm::abs(glm::dot(worldCenterToCamera, vecX)) <= halfSizeScaled.x &&
					glm::abs(glm::dot(worldCenterToCamera, vecY)) <= halfSizeScaled.y &&
					glm::abs(glm::dot(worldCenterToCamera, vecZ)) <= halfSizeScaled.z;
			}

			using AABB = std::array<BoundingBoxVertex, 8>;

			Material m_Material;

			std::vector<unsigned int> m_VertexIndicies;
			std::vector<Vertex> m_Vertices;

			AABB m_BoundingBoxVertices;
			std::vector<unsigned int> m_BoundingBoxIndices;
			glm::vec3 m_BoundingBoxCenter;
			glm::vec3 m_BoundingBoxHalfSize;

			size_t m_MeshID;
		};

		MeshWorldObject(const std::string& name);

		virtual Type GetType() const { return Type::Mesh; }

		void SetPreferredPhysicsState(PreferredPhysicsState state) { m_PreferredPhysicsState = state; }
		PreferredPhysicsState GetPreferredPhysicsState() const { return m_PreferredPhysicsState; }

		void SetExcludeFromNavigationMesh(bool exclude) { m_ExcludeFromNavigationMesh = exclude; }
		bool GetExcludeFromNavigationMesh() const { return m_ExcludeFromNavigationMesh; }

		void SetCastShadows(bool cast) { m_CastShadows = cast; }
		bool GetCastShadows() const { return m_CastShadows; }

		mutable std::vector<Handle<MeshWorldObject::Mesh>> m_Meshes;

	private:
		PreferredPhysicsState m_PreferredPhysicsState = PreferredPhysicsState::Static;
		bool m_ExcludeFromNavigationMesh = false;
		bool m_CastShadows = true;

		mutable bool m_IsBound = false;
	};
}