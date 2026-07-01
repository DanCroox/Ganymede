#include "LightingRenderPass.h"

#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/GraphicsShader.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	bool LightingRenderPass::Initialize(RenderContext& renderContext)
	{
		m_LightingRT = renderContext.CreateSingleSampleRenderTarget("Lighting", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { 1920, 1080 });
		m_FrameBuffer = renderContext.CreateFrameBuffer(
			"Lighting",
			{ { FrameBufferAttachmentTypee::Color, 0, m_LightingRT } },
			{ 1920, 1080 });
		m_LightingShader = renderContext.LoadGraphicsShader("Lighting", { "res/shaders/lighting.shader" });
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

		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("PositionsMS"), 0);						//u_GPositions
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("NormalsMS"), 1);						//u_GNormals
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("DepthMS"), 2);							//u_GDepths
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("AlbedoMS"), 3);						//u_GAlbedo
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("MetalRoughnessMS"), 4);				//u_GMetalRough
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("EmissionMS"), 5);						//u_GEmission
		m_LightingShader->BindTexture(*renderContext.GetMultiSampleRenderTarget("ComplexFragmentMS"), 6);				//u_ComplexFragment

		m_LightingShader->BindTexture(*renderContext.GetCubeMapArrayRenderTarget("OmniDirectionalShadowMapArray"), 7);	//u_DepthCubemapTexture

		const RenderView& view = renderContext.GetRenderView(0);
		
		m_LightingShader->SetUniform2i(8, 1920, 1080);																	//u_RenderResolution
		m_LightingShader->SetUniform1i(9, pointlights.size());															//u_PointlightCount
		m_LightingShader->SetUniform3f(10, view.GetPosition());															//u_ViewPos
		m_LightingShader->SetUniform2i(11, 1920, 1080);																	//u_ViewportResolution

		renderContext.GetRenderer().DrawVertexObject(*m_ScreenVO, 1, *m_FrameBuffer, *m_LightingShader, false);
	}
}