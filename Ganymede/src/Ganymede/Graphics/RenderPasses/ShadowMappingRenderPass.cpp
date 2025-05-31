#include "ShadowMappingRenderPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/World.h"
#include "gl/glew.h"

namespace Ganymede
{
	bool ShadowMappingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_ShadowMapsArray = renderContext.CreateCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray", 6 * 100, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { m_ShadowMapSize, m_ShadowMapSize });
		
		m_Framebuffer = renderContext.CreateFrameBuffer("OmniDirectionalShadowMapping", { m_ShadowMapSize, m_ShadowMapSize }, false);
		m_Framebuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_ShadowMapsArray);

		m_ShadowMappingShader = renderContext.LoadShader("OmniDirectionalShadowMappingShader", "res/shaders/OmnidirectionalShadowMapInstances.shader");
		
		//m_AnimationDataSSBO = renderContext.GetSSBO("AnimationData");

		return true;
	}

	void ShadowMappingRenderPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("ShadowMapping Pass");

		const auto pointlightsView = renderContext.GetWorld().GetEntities(Include<GCPointlight>{});

		unsigned int lightID = 0;
		for (auto [entity, pl] : pointlightsView.each())
		{
			const int currentLightID = lightID++;

			unsigned int m_DepthCubemapTexture = m_ShadowMapsArray->GetRenderID();
			float depthClear = 1.0f;
			glClearTexSubImage(
				m_DepthCubemapTexture,
				0,
				0, 0,
				currentLightID * 6,
				m_ShadowMapSize, m_ShadowMapSize,
				6,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				&depthClear
			);
		}

		OGLBindingHelper::BindFrameBuffer(*m_Framebuffer);
		OGLBindingHelper::BindShader(m_ShadowMappingShader->GetRendererID());

		glEnable(GL_DEPTH_TEST);

		std::vector<RenderMeshInstanceCommand>& renderInfos = renderContext.m_RenderInfo;
		for (int i = 1; i < renderContext.m_RenderInfoOffsets.size(); ++i)
		{
			const RenderMeshInstanceCommandOffsetsByView offset = renderContext.m_RenderInfoOffsets[i];
			
			for (unsigned int idx = offset.m_StartIndex; idx < offset.m_StartIndex + offset.m_LastIndex; ++idx)
			{
				RenderMeshInstanceCommand& renderInfo = renderInfos[idx];
				MeshWorldObject::Mesh& mesh = *renderContext.m_MeshIDMapping[renderInfo.m_MeshID];
		
				const VertexObject& voPtr = renderContext.GetVO(mesh);
				OGLBindingHelper::BindVertexArrayObject(voPtr.GetRenderID());
		
				glm::uint offset = renderInfo.m_IndirectCommandIndex * 20;
				glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)offset);
				NAMED_COUNTER("Num Drawcalls (Shadow Mapping)");
			}
		}
	}
}