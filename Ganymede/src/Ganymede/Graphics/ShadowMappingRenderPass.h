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
		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_Framebuffer;
		CubeMapArrayRenderTarget* m_ShadowMapsArray;
		SSBO* m_AnimationDataSSBO;
		SSBO* m_PointLightSortedToCamDistanceOcclusionCheckUBO;
		Shader* m_ShadowMappingShader;
		DataBuffer<MeshInstanceVertexData>* m_InstanceDataBuffer;
		glm::mat4 m_PointLightProjectionMatrix;
		float m_PointLightNearClip;
		float m_PointLightFarClip;
	};
}