#include "ShadowMappingRenderPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/GPUCommands.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Renderer.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	bool ShadowMappingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_ShadowMapsArray = renderContext.CreateCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray", 6 * 100, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { m_ShadowMapSize, m_ShadowMapSize });
		
		m_Framebuffer = renderContext.CreateFrameBuffer("OmniDirectionalShadowMapping", { m_ShadowMapSize, m_ShadowMapSize }, false);
		m_Framebuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_ShadowMapsArray);

		m_ShadowMappingShader = renderContext.LoadShader("OmniDirectionalShadowMappingShader", {"res/shaders/OmnidirectionalShadowMapInstances.shader"});

		ssbo_IndirectDrawCmds = renderContext.GetSSBO("IndirectDrawCommands");

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

			GPUCommands::RenderTarget::ClearRenderTarget(
				*m_ShadowMapsArray,
				0,
				0,0,
				currentLightID * 6,
				m_ShadowMapSize, m_ShadowMapSize,
				6,
				&depthClear);
		}

		Renderer& renderer = renderContext.GetRenderer();

		std::vector<RenderMeshInstanceCommand>& renderInfos = renderContext.m_RenderInfo;
		for (int i = 1; i < renderContext.m_RenderInfoOffsets.size(); ++i)
		{
			const RenderMeshInstanceCommandOffsetsByView offset = renderContext.m_RenderInfoOffsets[i];
			
			for (unsigned int idx = offset.m_StartIndex; idx < offset.m_StartIndex + offset.m_LastIndex; ++idx)
			{
				RenderMeshInstanceCommand& renderInfo = renderInfos[idx];
				MeshWorldObject::Mesh& mesh = *renderContext.m_MeshIDMapping[renderInfo.m_MeshID];
				const VertexObject& voPtr = renderContext.GetVO(mesh);

				renderer.DrawIndirect(voPtr, *ssbo_IndirectDrawCmds, renderInfo.m_IndirectCommandIndex, *m_Framebuffer, *m_ShadowMappingShader, true);
			}
		}
	}
}