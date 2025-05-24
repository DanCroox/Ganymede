#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/RenderPass.h"
#include <optional>

namespace Ganymede
{
	class SSBO;

	class GANYMEDE_API CollectGeometryPass : public RenderPass2
	{
	public:
		struct GANYMEDE_API InstanceDataGBuffer
		{
			glm::mat4 m_M;
			glm::mat4 m_MV;
			glm::uvec4 m_AnimationDataOffset;
		};

		struct GANYMEDE_API InstanceDataCubemapShadowMapping
		{
			glm::mat4 m_M;
			glm::mat4 m_MVP;
			glm::uvec4 m_Attribs;
		};

		bool Initialize(RenderContext& renderrContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		SSBO* m_CubemapShadowMappingInstanceDataSSBO;
		SSBO* m_AnimationDataSSBO;

		glm::mat4 m_PointLightProjectionMatrix;

		struct Visibility
		{
			std::int32_t m_InstanceID;
			glm::mat4 m_MVP;
		};

		std::vector<Visibility> cubemapInstanceDataIndices;
	};
}