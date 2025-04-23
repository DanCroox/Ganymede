#include "ShadowMappingRenderPass.h"

#include "FrameBuffer.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/PointlightWorldObjectInstance.h"
#include "Ganymede/World/World.h"
#include "gl/glew.h"
#include "RenderContext.h"
#include "Shader.h"
#include "SSBO.h"
#include "VertexDataTypes.h"
#include "Ganymede/Common/Helpers.h"


namespace Ganymede
{
	bool ShadowMappingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_Framebuffer = renderContext.CreateFrameBuffer("OmniDirectionalShadowMapping", { m_ShadowMapSize, m_ShadowMapSize }, false);
		m_ShadowMapsArray = renderContext.CreateCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray", 6 * 300, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B16, { m_ShadowMapSize, m_ShadowMapSize });

		m_Framebuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_ShadowMapsArray);
		
		m_PointlightDataSSBO = renderContext.CreateSSBO("PointlightUpdateBuffer", 1, 320 * (sizeof(OmniPointlightData)), true);
		
		m_PointLightNearClip = 0.01f;
		m_PointLightFarClip = 1000.0f;
		m_PointLightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.f, m_PointLightNearClip, m_PointLightFarClip);

		m_ShadowMappingShader = renderContext.LoadShader("OmniDirectionalShadowMappingShader", "res/shaders/OmnidirectionalShadowMapInstances.shader");
		m_ShadowMappingShader->SetUniform1f("far_plane", m_PointLightFarClip);

		m_AnimationDataSSBO = renderContext.GetSSBO("AnimationData");
		m_InstanceDataBuffer = renderContext.CreateDataBuffer<ShadowMappingInstanceVertexData>("ShadowMappingInstanceVertexData", nullptr, 100000, DataBufferType::Dynamic);

		m_InstanceDataIndexBuffer = renderContext.GetDataBuffer<UInt32VertexData>("InstanceDataIndexBuffer");

		return true;
	}

	void ShadowMappingRenderPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("ShadowMapping Pass");

		OGLBindingHelper::BindFrameBuffer(*m_Framebuffer);

		const World& world = renderContext.GetWorld();
		auto pointlights = world.GetWorldObjectInstances<PointlightWorldObjectInstance>();
		std::vector<OmniPointlightData> pointlightsTotal;
		pointlightsTotal.reserve(pointlights.size());
		int lightId = 0;
		for (PointlightWorldObjectInstance* pointlight : pointlights)
		{
			const int currentLightID = lightId++;

			pointlight->SetLightID(currentLightID);
			pointlight->SetLightingState(LightsManager::LightingState::DynamicShadow);

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

			OmniPointlightData& pl = pointlightsTotal.emplace_back();
			pl.m_WorldPosition = pointlight->GetPosition();
			pl.m_ID = currentLightID;
		}
		m_PointlightDataSSBO->Write(0, pointlightsTotal.size() * sizeof(OmniPointlightData), pointlightsTotal.data());

		Renderer2& renderer = renderContext.GetRenderer();
		RenderCommandQueue& commandQueue = renderContext.m_CubemapShadowMappingCommandQueue;
		unsigned int indexOffset = 0;
		
		for (int i = 0; i < commandQueue.size(); ++i)
		{
			// CommandQueue comes sorted by mesh and shader type.
			// This allows to queue-up multiple instances and draw them with one submission.
			RenderCommand& renderCommand = commandQueue[i];

			VertexObject& vo = *renderCommand.m_VO;

			glm::u32vec1 d(renderCommand.m_SSBOInstanceID);
			m_InterInstanceDataIndexBuffer.push_back(d);

			const unsigned int nextIndex = i + 1;
			const bool isLastInstance = (commandQueue.size() - 1) == i;
			const bool submitInstance = isLastInstance ||
				commandQueue[nextIndex].m_VO != &vo;

			if (submitInstance)
			{
				m_InstanceDataIndexBuffer->Write(&m_InterInstanceDataIndexBuffer[0], m_InterInstanceDataIndexBuffer.size(), 0);
				renderer.DrawVertexObject(vo, m_InterInstanceDataIndexBuffer.size(), *m_Framebuffer, *m_ShadowMappingShader, true);
				m_InterInstanceDataIndexBuffer.clear();
			}
		}
	}
}