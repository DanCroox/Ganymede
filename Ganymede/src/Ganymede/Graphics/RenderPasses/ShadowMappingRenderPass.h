#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/RenderPass.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;
	class GraphicsShader;
	class RenderTarget;
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

		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		FrameBuffer* m_Framebuffer;
		RenderTarget* m_ShadowMapsCubeArray;
		SSBO* m_AnimationDataSSBO;
		SSBO* m_PointlightDataSSBO;
		GraphicsShader* m_ShadowMappingShader;
		DataBufferBase* m_InstanceDataBuffer;
		std::vector<glm::u32vec1> m_InterInstanceDataIndexBuffer;

		SSBO* ssbo_IndirectDrawCmds;

		glm::mat4 m_PointLightProjectionMatrix;
		float m_PointLightNearClip;
		float m_PointLightFarClip;
		unsigned int m_ShadowMapSize = 1024;
	};
}