#include "GeometryRenderPass.h"

#include "SSBO.h"
#include "FrameBuffer.h"
#include "RenderTarget.h"
#include "VertexDataTypes.h"
#include "RenderContext.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/World/World.h"
#include "VertexObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Runtime/GMTime.h"
#include "gl/glew.h"
#include "CollectGeometryPass.h"
#include "Ganymede/Common/Helpers.h"


namespace Ganymede
{
	bool GeometryRenderPass::Initialize(RenderContext& renderContext)
	{
		m_FrameBufferMS = renderContext.CreateFrameBuffer("GeometryMS", { 1920, 1080 }, false);
		m_PositionsRTMS = renderContext.CreateMultiSampleRenderTarget("PositionsMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_NormalsRTMS = renderContext.CreateMultiSampleRenderTarget("NormalsMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_AlbedoRTMS = renderContext.CreateMultiSampleRenderTarget("AlbedoMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_MetalRoughnessRTMS = renderContext.CreateMultiSampleRenderTarget("MetalRoughnessMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_EmissionRTMS = renderContext.CreateMultiSampleRenderTarget("EmissionMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_DepthRTMS = renderContext.CreateMultiSampleRenderTarget("DepthMS", 4, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_ComplexFragmentMS = renderContext.CreateMultiSampleRenderTarget("ComplexFragmentMS", 4, RenderTargetTypes::ComponentType::R, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });

		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color0, *m_PositionsRTMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color1, *m_NormalsRTMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color2, *m_AlbedoRTMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color3, *m_MetalRoughnessRTMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color4, *m_EmissionRTMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color5, *m_ComplexFragmentMS);
		m_FrameBufferMS->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_DepthRTMS);

		m_FrameBuffer = renderContext.CreateFrameBuffer("Geometry", { 1920, 1080 }, false);
		m_PositionsRT = renderContext.CreateSingleSampleRenderTarget("Positions", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_NormalsRT = renderContext.CreateSingleSampleRenderTarget("Normals", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_AlbedoRT = renderContext.CreateSingleSampleRenderTarget("Albedo", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_MetalRoughnessRT = renderContext.CreateSingleSampleRenderTarget("MetalRoughness", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_EmissionRT = renderContext.CreateSingleSampleRenderTarget("Emission", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_DepthRT = renderContext.CreateSingleSampleRenderTarget("Depth", RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color0, *m_PositionsRT);
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color1, *m_NormalsRT);
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color2, *m_AlbedoRT);
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color3, *m_MetalRoughnessRT);
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color4, *m_EmissionRT);
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_DepthRT);

		m_InstanceDataSSBO = renderContext.GetSSBO("GBufferInstanceData");
		m_InstanceDataIndexBuffer = renderContext.GetDataBuffer<UInt32VertexData>("InstanceDataIndexBuffer");

		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Color0, FrameBuffer::AttachmentType::Color0, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });
		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Color1, FrameBuffer::AttachmentType::Color1, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });
		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Color2, FrameBuffer::AttachmentType::Color2, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });
		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Color3, FrameBuffer::AttachmentType::Color3, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });
		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Color4, FrameBuffer::AttachmentType::Color4, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });
		m_MultiToSingleSampleBlitFBConfig.m_AttachementsToBlit.push_back({ *m_FrameBufferMS, *m_FrameBuffer, FrameBuffer::AttachmentType::Depth, FrameBuffer::AttachmentType::Depth, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBuffer::BlitFilterType::Nearest });

		return true;
	}

	void GeometryRenderPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("Geometry Pass");
		Renderer2& renderer = renderContext.GetRenderer();

		renderer.ClearFrameBuffer(*m_FrameBufferMS, true, true);
		renderer.ClearFrameBuffer(*m_FrameBuffer, true, true);

		RenderCommandQueue& commandQueue = renderContext.m_GBufferCommandQueue;
		unsigned int indexOffset = 0;

		for(int i = 0; i < commandQueue.size(); ++i)
		{
			// CommandQueue comes sorted by mesh and shader type.
			// This allows to queue-up multiple instances and draw them with one submission.
			RenderCommand& renderCommand = commandQueue[i];

			VertexObject& vo = *renderCommand.m_VO;
			Material& material = *renderCommand.m_Material;

			glm::u32vec1 d(renderCommand.m_SSBOInstanceID);
			m_InstanceDataIndexBuffer->Write(&d, 1, indexOffset++);

			const unsigned int nextIndex = i + 1;
			const bool isLastInstance = (commandQueue.size() - 1) == i;
			const bool submitInstance = isLastInstance ||
				commandQueue[nextIndex].m_VO != &vo ||
				commandQueue[nextIndex].m_Material != &material;

			if (submitInstance)
			{
				const FPSCamera& camera = renderContext.GetCamera();
				Shader& shader = *material.m_Shader;
				shader.SetUniformMat4f("u_Projection", camera.GetProjection());
				shader.SetUniformMat4f("u_View", camera.GetTransform());
				shader.SetUniform1f("u_ClipNear", camera.GetNearClip());
				shader.SetUniform1f("u_ClipFar", camera.GetFarClip());
				shader.SetUniform1f("u_GameTime", GMTime::s_Time);
				material.Bind();

				renderer.DrawVertexObject(vo, indexOffset, *m_FrameBufferMS, shader, true);
				indexOffset = 0;
			}
		}

		FrameBuffer::Blit(m_MultiToSingleSampleBlitFBConfig);
	}
}