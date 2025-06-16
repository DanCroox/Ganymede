#include "LightingRenderPass.h"

#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/World.h"

#include <GL/glew.h>

namespace Ganymede
{
	bool LightingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_FrameBuffer = renderContext.CreateFrameBuffer("Lighting", { 1920, 1080 }, false);
		m_LightingRT = renderContext.CreateSingleSampleRenderTarget("Lighting", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_FrameBuffer->SetFrameBufferAttachment(FrameBuffer::AttachmentType::Color0, *m_LightingRT);
		m_LightingShader = renderContext.LoadShader("Lighting", { "res/shaders/lighting.shader" });
		m_PointLightSortedToCamDistanceSSBO = renderContext.CreateSSBO("PointlightData", 0, 320 * (sizeof(PointLight)), true);
		
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

		const auto pointlightsView = renderContext.GetWorld().GetEntities(Include<GCPointlight, GCTransform>{});
		const unsigned int numPointlights = std::distance(pointlightsView.begin(), pointlightsView.end());

		std::vector<glm::mat4> pointlightVPTransforms;
		pointlightVPTransforms.resize(numPointlights * 6);
		unsigned int currentLightID = 0;

		std::vector<PointLight> pointlights;
		pointlights.reserve(numPointlights);

		for (auto [entity, pl, plTransform] : pointlightsView.each())
		{
			PointLight& pointlight = pointlights.emplace_back();
			pointlight.m_LightColor = glm::vec4(pl.m_Color * pl.m_Brightness, 1);
			pointlight.lightPos = plTransform.GetPosition();
			pointlight.u_LightID = pl.m_CubemapViews[0]->m_FaceIndex / 6;
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

		const RenderView& view = renderContext.GetRenderView(0);
		
		m_LightingShader->SetUniform2i("u_RenderResolution", 1920, 1080);
		m_LightingShader->SetUniform1i("u_PointlightCount", pointlights.size());
		m_LightingShader->SetUniform3f("u_ViewPos", view.GetPosition());
		m_LightingShader->SetUniform2i("u_ViewportResolution", 1920, 1080);

		renderContext.GetRenderer().DrawVertexObject(*m_ScreenVO, 1, *m_FrameBuffer, *m_LightingShader, false);
	}
}