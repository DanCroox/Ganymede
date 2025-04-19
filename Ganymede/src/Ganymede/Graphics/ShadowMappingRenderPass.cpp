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

namespace Ganymede
{
	bool ShadowMappingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_Framebuffer = renderContext.CreateFrameBuffer("OmniDirectionalShadowMapping", { 512, 512 }, false);
		m_ShadowMapsArray = renderContext.CreateCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray", 6 * 300, RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B16, {512, 512});

		m_Framebuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Depth, *m_ShadowMapsArray);
		
		m_PointLightSortedToCamDistanceOcclusionCheckUBO = renderContext.CreateSSBO("PointlightUpdateBuffer", 1, 320 * (sizeof(PointLight)));
		
		m_ShadowMappingShader = renderContext.LoadShader("OmniDirectionalShadowMappingShader", "res/shaders/OmnidirectionalShadowMapInstances.shader");
		m_PointLightNearClip = 0.01f;
		m_PointLightFarClip = 1000.0f;
		m_PointLightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.f, m_PointLightNearClip, m_PointLightFarClip);

		m_AnimationDataSSBO = renderContext.GetSSBO("AnimationData");
		m_InstanceDataBuffer = renderContext.GetDataBuffer<MeshInstanceVertexData>("MeshInstancesVertexDataBuffer");

		return true;
	}

	void ShadowMappingRenderPass::Execute(RenderContext& renderContext)
	{
		OGLBindingHelper::BindFrameBuffer(*m_Framebuffer);

		const World& world = renderContext.GetWorld();
		ConstListSlice<PointlightWorldObjectInstance*> pointlights = world.GetWorldObjectInstances<PointlightWorldObjectInstance>();

		std::vector<PointLight> pointlightsTotal;
		pointlightsTotal.reserve(pointlights.size());

		int lightId = 0;
		for (PointlightWorldObjectInstance* pointlight : pointlights)
		{
			const int currentLightID = lightId++;

			pointlight->SetLightID(currentLightID);
			pointlight->SetLightingState(LightsManager::LightingState::DynamicShadow);
			
			PointLight& pl = pointlightsTotal.emplace_back();
			const glm::vec3& lightPos = pointlight->GetPosition();
			pl.lightPos = lightPos;
			pl.u_LightID = currentLightID;

			pl.u_ShadowMatrices[0] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			pl.u_ShadowMatrices[1] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			pl.u_ShadowMatrices[2] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
			pl.u_ShadowMatrices[3] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
			pl.u_ShadowMatrices[4] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
			pl.u_ShadowMatrices[5] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

			unsigned int m_DepthCubemapTexture = m_ShadowMapsArray->GetRenderID();
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_DepthCubemapTexture);
			float depthClear = 1.0f;
			glClearTexSubImage(
				m_DepthCubemapTexture, // texture
				0,                     // level
				0, 0,                  // xoffset, yoffset
				currentLightID * 6, // zoffset (erstes Face)
				512, 512,         // Breite, Höhe
				6,                     // depth = 6 Faces
				GL_DEPTH_COMPONENT,    // format
				GL_FLOAT,              // type
				&depthClear            // pointer auf den Wert
			);

		}

		m_PointLightSortedToCamDistanceOcclusionCheckUBO->Write(0, pointlightsTotal.size() * sizeof(PointLight), pointlightsTotal.data());

		Renderer2& renderer = renderContext.GetRenderer();
		unsigned int animationDataOffset = 0;

		auto instances = renderContext.GetWorld().GetWorldObjectInstances<MeshWorldObjectInstance>();
		for (const auto& instance : instances)
		{
			const MeshWorldObject* mwo = instance->GetMeshWorldObject();
			for (MeshWorldObject::Mesh* mesh : mwo->m_Meshes)
			{
				DataBuffer<MeshVertexData> buffer(&mesh->m_Vertices[0], mesh->m_Vertices.size(), DataBufferType::Static);
				VertexObject vo(&mesh->m_VertexIndicies[0], mesh->m_VertexIndicies.size());
				vo.LinkBuffer(buffer);
				vo.LinkBuffer(*m_InstanceDataBuffer, true);

				unsigned int instanceAnimOffset = 0;

				if (SkeletalMeshWorldObjectInstance* skeletalMesh = dynamic_cast<SkeletalMeshWorldObjectInstance*>(instance))
				{
					const std::vector<glm::mat4>& animationFrame = skeletalMesh->GetAnimationBoneData();
					m_AnimationDataSSBO->Write(sizeof(glm::mat4) * animationDataOffset, sizeof(glm::mat4) * animationFrame.size(), (void*)&animationFrame[0]);
					instanceAnimOffset = animationDataOffset;
					animationDataOffset += animationFrame.size();
				}

				std::vector<IData> pds;
				for (int plIdx = 0; plIdx < pointlightsTotal.size(); ++plIdx)
				{
					for (int i = 0; i < 6; ++i)
					{
						IData& pd = pds.emplace_back();
						pd.instance = instance->GetTransform();
						pd.pid = { static_cast<float>(plIdx), static_cast<float>(i), static_cast<float>(instanceAnimOffset) };
						pd.mv = pointlightsTotal[plIdx].u_ShadowMatrices[i];
					}
				}

				m_InstanceDataBuffer->Write(&pds[0], pds.size(), 0);

				m_ShadowMappingShader->SetUniform1f("far_plane", m_PointLightFarClip);

				renderer.DrawVertexObject(vo, pds.size(), *m_Framebuffer, *m_ShadowMappingShader, true);
			}
		}
	}
}