#include "GeometryRenderPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCRenderObject.h"
#include "Ganymede/ECS/Components/GCGPUMeshData.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/World/World.h"
#include "gl/glew.h"

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
		Renderer& renderer = renderContext.GetRenderer();

		renderer.ClearFrameBuffer(*m_FrameBufferMS, true, true);
		renderer.ClearFrameBuffer(*m_FrameBuffer, true, true);

		std::vector<RenderMeshInstanceCommand>& renderInfos = renderContext.m_RenderInfo;
		glEnable(GL_DEPTH_TEST);

		OGLBindingHelper::BindFrameBuffer(*m_FrameBufferMS);

		const RenderMeshInstanceCommandOffsetsByView offset = renderContext.m_RenderInfoOffsets[0];
		NUMBERED_NAMED_COUNTER("Num Drawcalls (FPS Camera)", offset.m_LastIndex);

		for (unsigned int idx = offset.m_StartIndex; idx < offset.m_StartIndex + offset.m_LastIndex; ++idx)
		{
			RenderMeshInstanceCommand& renderInfo = renderInfos[idx];
			MeshWorldObject::Mesh& mesh = *renderContext.m_MeshIDMapping[renderInfo.m_MeshID];
			const Material& material = mesh.m_MaterialHandle.GetData();
			renderContext.BindMaterial(material);
			OGLBindingHelper::BindShader(material.GetShader()->GetRendererID());

			const VertexObject& voPtr = renderContext.GetVO(mesh);
			OGLBindingHelper::BindVertexArrayObject(voPtr.GetRenderID());

			glm::uint offset = renderInfo.m_IndirectCommandIndex * 20;
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)offset);
		}

		FrameBuffer::Blit(m_MultiToSingleSampleBlitFBConfig);
	}
}