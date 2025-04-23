#pragma once

#include "Ganymede/Core/Core.h"
#include "DataBuffer.h"
#include "glm/glm.hpp"
#include "RenderPass.h"

namespace Ganymede
{
	struct MeshInstanceVertexData;
	class CubeMapArrayRenderTarget;
	class FrameBuffer;
	class Shader;
	class SSBO;

	class GANYMEDE_API ShadowMappingRenderPass : public RenderPass2
	{
	public:
		struct GANYMEDE_API OmniPointlightData
		{
			glm::vec3 m_WorldPosition;
			glm::uint m_ID;
		};

		struct GANYMEDE_API InstanceDataShadowMapping
		{
			glm::mat4 m_M;
			glm::mat4 m_MVP;
			glm::uvec4 m_Attribs;
		};

		VertexDataDefinition(ShadowMappingInstanceVertexData, InstanceDataShadowMapping,
			M(m_M, VertexDataPrimitiveType::Float, 16),
			M(m_MVP, VertexDataPrimitiveType::Float, 16),
			M(m_Attribs, VertexDataPrimitiveType::UInt, 4)
		);

		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_Framebuffer;
		CubeMapArrayRenderTarget* m_ShadowMapsArray;
		SSBO* m_AnimationDataSSBO;
		SSBO* m_PointlightDataSSBO;
		Shader* m_ShadowMappingShader;
		DataBuffer<ShadowMappingInstanceVertexData>* m_InstanceDataBuffer;
		DataBuffer<UInt32VertexData>* m_InstanceDataIndexBuffer;
		std::vector<glm::u32vec1> m_InterInstanceDataIndexBuffer;

		glm::mat4 m_PointLightProjectionMatrix;
		float m_PointLightNearClip;
		float m_PointLightFarClip;
		unsigned int m_ShadowMapSize = 1024;
	};
}