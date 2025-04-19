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

		m_AnimationDataSSBO = renderContext.CreateSSBO("AnimationData", 2, 80000 * sizeof(glm::mat4));
		m_InstanceDataBuffer = renderContext.CreateDataBuffer<MeshInstanceVertexData>("MeshInstancesVertexDataBuffer", nullptr, 100000, DataBufferType::Dynamic);

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
		Renderer2& renderer = renderContext.GetRenderer();

		renderer.ClearFrameBuffer(*m_FrameBufferMS, true, true);
		renderer.ClearFrameBuffer(*m_FrameBuffer, true, true);

		unsigned int animationDataOffset = 0;

		auto instances = renderContext.GetWorld().GetWorldObjectInstances<MeshWorldObjectInstance>();
		for (auto instance : instances)
		{
			const MeshWorldObject* mwo = instance->GetMeshWorldObject();
			for (MeshWorldObject::Mesh* mesh : mwo->m_Meshes)
			{
				DataBuffer<MeshVertexData> buffer(&mesh->m_Vertices[0], mesh->m_Vertices.size(), DataBufferType::Static);
				VertexObject vo(&mesh->m_VertexIndicies[0], mesh->m_VertexIndicies.size());
				vo.LinkBuffer(buffer);
				vo.LinkBuffer(*m_InstanceDataBuffer, true);

				const FPSCamera& camera = renderContext.GetCamera();

				IData pd;
				pd.instance = instance->GetTransform();
				pd.pid = { 0.0f, 0.0f };
				pd.mv = camera.GetTransform() * instance->GetTransform();

				if (SkeletalMeshWorldObjectInstance* skeletalMesh = dynamic_cast<SkeletalMeshWorldObjectInstance*>(instance))
				{
					const std::vector<glm::mat4>& animationFrame = skeletalMesh->GetAnimationBoneData();
					m_AnimationDataSSBO->Write(sizeof(glm::mat4) * animationDataOffset, sizeof(glm::mat4) * animationFrame.size(), (void*)&animationFrame[0]);
					pd.pid.m_AnimationDataOffset = animationDataOffset;
					animationDataOffset += animationFrame.size();
				}
				m_InstanceDataBuffer->Write(&pd, 1, 0);

				Shader& meshShader = *mesh->m_Material.m_Shader;
				meshShader.SetUniformMat4f("u_Projection", camera.GetProjection());
				meshShader.SetUniformMat4f("u_View", camera.GetTransform());
				meshShader.SetUniform1f("u_ClipNear", camera.GetNearClip());
				meshShader.SetUniform1f("u_ClipFar", camera.GetFarClip());
				meshShader.SetUniform1f("u_GameTime", GMTime::s_Time);
				mesh->m_Material.Bind();

				renderer.DrawVertexObject(vo, 1, *m_FrameBufferMS, meshShader, true);
			}
		}

		FrameBuffer::Blit(m_MultiToSingleSampleBlitFBConfig);
	}
}