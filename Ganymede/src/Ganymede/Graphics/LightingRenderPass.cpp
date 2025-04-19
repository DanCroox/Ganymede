#include "LightingRenderPass.h"

#include "FrameBuffer.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/PointlightWorldObjectInstance.h"
#include "Ganymede/World/World.h"
#include "RenderContext.h"
#include "RendererTypes.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "SSBO.h"
#include "VertexObject.h"

namespace Ganymede
{
	bool LightingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_FrameBuffer = renderContext.CreateFrameBuffer("Lighting", { 1920, 1080 }, false);
		m_LightingRT = renderContext.CreateSingleSampleRenderTarget("Lighting", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color0, *m_LightingRT);
		m_LightingShader = renderContext.LoadShader("Lighting", "res/shaders/lighting.shader");
		m_PointLightSortedToCamDistanceSSBO = renderContext.CreateSSBO("PointlightData", 0, 320 * (sizeof(PointLight)));

		std::vector<glm::vec3> vertices = {
			{ -1, 1, 0},
			{ 1, 1, 0 },
			{ 1, -1, 0 },
			{ -1, -1, 0 },
		};

		std::vector<glm::vec2> uvs = {
			{ 0, 1 },
			{ 1, 1 },
			{ 1, 0 },
			{ 0, 0 }
		};

		unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
		};

		m_ScreenVO = renderContext.CreateVertexObject("ScreenVertexObject", &indices[0], 6);
		m_ScreenVBPos = renderContext.CreateDataBuffer<Vec3VertexData>("ScreenVertexPositionBuffer", &vertices[0], vertices.size(), DataBufferType::Static);
		m_ScreenVBUV = renderContext.CreateDataBuffer<Vec2VertexData>("ScreenVertexUVBuffer", &uvs[0], uvs.size(), DataBufferType::Static);

		m_ScreenVO->LinkBuffer(*m_ScreenVBPos);
		m_ScreenVO->LinkBuffer(*m_ScreenVBUV);

		return true;
	}

	void LightingRenderPass::Execute(RenderContext& renderContext)
	{
		const World& world = renderContext.GetWorld();
		ConstListSlice<PointlightWorldObjectInstance*> pointlightInstances = world.GetWorldObjectInstances<PointlightWorldObjectInstance>();
		std::vector<PointLight> pointlights;
		pointlights.reserve(pointlightInstances.size());
		for (PointlightWorldObjectInstance* pointlight : pointlightInstances)
		{
			PointLight& pl = pointlights.emplace_back();
			pl.m_LightColor = glm::vec4(pointlight->GetColor() * pointlight->GetBrightness(), 1);
			const glm::vec3& lightPos = pointlight->GetPosition();
			pl.lightPos = lightPos;
			pl.u_LightID = pointlight->GetLightID();
			pl.updateShadowMap.x = 0;
		}

		m_PointLightSortedToCamDistanceSSBO->Write(0, pointlights.size() * sizeof(PointLight), pointlights.data());

		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("PositionsMS"), "u_GPositions");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("NormalsMS"), "u_GNormals");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("DepthMS"), "u_GDepths");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("AlbedoMS"), "u_GAlbedo");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("MetalRoughnessMS"), "u_GMetalRough");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("EmissionMS"), "u_GEmission");
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("ComplexFragmentMS"), "u_ComplexFragment");

		m_LightingShader->BindTexture(*renderContext.GetCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray"), "u_DepthCubemapTexture");

		const FPSCamera& camera = renderContext.GetCamera();

		m_LightingShader->SetUniform1f("u_ClipNear", camera.GetNearClip());
		m_LightingShader->SetUniform1f("u_ClipFar", camera.GetFarClip());
		m_LightingShader->SetUniform2i("u_RenderResolution", 1920, 1080);
		m_LightingShader->SetUniform1i("u_PointlightCount", pointlights.size());
		m_LightingShader->SetUniform3f("u_ViewPos", camera.GetPosition());
		m_LightingShader->SetUniform2i("u_ViewportResolution", 1920, 1080);

		renderContext.GetRenderer().DrawVertexObject(*m_ScreenVO, 1, *m_FrameBuffer, *m_LightingShader, false);
	}
}