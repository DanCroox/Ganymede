#include "GeometryRenderPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Data/StaticData.h"
#include "Ganymede/ECS/Components/GCGPUMeshData.h"
#include "Ganymede/ECS/Components/GCRenderObject.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	bool GeometryRenderPass::Initialize(RenderContext& renderContext)
	{
		m_PositionsRTMS = renderContext.CreateMultiSampleRenderTarget("PositionsMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_NormalsRTMS = renderContext.CreateMultiSampleRenderTarget("NormalsMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_AlbedoRTMS = renderContext.CreateMultiSampleRenderTarget("AlbedoMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_MetalRoughnessRTMS = renderContext.CreateMultiSampleRenderTarget("MetalRoughnessMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_EmissionRTMS = renderContext.CreateMultiSampleRenderTarget("EmissionMS", 4, RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_DepthRTMS = renderContext.CreateMultiSampleRenderTarget("DepthMS", 4, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_ComplexFragmentMS = renderContext.CreateMultiSampleRenderTarget("ComplexFragmentMS", 4, RenderTargetTypes::ComponentType::R, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
 
		m_FrameBufferMS = renderContext.CreateFrameBuffer(
			"GeometryMS", 
			{
				{ FrameBufferAttachmentTypee::Color, 0, m_PositionsRTMS },
				{ FrameBufferAttachmentTypee::Color, 1, m_NormalsRTMS },
				{ FrameBufferAttachmentTypee::Color, 2, m_AlbedoRTMS },
				{ FrameBufferAttachmentTypee::Color, 3, m_MetalRoughnessRTMS },
				{ FrameBufferAttachmentTypee::Color, 4, m_EmissionRTMS },
				{ FrameBufferAttachmentTypee::Color, 5, m_ComplexFragmentMS },
				{ FrameBufferAttachmentTypee::Depth, 6, m_DepthRTMS }
			},
			{ 1920, 1080 }
		);

		m_PositionsRT = renderContext.CreateSingleSampleRenderTarget("Positions", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_NormalsRT = renderContext.CreateSingleSampleRenderTarget("Normals", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_AlbedoRT = renderContext.CreateSingleSampleRenderTarget("Albedo", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_MetalRoughnessRT = renderContext.CreateSingleSampleRenderTarget("MetalRoughness", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { 1920, 1080 });
		m_EmissionRT = renderContext.CreateSingleSampleRenderTarget("Emission", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_DepthRT = renderContext.CreateSingleSampleRenderTarget("Depth", RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		
		m_FrameBuffer = renderContext.CreateFrameBuffer("Geometry",
			{
				{ FrameBufferAttachmentTypee::Color, 0, m_PositionsRT },
				{ FrameBufferAttachmentTypee::Color, 1, m_NormalsRT },
				{ FrameBufferAttachmentTypee::Color, 2, m_AlbedoRT },
				{ FrameBufferAttachmentTypee::Color, 3, m_MetalRoughnessRT },
				{ FrameBufferAttachmentTypee::Color, 4, m_EmissionRT },
				{ FrameBufferAttachmentTypee::Depth, 5, m_DepthRT }
			},
			{ 1920, 1080 }
		);

		ssbo_IndirectDrawCmds = renderContext.GetSSBO("IndirectDrawCommands");

		return true;
	}

	void GeometryRenderPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("Geometry Pass");
		Renderer& renderer = renderContext.GetRenderer();

		renderer.ClearFrameBuffer(*m_FrameBufferMS, true, true);
		renderer.ClearFrameBuffer(*m_FrameBuffer, true, true);

		std::vector<RenderMeshInstanceCommand>& renderInfos = renderContext.m_RenderInfo;

		const RenderMeshInstanceCommandOffsetsByView offset = renderContext.m_RenderInfoOffsets[0];
		NUMBERED_NAMED_COUNTER("Num Drawcalls (FPS Camera)", offset.m_LastIndex);

		for (unsigned int idx = offset.m_StartIndex; idx < offset.m_StartIndex + offset.m_LastIndex; ++idx)
		{
			RenderMeshInstanceCommand& renderInfo = renderInfos[idx];
			MeshWorldObject::Mesh& mesh = *renderContext.m_MeshIDMapping[renderInfo.m_MeshID];
			const Material& material = mesh.m_MaterialHandle.GetData();
			const VertexObject& voPtr = renderContext.GetVO(mesh);

			renderer.DrawIndirect(voPtr, *ssbo_IndirectDrawCmds, renderInfo.m_IndirectCommandIndex, *m_FrameBufferMS, material, true);
		}

		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Color, 0, FrameBufferAttachmentTypee::Color, 0, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Color, 1, FrameBufferAttachmentTypee::Color, 1, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Color, 2, FrameBufferAttachmentTypee::Color, 2, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Color, 3, FrameBufferAttachmentTypee::Color, 3, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Color, 4, FrameBufferAttachmentTypee::Color, 4, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
		m_FrameBuffer->Blit(*m_FrameBufferMS, FrameBufferAttachmentTypee::Depth, 5, FrameBufferAttachmentTypee::Depth, 5, { 0, 0, 1920, 1080 }, { 0, 0, 1920, 1080 }, FrameBufferBlitFilterType::Nearest);
	}
}