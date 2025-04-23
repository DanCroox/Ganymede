#include "Renderer.h"

#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include "Ganymede/World/PointlightWorldObject.h"



#include <GL/glew.h>

#include <chrono>
#include <random>

#include "IndexBuffer.h"

#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "SSBO.h"

#include "ShaderManager.h"
#include "Ganymede/Runtime/GMTime.h"


#include <algorithm> 
#include <thread> 
#include <mutex>
#include <iostream>
#include <set>

#include "Ganymede/World/World.h"

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "Ganymede/Log/Log.h"

namespace Ganymede
{
	namespace Renderer_Private
	{
		static const std::string locTextureShaderName = std::string("u_Texture");

		static float locLerp(float a, float b, float f)
		{
			return a + f * (b - a);
		}
	}

	Renderer::Renderer(ShaderManager& shaderManager)
	{
		m_LightingShader = shaderManager.RegisterAndLoadShader("lighting");
		m_SSAOShader = shaderManager.RegisterAndLoadShader("ssao");
		m_SSRShader = shaderManager.RegisterAndLoadShader("ssr");
		m_VolumetricLightShader = shaderManager.RegisterAndLoadShader("volumetriclight");
		m_BlurShader = shaderManager.RegisterAndLoadShader("blurshader");
		m_BlurSSRShader = shaderManager.RegisterAndLoadShader("blurshaderssr");
		m_GBufferShader = shaderManager.RegisterAndLoadShader("gbuffer");
		m_OmnidirectionalShadowShader = shaderManager.RegisterAndLoadShader("OmnidirectionalShadowMap");
		m_OmnidirectionalShadowInstancesShader = shaderManager.RegisterAndLoadShader("OmnidirectionalShadowMapInstances");
		m_DebugDrawMeshShader = shaderManager.RegisterAndLoadShader("debugdrawmesh");
	
		m_OcclusionQueryShaderOmni = shaderManager.RegisterAndLoadShader("occlusionOmindirectional");
		m_ViewportShader = shaderManager.RegisterAndLoadShader("ScreenShader");
		m_MSAAResolver = shaderManager.RegisterAndLoadShader("msaaresolve");
		m_DownsampleMaxShader = shaderManager.RegisterAndLoadShader("downsamplemax");
		m_PBBDownsampleShader = shaderManager.RegisterAndLoadShader("pbbdownsample");
		m_PBBUpsampleShader = shaderManager.RegisterAndLoadShader("pbbupsample");

		// Setup gpu timers
		InitGPUTimers();

		// 1.f = aspect ratio cause shadow maps are square
		m_PointLightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.f, m_PointLightNearClip, m_PointLightFarClip);

		// Frustum culling thread count

		//m_RendererThreadPool.resize(80);
		m_RenderDataBuffer.Initialize(m_RendererThreadPool.size());


		float vertices[] = {
		-1, 1, 0,
		 1, 1, 0,
		 1, -1, 0,
		-1, -1, 0,
		};

		float uvs[] = {
			0, 1,
			1, 1,
			1, 0,
			0, 0
		};

		unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
		};

		m_PointLightSortedToCamDistanceUBO = new SSBO(0, 320 * (sizeof(PointLight)), false);
		m_PointLightSortedToCamDistanceOcclusionCheckUBO = new SSBO(1, 320 * (sizeof(PointLight)),false);

		GLCall(glGenVertexArrays(1, &m_ViewportQuadVAO));
		GLCall(glBindVertexArray(m_ViewportQuadVAO));

		GLCall(glGenBuffers(1, &m_ViewportQuadVertexBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_ViewportQuadVertexBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), &vertices[0], GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
		GLCall(glEnableVertexAttribArray(0));

		GLCall(glGenBuffers(1, &m_ViewportQuadUVsBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_ViewportQuadUVsBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), &uvs[0], GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));
		GLCall(glEnableVertexAttribArray(1));

		GLCall(glGenBuffers(1, &m_ViewportQuadIndexBuffer));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ViewportQuadIndexBuffer));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW));

		GLCall(glBindVertexArray(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	Renderer::~Renderer()
	{
		DeleteGPUTimers();

		delete m_PointLightSortedToCamDistanceUBO;
		delete m_PointLightSortedToCamDistanceOcclusionCheckUBO;

		GLCall(glBindVertexArray(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

		GLCall(glDeleteVertexArrays(1, &m_ViewportQuadVAO));

		GLCall(glDeleteBuffers(1, &m_ViewportQuadVertexBuffer));
		GLCall(glDeleteBuffers(1, &m_ViewportQuadIndexBuffer));

		GLCall(glDeleteTextures(1, &m_FrameTexture));
		GLCall(glDeleteTextures(1, &m_DepthTexture));
		GLCall(glDeleteFramebuffers(1, &m_FrameBuffer));

		GLCall(glDeleteFramebuffers(1, &m_DepthCubemapFBO));
		GLCall(glDeleteTextures(1, &m_DepthCubemapTexture));

		GLCall(glDeleteFramebuffers(1, &m_OcclusionQueryFBO));
	}

	static unsigned int m_NumHIZMips = 8;
	static unsigned int m_NumSSRLightingMips = 8;

	void Renderer::CreateFrameBuffersIfRequested()
	{
		if (!m_CreateFrameBuffer)
		{
			return;
		}

		m_SSRPassSize = m_ScreenSize / glm::u32vec2(2);

		m_CreateFrameBuffer = false;
	
		//SSR FBO
		GLCall(glGenFramebuffers(1, &m_SSRFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSRFBO));

		GLCall(glGenTextures(1, &m_SSRTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_SSRPassSize.x, m_SSRPassSize.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glGenTextures(1, &m_SSRTextureNormals));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureNormals));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_SSRPassSize.x, m_SSRPassSize.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glGenTextures(1, &m_SSRHalfDepthMax));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRHalfDepthMax));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_SSRPassSize.x, m_SSRPassSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glGenTextures(1, &m_SSRTextureBlurred));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureBlurred));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glGenTextures(1, &m_SSRTextureMips));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureMips));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_NumSSRLightingMips - 1));
		glm::u32vec2 mipSizee(m_SSRPassSize.x, m_SSRPassSize.y);
		for (int i = 0; i < m_NumSSRLightingMips; ++i)
		{
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA32F, mipSizee.x, mipSizee.y, 0, GL_RGBA, GL_FLOAT, NULL);
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			mipSizee /= 2;
		}
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		// SSAO fbo
		GLCall(glGenFramebuffers(1, &m_SSAOFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO));

		GLCall(glGenTextures(1, &m_SSAOTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAOTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, m_SSAOTextureResolution.x, m_SSAOTextureResolution.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		GLCall(glGenTextures(1, &m_SSAOTextureBlurred));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAOTextureBlurred));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, m_SSAOTextureResolution.x, m_SSAOTextureResolution.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		GLCall(glGenTextures(1, &m_SSAOPositionTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAOPositionTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glGenTextures(1, &m_SSAONormalTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAONormalTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		mySSAOKernel.clear();
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0;
			scale = Renderer_Private::locLerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			mySSAOKernel.push_back(sample);
		}

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		GLCall(glGenTextures(1, &m_SSAONoiseTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOTexture, 0));
		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);

		// MS GBuffer
	
		GLCall(glGenFramebuffers(1, &m_MSGBufferIDs.m_GBuffer));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_MSGBufferIDs.m_GBuffer));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_Positions));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Positions));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Positions, 0));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_Normals));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals, 0));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_Albedos));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Albedos));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Albedos, 0));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_MetalRough));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_MetalRough));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_MetalRough, 0));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_Emission));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Emission));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Emission, 0));

		GLCall(glGenTextures(1, &m_ComplexFragmentTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_R8, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture, 0));

		GLCall(glGenTextures(1, &m_MSGBufferIDs.m_Depth));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, GL_TRUE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth, 0));

		const unsigned int atts[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		GLCall(glDrawBuffers(6, atts));
	

		// Gbuffer
		GLCall(glGenFramebuffers(1, &m_GBufferIDs.m_GBuffer));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferIDs.m_GBuffer));
		//glEnable(GL_FRAMEBUFFER_SRGB);
		GLCall(glGenTextures(1, &m_GBufferIDs.m_Positions));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Positions));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferIDs.m_Positions, 0));

		GLCall(glGenTextures(1, &m_GBufferIDs.m_Normals));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Normals));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferIDs.m_Normals, 0));

		GLCall(glGenTextures(1, &m_GBufferIDs.m_Albedos));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Albedos));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferIDs.m_Albedos, 0));

		GLCall(glGenTextures(1, &m_GBufferIDs.m_MetalRough));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_MetalRough));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBufferIDs.m_MetalRough, 0));

		GLCall(glGenTextures(1, &m_GBufferIDs.m_Emission));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Emission));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_GBufferIDs.m_Emission, 0));
	
		GLCall(glGenTextures(1, &m_GBufferIDs.m_Depth));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferIDs.m_Depth, 0));

		//DownSample framebuffer
		// need these vectors in world space (not viewspace)
		//GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_SSAOPositionTexture, 0));
		//GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, m_SSAONormalTexture, 0));

		const unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		GLCall(glDrawBuffers(5, attachments));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		GLCall(glGenFramebuffers(1, &m_HZFramebuffer));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_HZFramebuffer));

		GLCall(glGenTextures(1, &m_HZTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_HZTexture));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_NumHIZMips - 1);

		glm::u32vec2 mipSize(m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale);
		for (int i = 0; i < m_NumHIZMips; ++i)
		{
			glTexImage2D(GL_TEXTURE_2D, i, GL_DEPTH_COMPONENT32F, mipSize.x, mipSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			mipSize /= 2;
		}

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_HZTexture, 0));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	
		//  Bloom FBO
		GLCall(glGenFramebuffers(1, &m_PBBFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_PBBFBO));

		GLCall(glGenTextures(1, &m_PBBTextureMips));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PBBTextureMips));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_NumPBBMips - 1);

		mipSize = glm::u32vec2(m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale);
		for (int i = 0; i < m_NumPBBMips; ++i)
		{
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA32F, mipSize.x, mipSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			mipSize /= 2;
		}
		//GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PBBTextureMips, 0));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		// Create occlusion query framebuffer (uses different depth textures for occlusion testing)
		GLCall(glGenFramebuffers(1, &m_OcclusionQueryFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_OcclusionQueryFBO));

		//GLCall(glDrawBuffer(GL_NONE));
		//GLCall(glReadBuffer(GL_NONE));

		GLCall(glGenTextures(1, &m_OcclusionQueryDebugTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_OcclusionQueryDebugTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_OcclusionQueryDebugTexture, 0));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));



		// Create Depth Cubemap FrameBuffer
		GLCall(glGenFramebuffers(1, &m_DepthCubemapFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthCubemapFBO));

		GLCall(glGenTextures(1, &m_DepthCubemapTexture));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_DepthCubemapTexture))
		GLCall(glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT16, m_ShadowMapSize, m_ShadowMapSize, 6 * 300, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0));
	
		GLCall(glGenTextures(1, &m_PositionCubemapTexture));
		GLCall(glBindTexture(1, m_PositionCubemapTexture))
		GLCall(glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, m_ShadowMapSize, m_ShadowMapSize, 6 * 300, 0, GL_RGB, GL_FLOAT, NULL));
		//GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, m_ShadowMapSize* m_RenderScale, m_ShadowMapSize* m_RenderScale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0));

		GLCall(glClearColor(1,1,1,1));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		// Create volumetric light framebuffer
		GLCall(glGenFramebuffers(1, &m_VolumetricLightFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_VolumetricLightFBO));

		GLCall(glGenTextures(1, &m_VolumetricLightTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_VolumetricLightTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, m_VolumetricLightMapSize.x, m_VolumetricLightMapSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_VolumetricLightTexture, 0));

		GLCall(glGenTextures(1, &m_BlurredVolumetricTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_BlurredVolumetricTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, m_VolumetricLightMapSize.x, m_VolumetricLightMapSize.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		//Blur fbo
		GLCall(glGenFramebuffers(1, &m_BlurFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO));

		GLCall(glGenTextures(1, &m_BlurTextureCache));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_BlurTextureCache));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, m_VolumetricLightMapSize.x, m_VolumetricLightMapSize.y, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLCall(glGenTextures(1, &m_BlurSSRTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_BlurSSRTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		// Create final framebuffer
		GLCall(glGenFramebuffers(1, &m_LightingFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_LightingFBO));

		GLCall(glGenTextures(1, &m_LightingTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_LightingTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale, 0, GL_RGBA, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_LightingTexture, 0));

		// Unbind FrameBuffer
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void Renderer::GetSortedPointlightsByDistanceToCamera(std::vector<PointlightWorldObjectInstance*>& pointlights) const
	{
		SCOPED_TIMER("Sort lights by distance");

		const World& world = *m_World;
		const FPSCamera& camera = *m_Camera;

		struct LightWithCamDistance
		{
			PointlightWorldObjectInstance* m_Light;
			float m_CamDistance = 0;
		};

		std::vector<LightWithCamDistance> lightsWithDistance;

		const auto lights = world.GetWorldObjectInstances<PointlightWorldObjectInstance>();
		for (PointlightWorldObjectInstance* light : lights)
		{
			if (light->GetBrightness() <= 0 ||
				(light->GetColor().x == 0 && light->GetColor().y == 0 && light->GetColor().z == 0))
				continue;

			LightWithCamDistance lwcd;
			lwcd.m_Light = light;
			lwcd.m_CamDistance = glm::length(light->GetPosition() - camera.GetPosition());
			lightsWithDistance.push_back(lwcd);
		}

		std::qsort(lightsWithDistance.data(), lightsWithDistance.size(), sizeof(LightWithCamDistance),
			[](const void* aa, const void* bb)
			{
				const LightWithCamDistance& a = *((const LightWithCamDistance*)aa);
				const LightWithCamDistance& b = *((const LightWithCamDistance*)bb);


				if (a.m_Light->GetLightingState() == LightsManager::LightingState::Initialize || b.m_Light->GetLightingState() == LightsManager::LightingState::Initialize)
				{
					if (a.m_Light->GetLightingState() == LightsManager::LightingState::Initialize && b.m_Light->GetLightingState() == LightsManager::LightingState::Initialize)
						return 0;

					if (a.m_Light->GetLightingState() == LightsManager::LightingState::Initialize && b.m_Light->GetLightingState() != LightsManager::LightingState::Initialize)
						return -1;

					if (a.m_Light->GetLightingState() != LightsManager::LightingState::Initialize && b.m_Light->GetLightingState() == LightsManager::LightingState::Initialize)
						return 1;
				}

				if (a.m_Light->GetImportance() != b.m_Light->GetImportance())
				{
					return a.m_Light->GetImportance() > b.m_Light->GetImportance() ? -1 : 1;
				}

				if (a.m_CamDistance == b.m_CamDistance)
					return 0;

				return a.m_CamDistance < b.m_CamDistance ? -1 : 1;
			}
		);

		for (const LightWithCamDistance& light : lightsWithDistance)
		{
			pointlights.push_back(light.m_Light);
		}
	}

	void Renderer::RenderCubeShadowMaps(const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances, const std::vector<PointLight>& pointlightsForOcclusionTest)
	{
		GLCall(glViewport(0, 0, m_ShadowMapSize, m_ShadowMapSize));
		GLCall(glEnable(GL_DEPTH_TEST));

		StartGPUTimer(GPUTimerType::PointlightShadowMapPass);

		m_PointLightSortedToCamDistanceOcclusionCheckUBO->Write(0, pointlightsForOcclusionTest.size() * sizeof(PointLight), (void*) & pointlightsForOcclusionTest[0]);
		
		m_OmnidirectionalShadowInstancesShader->Bind();
		m_OmnidirectionalShadowInstancesShader->SetUniform1f("far_plane", m_PointLightFarClip);
		m_OmnidirectionalShadowInstancesShader->BindUBOBlock(*m_PointLightSortedToCamDistanceOcclusionCheckUBO, "PointLightDataBlock");
		m_OmnidirectionalShadowInstancesShader->SetUniform1i("u_PointlightCount", pointlightsForOcclusionTest.size());

		RenderScene(*m_World, m_Camera->GetTransform(), m_PointLightProjectionMatrix, meshInstances, RenderPass::LightDepth, &pointlightsForOcclusionTest);

		m_OmnidirectionalShadowInstancesShader->Unbind();

		StopGPUTimer();
	}

	void Renderer::UpdatePointlightShadowMaps(const std::vector<PointlightWorldObjectInstance*>& pointlightsClosestToCamera, std::vector<PointLight>& pointlightsForOcclusionTest)
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_DepthCubemapFBO));

		std::vector<PointLight> pointlightsTotal;
		unsigned int numLights = 0;
		
		for (PointlightWorldObjectInstance* pointlight : pointlightsClosestToCamera)
		{
			PointLight pl;
			pl.m_LightColor = glm::vec4(pointlight->GetColor() * pointlight->GetBrightness(), 1);
			const glm::vec3& lightPos = pointlight->GetPosition();
			pl.lightPos = lightPos;
			pl.u_LightID = pointlight->GetLightID();

			if (numLights < LightsManager::MAX_POINTLIGHTS_DYNAMIC_SHADOWS)
			{
				pointlight->SetLightingState(LightsManager::LightingState::DynamicShadow);
				++numLights;

				if (pointlight->DoUpdateShadowMap())
				{
					//pl.u_ShadowMatrices[0] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
					//pl.u_ShadowMatrices[1] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
					//pl.u_ShadowMatrices[2] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
					//pl.u_ShadowMatrices[3] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
					//pl.u_ShadowMatrices[4] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
					//pl.u_ShadowMatrices[5] = m_PointLightProjectionMatrix * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));


					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 0));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 0));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 1));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 1));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 2));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 2));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 3));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 3));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 4));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 4));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0, (pointlight->GetLightID() * 6) + 5));
					GLCall(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0, (pointlight->GetLightID() * 6) + 5));
					GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
				}

				pointlightsForOcclusionTest.push_back(pl);
			}
			else if (pointlight->GetLightingState() != LightsManager::LightingState::Initialize)
			{
				pointlight->SetLightingState(LightsManager::LightingState::StaticShadow);
			}

			pointlightsTotal.push_back(pl);
		}

		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_PositionCubemapTexture, 0));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTexture, 0));

		m_PointLightSortedToCamDistanceUBO->Write(0, pointlightsTotal.size() * sizeof(PointLight), &pointlightsTotal[0]);
	}

	// TODO: This frustum culling function has multiple problems and has to be refactored!
	void Renderer::CollectGeometryDataForRendering(const std::vector<PointLight>& pointlightsForOcclusionTest,
		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& pointlightRenderData,
		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& cameraRenderData)
	{
		SCOPED_TIMER("Collect Geometry For Rendering");

		const glm::mat4& cameraProjection = m_Camera->GetProjection();
		const glm::mat4& cameraTransform = m_Camera->GetTransform();
		const glm::mat4 cameraViewProjection = cameraProjection * cameraTransform;

		const auto instances = m_World->GetWorldObjectInstances<MeshWorldObjectInstance>();
		const unsigned int numInstances = instances.size();
		const unsigned int numThreads = m_RendererThreadPool.size();
		const unsigned int threadsRequired = glm::min(numThreads, numInstances);
		auto nextIterator = instances.begin();

		unsigned int numInstancesLeft = numInstances;
		for (unsigned int threadIndex = 0; threadIndex < threadsRequired; ++threadIndex)
		{
			unsigned int currentIncrement = glm::ceil(static_cast<float>(numInstancesLeft) / (numThreads- threadIndex));
			numInstancesLeft -= currentIncrement;

			auto& outDataForCamera = m_RenderDataBuffer.m_CameraRenderDataBuffer[threadIndex];
			auto& outDataForPointlights = m_RenderDataBuffer.m_PointlightsRenderDataBuffer[threadIndex];

			outDataForCamera.clear();
			outDataForPointlights.clear();

			Thread& thread = m_RendererThreadPool[threadIndex];
			thread.m_WorkerFunction = [nextIterator, currentIncrement, &cameraViewProjection, &cameraTransform, &outDataForCamera, &outDataForPointlights, &pointlightsForOcclusionTest]() mutable
			{
				for (unsigned int instanceIndex = 0; instanceIndex < currentIncrement; ++instanceIndex)
				{
					MeshWorldObjectInstance* mwoi = *nextIterator;

					glm::mat4 instanceTransform = mwoi->GetTransform();
					glm::mat4 camMvp = cameraViewProjection * instanceTransform;
					glm::mat4 camMv = cameraTransform * instanceTransform;

					for (const MeshWorldObject::Mesh* mesh : mwoi->GetMeshWorldObject()->m_Meshes)
					{
						auto [it, inserted] = outDataForCamera.emplace(mesh, std::vector<MeshInstancess>());
						std::vector<MeshInstancess>& meshInstancesCamera = (*it).second;

						// Frustum cull from camera projection
						MeshInstancess mInstance;
						mInstance.m_MVP = camMv;

						int clipSides[6];
						for (const MeshWorldObject::Mesh::BoundingBoxVertex& bbVert : mesh->m_BoundingBoxVertices)
						{
							glm::vec4 clipPoint = camMvp * glm::vec4(bbVert.m_Position, 1);

							clipSides[0] += clipPoint.x < -clipPoint.w; //left of Left plane
							clipSides[1] += clipPoint.x > clipPoint.w;  //right of Right plane
							clipSides[2] += clipPoint.y < -clipPoint.w; //below Bottom plane
							clipSides[3] += clipPoint.y > clipPoint.w;  //above Top plane
							clipSides[4] += clipPoint.z < -clipPoint.w; //in front of Near plane
							clipSides[5] += clipPoint.z > clipPoint.w;  //behind Far plane
						}

						const bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
							clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

						if (!isOutSideFrustum)
						{
							mInstance.m_Instance = mwoi;
							meshInstancesCamera.push_back(mInstance);
						}

						if (!mwoi->GetCastShadows())
						{
							++nextIterator;
							continue;
						}
						auto [itpl, insertedpl] = outDataForPointlights.emplace(mesh, std::vector<MeshInstancess>());
						std::vector<MeshInstancess>& meshInstancesPointlights = itpl->second;

						// Frustum cull from poinlight cubemap faces projections
						{
							for (unsigned int lightIndex = 0; lightIndex < pointlightsForOcclusionTest.size(); ++lightIndex)
							{
								const PointLight& pl = pointlightsForOcclusionTest[lightIndex];

								//TODO: Add distace culling for geometry which is outside of light radius of this pointlight
								for (int faceIdx = 0; faceIdx < 6; ++faceIdx)
								{
									MeshInstancess mInstance;
									//mInstance.m_MVP = pl.u_ShadowMatrices[faceIdx];
									//const glm::mat4 mvp = pl.u_ShadowMatrices[faceIdx] * instanceTransform;

									int clipSides[6];
									for (const MeshWorldObject::Mesh::BoundingBoxVertex& bbVert : mesh->m_BoundingBoxVertices)
									{
										//glm::vec4 clipPoint = mvp * glm::vec4(bbVert.m_Position, 1);

										//clipSides[0] += clipPoint.x < -clipPoint.w; //left of Left plane
										//clipSides[1] += clipPoint.x > clipPoint.w;  //right of Right plane
										//clipSides[2] += clipPoint.y < -clipPoint.w; //below Bottom plane
										//clipSides[3] += clipPoint.y > clipPoint.w;  //above Top plane
										//clipSides[4] += clipPoint.z < -clipPoint.w; //in front of Near plane
										//clipSides[5] += clipPoint.z > clipPoint.w;  //behind Far plane
									}

									const bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
										clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

									if (!isOutSideFrustum)
									{
										mInstance.m_Instance = mwoi;
										mInstance.m_LightIndex = lightIndex;
										mInstance.m_TargetLayerID = faceIdx;

										meshInstancesPointlights.push_back(mInstance);
									}
								}
							}
						}
					}
					++nextIterator;
				}
			};

			thread.Start();
			
			// advance iterator
			for (int i = 0; i < currentIncrement; ++i)
			{
				++nextIterator;
			}
		}
		{
			SCOPED_TIMER("Frustum Culling");

			for (int i = 0; i < numThreads; ++i)
			{
				while (m_RendererThreadPool[i].IsRunning());

				for (const auto& [mesh, instances] : m_RenderDataBuffer.m_CameraRenderDataBuffer[i]) {
					auto [iter, inserted] = cameraRenderData.emplace(mesh, std::vector<MeshInstancess>());
					std::vector<MeshInstancess>& data = (*iter).second;
					data.insert(data.end(), instances.begin(), instances.end());
				}

				for (const auto& [mesh, instances] : m_RenderDataBuffer.m_PointlightsRenderDataBuffer[i]) {
					auto [iter, inserted] = pointlightRenderData.emplace(mesh, std::vector<MeshInstancess>());
					std::vector<MeshInstancess>& data = (*iter).second;
					data.insert(data.end(), instances.begin(), instances.end());
				}
			}
		}
	}

	void Renderer::Draw(const World& world, WorldPartitionManager& worldPartitionManager, const FPSCamera& camera)
	{
		SCOPED_TIMER("Draw function");

		m_WorldPartitionManager = &worldPartitionManager;
		m_World = &world;
		m_Camera = &camera;

		CreateFrameBuffersIfRequested();

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			return;
		}

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// Search for non occluded geometry from camera perspective
		//FindCameraNonOccludedGeometry(allGeometry);

		// Sort pointlights according to distance to camera
		std::vector<PointlightWorldObjectInstance*> allAointlightsClosestToCamera;
		GetSortedPointlightsByDistanceToCamera(allAointlightsClosestToCamera);
		NUMBERED_NAMED_COUNTER("Pointlights total", allAointlightsClosestToCamera.size());

		// Update lights lightmap data and check which light is concidered to update its shadow map
		m_LightsManager.Update(allAointlightsClosestToCamera);
	

		std::vector<PointLight> pointlightsToUpdateShadowMaps;
		UpdatePointlightShadowMaps(allAointlightsClosestToCamera, pointlightsToUpdateShadowMaps);

		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>> cameraRenderData;
		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>> pointlightRenderData;
		CollectGeometryDataForRendering(pointlightsToUpdateShadowMaps, pointlightRenderData, cameraRenderData);
		RenderCubeShadowMaps(pointlightRenderData, pointlightsToUpdateShadowMaps);
	
		// Search for non occluded geometry from all pointlights perspectives
		//FindPointLightsNonOccludedGeometry(allGeometry, pl);

		WorldPartitionManager& wpMan = *m_WorldPartitionManager;
	
		StartGPUTimer(GPUTimerType::OcclusionCullingPass);
		GLCall(glViewport(0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_OcclusionQueryFBO));
		GLCall(glClearColor(0, 0, 0, 0.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // we're not using the stencil buffer now
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDisable(GL_CULL_FACE));

		GLCall(glDepthMask(GL_FALSE));
		GLCall(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferIDs.m_Depth, 0)); // We're reusing the existing depth texture 

		//glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth);

		if (!m_DebugDisableOcclusionCulling)
		{
			m_OcclusionCulledInstances.clear();
			wpMan.Prepare(*m_Camera);
			m_OcclusionCulledInstances = wpMan.UpdateVisibility();
			wpMan.Finish();
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GLCall(glDisable(GL_CULL_FACE));
		GLCall(glDepthMask(GL_TRUE));
		GLCall(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		StopGPUTimer();

		//inline const std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>>& GetAllWorldObjectInstances() { return m_WorldObjectInstancesByType; }

		cameraRenderData.clear(); 
		const auto meshWorldObjectInstances = m_World->GetWorldObjectInstances<MeshWorldObjectInstance>();

		for (const auto mwoi : meshWorldObjectInstances)
		{
			for (const MeshWorldObject::Mesh* mesh : mwoi->GetMeshWorldObject()->m_Meshes)
			{
				const auto& it = cameraRenderData.find(mesh);

				std::vector<MeshInstancess>* minstances;
				if (it == cameraRenderData.end())
				{
					auto [itpl, insertedpl] = cameraRenderData.emplace(mesh, std::vector<MeshInstancess>());
					minstances = &(*itpl).second;
				}
				else
				{
					minstances = &it->second;
				}

				glm::mat4 camMv = m_Camera->GetTransform() * mwoi->GetTransform();

				minstances->push_back({ mwoi, camMv, 0,0 });
			}
		}

		GBufferPass(world, camera, cameraRenderData);

		//SSAOPass();

		LightVolumetricPass(world, camera, allAointlightsClosestToCamera);
		LightingPass(world, camera, allAointlightsClosestToCamera);

		SSRPass();

		PBBPass();

		CompositePass(camera);

		m_CameraTransformLastFrame = camera.GetTransform();
		//DebugDrawPass(camera);

		PrintGPUTimers();
	}

	void Renderer::GBufferPass(const World& world, const FPSCamera& camera, const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances)
	{
		//StartGPUTimer(GPUTimerType::GeometryPass);
		glEnable(GL_MULTISAMPLE);

		if (meshInstances.size() == 0)
		{
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_MSGBufferIDs.m_GBuffer));
			GLCall(glClearColor(0, 0, 0, 1.0f));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // we're not using the stencil buffer now

			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferIDs.m_GBuffer));
			GLCall(glClearColor(0, 0, 0, 1.0f));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // we're not using the stencil buffer now
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			return;
		}
		

		GLCall(glViewport(0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_MSGBufferIDs.m_GBuffer));
		GLCall(glClearColor(0, 0, 0, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // we're not using the stencil buffer now
		GLCall(glEnable(GL_DEPTH_TEST));
	

		RenderScene(world, camera.GetTransform(), camera.GetProjection(), meshInstances, RenderPass::Geometry, nullptr, true);

		//GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		//GLCall(glDisable(GL_DEPTH_TEST));
		glDisable(GL_CULL_FACE);

		GLCall(glViewport(0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale));
		//GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE));
		//GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferIDs.m_GBuffer));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferIDs.m_GBuffer));
		GLCall(glClearColor(0, 0, 0, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // we're not using the stencil buffer now

		//GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferIDs.m_Albedos, 0));

		//const unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		//GLCall(glDrawBuffers(4, attachments));

		m_MSAAResolver->Bind();
	
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Albedos));
		m_MSAAResolver->SetUniform1i("m_Albedos", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		m_MSAAResolver->SetUniform1i("m_Depth", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_MetalRough));
		m_MSAAResolver->SetUniform1i("m_MetalRough", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		m_MSAAResolver->SetUniform1i("m_Normals", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Positions));
		m_MSAAResolver->SetUniform1i("m_Positions", 4);

		GLCall(glActiveTexture(GL_TEXTURE5));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Emission));
		m_MSAAResolver->SetUniform1i("m_Emissive", 5);
	
		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_MSAAResolver->Unbind();


	

		// BEGIN GDepth downsample
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSRFBO));
		GLCall(glViewport(0, 0, m_SSRPassSize.x, m_SSRPassSize.y));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_SSRHalfDepthMax, 0));

		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glClear(GL_DEPTH_BUFFER_BIT));
		m_DownsampleMaxShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth));
		m_DownsampleMaxShader->SetUniform1i("u_GDepth", 0);

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_DownsampleMaxShader->Unbind();
		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));

		//StopGPUTimer();
	}

	SSBO* m_AnimationDataUBO;
	std::unordered_map<const MeshWorldObject::Mesh*, unsigned int> objectPtr;
	unsigned int m_InstanceDataBuffer = 0;
	const Shader* m_CurrentShader = nullptr;
	const Material* m_CurrentMaterial = nullptr;

	void Renderer::RenderScene(const World& world, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances, RenderPass renderPass, const std::vector<PointLight>* pointlightsToTest, bool debug)
	{
		if (renderPass == RenderPass::Geometry)
		{
			StartGPUTimer(GPUTimerType::GeometryPass);
		}

		if (m_InstanceDataBuffer == 0)
		{
			//m_AnimationDataUBO->Write(0, sizeof(glm::mat4) * m_AnimationDataOffset, m_BatchAnimationDataBufferInter);
			m_AnimationDataUBO = new SSBO(2, 80000 * sizeof(glm::mat4), false);
			//Update Transform buffer for instances
			GLCall(glGenBuffers(1, &m_InstanceDataBuffer));
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_InstanceDataBuffer));
			GLCall(glBufferData(GL_ARRAY_BUFFER, 100000 * sizeof(IData), nullptr, GL_STREAM_DRAW)); // IMPORTANT: Size of PID (per instance data) should be concidered well.
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));												// Remember omnilights uses instances for rendering every 6 side. PID can grow real quick.
		}

		std::vector<const MeshWorldObject::Mesh*> meshesSortedByMaterial;
		{
			SCOPED_TIMER("Sorting by material");
			//meshesSortedByMaterial.resize(meshInstances.size());

			std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>::const_iterator it = meshInstances.begin();
			while (it != meshInstances.end())
			{
				const MeshWorldObject::Mesh* mesh = it->first;
				meshesSortedByMaterial.push_back(mesh);
				it = std::next(it);
			}
		
			std::sort(meshesSortedByMaterial.begin(), meshesSortedByMaterial.end(),
				[](const MeshWorldObject::Mesh* aa, const MeshWorldObject::Mesh* bb)
				{
					return aa->m_Material.GetShader() < bb->m_Material.GetShader();
				}
			);
		
		}

		m_CurrentShader = nullptr;
		m_CurrentMaterial = nullptr;

		for (const MeshWorldObject::Mesh* mesh : meshesSortedByMaterial)
		{
			const std::vector<MeshInstancess>& mInstances = meshInstances.find(mesh)->second;

			unsigned int vaoID = 0;
			unsigned int tbID = 0;

			std::unordered_map<const MeshWorldObject::Mesh*, unsigned int>::const_iterator it = objectPtr.find(mesh);
			if (it == objectPtr.end()) 
			{
				// IDs f�r das VAO und VBO
				unsigned int VBO;

				// VAO erstellen
				glGenVertexArrays(1, &vaoID);
				glBindVertexArray(vaoID);

				// VBO erstellen
				glGenBuffers(1, &VBO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);

				// Daten an das VBO binden
				glBufferData(GL_ARRAY_BUFFER, sizeof(MeshWorldObject::Mesh::Vertex) * mesh->m_Vertices.size(), (void*)&mesh->m_Vertices[0], GL_STATIC_DRAW);

				// Positionsattribut (vec3)
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Position));
				glEnableVertexAttribArray(0);
				GLCall(glVertexAttribDivisor(0, 0));

				// UV Attribut (vec2)
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_UV));
				glEnableVertexAttribArray(1);
				GLCall(glVertexAttribDivisor(1, 0));

				// Normalenattribut (vec3)
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Normal));
				glEnableVertexAttribArray(2);
				GLCall(glVertexAttribDivisor(2, 0));

				// Tangent Attribut (vec3)
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Tangent));
				glEnableVertexAttribArray(3);
				GLCall(glVertexAttribDivisor(3, 0));

				// Bitangent Attribut (vec3)
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Bitangent));
				glEnableVertexAttribArray(4);
				GLCall(glVertexAttribDivisor(4, 0));

				// Bone Index
				glVertexAttribIPointer(5, 4, GL_UNSIGNED_INT, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_BoneIndex));
				glEnableVertexAttribArray(5);
				GLCall(glVertexAttribDivisor(5, 0));

				// Bone weight (influence)
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::Vertex), (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_BoneWeight));
				glEnableVertexAttribArray(6);
				GLCall(glVertexAttribDivisor(6, 0));

				// Create vertex data <-> instance data buffer bindings
				GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_InstanceDataBuffer));
				GLCall(glEnableVertexAttribArray(7));
				GLCall(glEnableVertexAttribArray(8));
				GLCall(glEnableVertexAttribArray(9));
				GLCall(glEnableVertexAttribArray(10));
				GLCall(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)0));
				GLCall(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)16));
				GLCall(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)32));
				GLCall(glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)48));
				GLCall(glVertexAttribDivisor(7, 1));
				GLCall(glVertexAttribDivisor(8, 1));
				GLCall(glVertexAttribDivisor(9, 1));
				GLCall(glVertexAttribDivisor(10, 1));

				GLCall(glEnableVertexAttribArray(11));
				GLCall(glEnableVertexAttribArray(12));
				GLCall(glEnableVertexAttribArray(13));
				GLCall(glEnableVertexAttribArray(14));
				GLCall(glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)64));
				GLCall(glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)80));
				GLCall(glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)96));
				GLCall(glVertexAttribPointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)112));
				GLCall(glVertexAttribDivisor(11, 1));
				GLCall(glVertexAttribDivisor(12, 1));
				GLCall(glVertexAttribDivisor(13, 1));
				GLCall(glVertexAttribDivisor(14, 1));

				GLCall(glEnableVertexAttribArray(15));
				GLCall(glVertexAttribPointer(15, 3, GL_FLOAT, GL_FALSE, sizeof(IData), (const void*)128));
				GLCall(glVertexAttribDivisor(15, 1));


				unsigned int idxID = 0;
				GLCall(glGenBuffers(1, &idxID));
				GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID));
				GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* mesh->m_VertexIndicies.size(), (void*)&mesh->m_VertexIndicies[0], GL_STATIC_DRAW));


				objectPtr.emplace(mesh, vaoID);
			}
			else
			{
				vaoID = it->second;
				glBindVertexArray(vaoID);
			}

			std::unordered_map<const SkeletalMeshWorldObjectInstance*, unsigned int> animationDataOffsetMap;
		
			unsigned int animationDataOffset = 0;
			unsigned int ptrOffset = 0;
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_InstanceDataBuffer));
			for (const MeshInstancess& instance : mInstances)
			{
				SCOPED_TIMER("Instance Data Preparation");
				const MeshWorldObjectInstance* mwoi = instance.m_Instance;
				glm::mat4 instanceTransform;
				if (mwoi->GetRigidBody().IsValid() && renderPass == RenderPass::Debug)
				{
					instanceTransform = mwoi->GetRigidBody().GetWorldTransform();
				}
				else
				{
					instanceTransform = mwoi->GetTransform();
				}
				
				IData pd;
				pd.instance = instanceTransform;
				//pd.pid = { (float)instance.m_LightIndex, (float)instance.m_TargetLayerID };
				pd.mv = instance.m_MVP;
				
				// Uploaded animation matrices if skeletalmesh
				if (const SkeletalMeshWorldObjectInstance* smwoi = dynamic_cast<const SkeletalMeshWorldObjectInstance*>(instance.m_Instance))
				{
					auto [iter, inserted] = animationDataOffsetMap.emplace(smwoi, animationDataOffset);

					if (inserted)
					{
						const std::vector<glm::mat4>& animationBoneData = smwoi->GetAnimationBoneData();
						if (animationBoneData.size() > 0)
						{
							pd.pid.m_AnimationDataOffset = animationDataOffset;

							m_AnimationDataUBO->Write(animationDataOffset * sizeof(glm::mat4), sizeof(glm::mat4) * animationBoneData.size(), (void*)&animationBoneData[0]);
							animationDataOffset += animationBoneData.size();
						}
					}
					else
					{
						pd.pid.m_AnimationDataOffset = iter->second;
					}
				}
				

				GLCall(glBufferSubData(GL_ARRAY_BUFFER, ptrOffset, sizeof(IData), (void*)&pd));
				ptrOffset += sizeof(IData);
			}

			GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

			const Material* material = &mesh->m_Material;


			if (renderPass == RenderPass::Geometry && mesh->m_Material.GetShader() != m_CurrentShader)
			{
				if (material != nullptr)
					m_CurrentShader->Unbind();

				// Bind shader
				const FPSCamera& camera = *m_Camera;
				const glm::mat4 camTransform = camera.GetTransform();
				material->m_Shader->Bind();
				material->GetShader()->SetUniformMat4f("u_Projection", &camera.GetProjection());
				material->GetShader()->SetUniformMat4f("u_View", &camTransform);
				material->GetShader()->SetUniform1f("u_ClipNear", camera.GetNearClip());
				material->GetShader()->SetUniform1f("u_ClipFar", camera.GetFarClip());
				material->GetShader()->SetUniform1f("u_GameTime", GMTime::s_Time);
				m_CurrentShader = material->GetShader();
				NAMED_COUNTER("Shader Switches");
			}

			if (renderPass == RenderPass::Geometry && material != m_CurrentMaterial)
			{
				material->Bind();
				m_CurrentMaterial = material;
				NAMED_COUNTER("Material Switches");
			}

			//Render
			glDrawElementsInstanced(GL_TRIANGLES, mesh->m_VertexIndicies.size(), GL_UNSIGNED_INT, 0, mInstances.size());

			glBindVertexArray(0);
		}

		if (renderPass == RenderPass::Geometry)
		{
			StopGPUTimer();
		}
	}

	/*
	void Renderer::RenderScene(const World& world, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances, RenderPass renderPass, const std::vector<PointLight>* pointlightsToTest, bool debug)
	{
		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>::const_iterator iterator = meshInstances.begin();
		int i = 0;
		if (debug)
		{
			StartGPUTimer(GPUTimerType::GeometryPass);
		}
		while (iterator != meshInstances.end())
		{
			NAMED_COUNTER("DrawCalls");
			const std::vector<MeshInstancess>& mInstances = iterator->second;
			const MeshWorldObject::Mesh* mesh = iterator->first;

			//const FPSCamera::Frustum& camFrustum = m_Camera->GetFrustum();

			AddToBatchResult result = m_RenderBatch.TryAddToBatch(*mesh, mInstances, projectionMatrix, viewMatrix, renderPass, pointlightsToTest);
			switch (result)
			{
			case AddToBatchResult::TooBigForBatch:
				ASSERT("Object too big for batching");
				// TODO invoke separte (batchless) drawcall, currently these objects are ignored
				iterator = std::next(iterator);
				continue;
			case AddToBatchResult::NotFitIntoBatch:
			case AddToBatchResult::MaterialChanged:
				m_RenderBatch.Flush(renderPass);
				++i;
				continue;
			}

			iterator = std::next(iterator);
		}
		m_RenderBatch.Flush(renderPass);
		if (debug)
		{
			StopGPUTimer();
		}
	}
	*/

	void Renderer::BlurTexture(unsigned int anInputTextureID, unsigned int anOutputTextureID, unsigned int m_ImmediateTextureCacheID, glm::vec2 textureSize, unsigned int numIterations, bool isSSRBlur)
	{
		// Blur
		GLCall(glViewport(0, 0, textureSize.x, textureSize.y));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO));
		GLCall(glDisable(GL_DEPTH_TEST));

		Shader* shader;
		if (isSSRBlur)
			shader = nullptr;
		else
			shader = m_BlurShader;

		shader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth));
		shader->SetUniform1i("u_Depth", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_ImmediateTextureCacheID));
		shader->SetUniform1i("u_InputTexture", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture));
		shader->SetUniform1i("u_ComplexFragments", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		shader->SetUniform1i("u_MSGDepth", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		shader->SetUniform1i("u_MSGNormal", 4);

		if (isSSRBlur)
		{
			GLCall(glActiveTexture(GL_TEXTURE5));
			GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Normals));
			shader->SetUniform1i("u_gNormal", 5);

			GLCall(glActiveTexture(GL_TEXTURE6));
			GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_MetalRough));
			shader->SetUniform1i("u_gMetalRoughness", 6);

			/*
			GLCall(glActiveTexture(GL_TEXTURE7));
			GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureMips));
			shader->SetUniform1i("u_SSRMips", 7);

			GLCall(glCopyImageSubData(anInputTextureID, GL_TEXTURE_2D, 0, 0, 0, 0, m_SSRTextureMips, GL_TEXTURE_2D, 0, 0, 0, 0, textureSize.x, textureSize.y, 1));
			GLCall(glGenerateMipmap(GL_TEXTURE_2D));
			*/
			return;
		}

		shader->SetUniform2i("u_RenderResolution", textureSize.x, textureSize.x);
		shader->SetUniform2i("u_ViewportResolution", m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale);

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, anOutputTextureID, 0));
		const unsigned int attachments4[1] = { GL_COLOR_ATTACHMENT0 };
		GLCall(glDrawBuffers(1, attachments4));

		GLCall(glClearColor(0, 0, 0, 0));
		GLCall(glClearDepth(1));

		GLCall(glBindVertexArray(m_ViewportQuadVAO));

		GLCall(glCopyImageSubData(anInputTextureID, GL_TEXTURE_2D, 0, 0, 0, 0, m_ImmediateTextureCacheID, GL_TEXTURE_2D, 0, 0, 0, 0, textureSize.x, textureSize.y, 1));

		const FPSCamera& camera = *m_Camera;
		shader->SetUniform1f("u_ClipNear", camera.GetNearClip());
		shader->SetUniform1f("u_ClipFar", camera.GetFarClip());
		shader->SetUniform1f("u_IsSSRBlur", isSSRBlur ? 1.f : 0.f);

		for (unsigned int i = 0; i < numIterations; ++i)
		{
			for (int hori = 0; hori < 2; ++hori)
			{
				shader->SetUniform1i("u_Horizontal", hori);
				GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
				GLCall(glCopyImageSubData(anOutputTextureID, GL_TEXTURE_2D, 0, 0, 0, 0, m_ImmediateTextureCacheID, GL_TEXTURE_2D, 0, 0, 0, 0, textureSize.x, textureSize.y, 1));
			}
		}

		GLCall(glBindVertexArray(0));

		shader->Unbind();
	}

	void Renderer::LightVolumetricPass(const World& world, const FPSCamera& camera, const std::vector<PointlightWorldObjectInstance*>& pointlightWorldObjects)
	{
		StartGPUTimer(GPUTimerType::VolumetricsPass);

		GLCall(glViewport(0, 0, m_VolumetricLightMapSize.x, m_VolumetricLightMapSize.y));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_VolumetricLightFBO));
		GLCall(glClearColor(1, 1, 1, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // we're not using the stencil buffer now
		GLCall(glDisable(GL_DEPTH_TEST));

		m_VolumetricLightShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Positions));
		m_VolumetricLightShader->SetUniform1i("u_GPositions", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth));
		m_VolumetricLightShader->SetUniform1i("u_GDepths", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_DepthCubemapTexture));
		m_VolumetricLightShader->SetUniform1i("u_DepthCubemapTexture", 2);

		// Pass render resolution to shader
		m_VolumetricLightShader->SetUniform2i("u_VolumetricRenderResolution", m_VolumetricLightMapSize.x, m_VolumetricLightMapSize.y);

		m_VolumetricLightShader->BindUBOBlock(*m_PointLightSortedToCamDistanceUBO, "PointLightDataBlock");
		m_VolumetricLightShader->SetUniform1i("u_PointlightCount", pointlightWorldObjects.size()); // limit to number of shadow maps

		const glm::vec3 camPosition = camera.GetPosition();
		m_VolumetricLightShader->SetUniform3f("u_ViewPos", camPosition.x, camPosition.y, camPosition.z);

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_VolumetricLightShader->Unbind();

		BlurTexture(m_VolumetricLightTexture, m_BlurredVolumetricTexture, m_BlurTextureCache, m_VolumetricLightMapSize, 2);
		StopGPUTimer();
	}

	void Renderer::SSAOPass()
	{
		SCOPED_TIMER("SSAO pass");

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO));

		GLCall(glViewport(0, 0, m_SSAOTextureResolution.x, m_SSAOTextureResolution.y));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GLCall(glDisable(GL_DEPTH_TEST));

		m_SSAOShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAOPositionTexture));
		m_SSAOShader->SetUniform1i("gPosition", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAONormalTexture));
		m_SSAOShader->SetUniform1i("gNormal", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTexture));
		m_SSAOShader->SetUniform1i("texNoise", 2);

		glm::mat4 proj = m_Camera->GetProjection();
		glm::mat4 projView = m_Camera->GetProjection() * m_Camera->GetTransform();

		m_SSAOShader->SetUniformMat4f("projection", &proj);
		m_SSAOShader->SetUniformMat4f("projectionView", &projView);
		m_SSAOShader->SetUniform3fv("samples", &mySSAOKernel[0].x, mySSAOKernel.size());
		m_SSAOShader->SetUniform3f("cameraPosition", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z);
		m_SSAOShader->SetUniform3f("cameraDirection", m_Camera->GetFrontVector().x, m_Camera->GetFrontVector().y, m_Camera->GetFrontVector().z);

		glBindVertexArray(m_ViewportQuadVAO);
	
		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_SSAOShader->Unbind();


		// Blur
		//BlurTexture(m_SSAOTexture, m_SSAOTextureBlurred, m_VolumetricLightMapSize);
	}

	void Renderer::LightingPass(const World& world, const FPSCamera& camera, const std::vector<PointlightWorldObjectInstance*>& pointlightWorldObjects)
	{
		StartGPUTimer(GPUTimerType::LightingPass);

		GLCall(glViewport(0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_LightingFBO));
		GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
		m_LightingShader->Bind();

		// Bind Gbuffer
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		m_LightingShader->SetUniform1i("u_GNormals", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Positions));
		m_LightingShader->SetUniform1i("u_GPositions", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		m_LightingShader->SetUniform1i("u_GDepths", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Albedos));
		m_LightingShader->SetUniform1i("u_GAlbedo", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_MetalRough));
		m_LightingShader->SetUniform1i("u_GMetalRough", 4);
	
		GLCall(glActiveTexture(GL_TEXTURE5));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_PositionCubemapTexture));
		m_LightingShader->SetUniform1i("u_ColorCube", 5);

		GLCall(glActiveTexture(GL_TEXTURE6));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture));
		m_LightingShader->SetUniform1i("u_ComplexFragment", 6);

		GLCall(glActiveTexture(GL_TEXTURE7));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Emission));
		m_LightingShader->SetUniform1i("u_GEmission", 7);

		// Bind UBOs
		m_LightingShader->BindUBOBlock(*m_PointLightSortedToCamDistanceUBO, "PointLightDataBlock");
		m_LightingShader->SetUniform1i("u_PointlightCount", pointlightWorldObjects.size());

		// Write light infos to shader
		GLCall(glActiveTexture(GL_TEXTURE8));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_DepthCubemapTexture));
		m_LightingShader->SetUniform1i("u_DepthCubemapTexture", 8);

		const glm::vec3 camPosition = camera.GetPosition();
		m_LightingShader->SetUniform3f("u_ViewPos", camPosition.x, camPosition.y, camPosition.z);

		// Pass render resolution to shader
		m_LightingShader->SetUniform2i("u_RenderResolution", camera.GetScreenSize().x * m_RenderScale, camera.GetScreenSize().y * m_RenderScale);
		m_LightingShader->SetUniform2i("u_ViewportResolution", camera.GetScreenSize().x, camera.GetScreenSize().y);

		m_LightingShader->SetUniform1f("u_ClipNear", camera.GetNearClip());
		m_LightingShader->SetUniform1f("u_ClipFar", camera.GetFarClip());

		// Draw to screen canvas
		glBindVertexArray(m_ViewportQuadVAO);

		GLCall(glDisable(GL_DEPTH_TEST));

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_LightingShader->Unbind();

		StopGPUTimer();
	}


	void Renderer::SSRPass()
	{
		StartGPUTimer(GPUTimerType::SSRPass);

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSRFBO));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSRTexture, 0));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_SSRTextureNormals, 0));
	
		const unsigned int atts[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GLCall(glDrawBuffers(2, atts));

		GLCall(glViewport(0, 0, m_SSRPassSize.x, m_SSRPassSize.y));
		GLCall(glClearColor(0, 0, 0, 0));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
		GLCall(glDisable(GL_DEPTH_TEST));

		m_SSRShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Positions));
		m_SSRShader->SetUniform1i("gPosition", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		m_SSRShader->SetUniform1i("depthMapMS", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_LightingTexture));
		m_SSRShader->SetUniform1i("colorBuffer", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_MetalRough));
		m_SSRShader->SetUniform1i("u_MetalRough", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		m_SSRShader->SetUniform1i("gNormalMS", 4);
	
		m_SSRShader->SetUniform1f("u_GameTime", GMTime::s_Time);

		const FPSCamera& camera = *m_Camera;

		m_SSRShader->SetUniformMat4f("projection", &camera.GetProjection());

		glm::mat4 invProj = glm::inverse(camera.GetProjection());
		m_SSRShader->SetUniformMat4f("invProjection", &invProj);
		m_SSRShader->SetUniform3f("u_CameraWorldPosition", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		const glm::mat4 invview = glm::inverse(camera.GetTransform());
		m_SSRShader->SetUniformMat4f("view", camera.GetTransform());
		m_SSRShader->SetUniformMat4f("invView", &invview);

		m_SSRShader->SetUniform2i("u_RenderResolution", m_SSRPassSize.x, m_SSRPassSize.y);

		GLCall(glDisable(GL_DEPTH_TEST));

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_SSRShader->Unbind();
		StopGPUTimer();
	
		//return;
		StartGPUTimer(GPUTimerType::SSRBlur);
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_SSRFBO));
		//GLCall(glCopyImageSubData(m_SSRTexture, GL_TEXTURE_2D, 0, 0, 0, 0, m_BlurSSRTexture, GL_TEXTURE_2D, 0, 0, 0, 0, m_SSRPassSize.x, m_SSRPassSize.y, 1));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSRTextureBlurred, 0));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0));

		GLCall(glViewport(0, 0, camera.GetScreenSize().x * m_RenderScale, camera.GetScreenSize().y* m_RenderScale));
		glClear(GL_COLOR_BUFFER_BIT);
		m_BlurSSRShader->Bind();
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Normals));
		m_BlurSSRShader->SetUniform1i("u_GNormals", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture));
		m_BlurSSRShader->SetUniform1i("u_ComplexFragments", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_BlurSSRTexture));
		m_BlurSSRShader->SetUniform1i("u_SSRTexture", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		m_BlurSSRShader->SetUniform1i("u_MSGNormals", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_MetalRough));
		m_BlurSSRShader->SetUniform1i("u_MSGMetalRough", 4);

		GLCall(glActiveTexture(GL_TEXTURE5));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTexture));
		m_BlurSSRShader->SetUniform1i("u_HalfResSSRTexture", 5);

		m_BlurSSRShader->SetUniform2i("u_ViewportResolution", m_ScreenSize.x* m_RenderScale, m_ScreenSize.y* m_RenderScale);

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		glm::vec2 horizontalBlurDirection(1,0);
		glm::vec2 verticalBlurDirection(0,1);

		m_BlurSSRShader->SetUniform1i("u_Iteration", -1);
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glCopyImageSubData(m_SSRTextureBlurred, GL_TEXTURE_2D, 0, 0, 0, 0, m_BlurSSRTexture, GL_TEXTURE_2D, 0, 0, 0, 0, m_ScreenSize.x* m_RenderScale, m_ScreenSize.y* m_RenderScale, 1));
		for (int i = 0; i < 2; ++i)
		{
			const float isHorizontal = (i % 2) == 0 ? 1.f : 0.f;
			m_BlurSSRShader->SetUniform2f("u_BlurDirection", isHorizontal ? horizontalBlurDirection : verticalBlurDirection);
			m_BlurSSRShader->SetUniform1i("u_Iteration", i);
			GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
			GLCall(glCopyImageSubData(m_SSRTextureBlurred, GL_TEXTURE_2D, 0, 0, 0, 0, m_BlurSSRTexture, GL_TEXTURE_2D, 0, 0, 0, 0, m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale, 1));
		}
		GLCall(glBindVertexArray(0));
		m_BlurSSRShader->Unbind();
		StopGPUTimer();
	}

	void Renderer::PBBPass()
	{
		StartGPUTimer(GPUTimerType::PBBPass);
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_PBBFBO));
		GLCall(glCopyImageSubData(m_LightingTexture, GL_TEXTURE_2D, 0, 0, 0, 0, m_PBBTextureMips, GL_TEXTURE_2D, 0, 0, 0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale, 1));

		m_PBBDownsampleShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PBBTextureMips));
		m_PBBDownsampleShader->SetUniform1i("srcTexture", 0);

		GLCall(glDisable(GL_DEPTH_TEST));

		glm::ivec2 mipResolution(m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale);
		glm::ivec2 previousMipSize = mipResolution;
		std::vector<glm::ivec2> mipResolutions;
		mipResolutions.push_back(mipResolution);
		mipResolution /= 2;

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		for (int i = 1; i < m_NumPBBMips; ++i)
		{
			GLCall(glViewport(0, 0, mipResolution.x, mipResolution.y));
			m_PBBDownsampleShader->SetUniform1i("srcTextureMipLevel", i - 1);
			m_PBBDownsampleShader->SetUniform2i("srcResolution", mipResolution.x, mipResolution.y);

			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PBBTextureMips, i));

			GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));

			mipResolutions.push_back(mipResolution);
			mipResolution /= 2;
		}
		m_PBBDownsampleShader->Unbind();

		m_PBBUpsampleShader->Bind();
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PBBTextureMips));
		m_PBBUpsampleShader->SetUniform1i("srcTexture", 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		for (int i = m_NumPBBMips - 1; i > 0; --i)
		{
			mipResolution = mipResolutions[i-1];
			GLCall(glViewport(0, 0, mipResolution.x, mipResolution.y));
			m_PBBUpsampleShader->SetUniform1i("srcTextureMipLevel", i);
			m_PBBUpsampleShader->SetUniform2i("srcResolution", mipResolution.x, mipResolution.y);

			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PBBTextureMips, i - 1));

			GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));

			previousMipSize = mipResolution;
			mipResolution *= 2;
		}
		GLCall(glBindVertexArray(0));

		glDisable(GL_BLEND);

		m_PBBUpsampleShader->Unbind();
		StopGPUTimer();
	}


	void Renderer::CompositePass(const FPSCamera& camera)
	{
		StartGPUTimer(GPUTimerType::CompositePass);

		GLCall(glViewport(0, 0, m_ViewportWidth, m_ViewportHeight));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		m_ViewportShader->Bind();

		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_LightingTexture));
		m_ViewportShader->SetUniform1i("m_LightingPass", 0);

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureBlurred));
		m_ViewportShader->SetUniform1i("m_SSRPass", 1);

		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_BlurredVolumetricTexture));
		m_ViewportShader->SetUniform1i("u_VolumetricLight", 2);

		GLCall(glActiveTexture(GL_TEXTURE3));
		//GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Depth));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_HZTexture));
		m_ViewportShader->SetUniform1i("u_GDepth", 3);

		GLCall(glActiveTexture(GL_TEXTURE4));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Albedos));
		m_ViewportShader->SetUniform1i("u_GAlbedo", 4);

		GLCall(glActiveTexture(GL_TEXTURE5));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ComplexFragmentTexture));
		m_ViewportShader->SetUniform1i("u_ComplexFragments", 5);

		GLCall(glActiveTexture(GL_TEXTURE6));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Depth));
		m_ViewportShader->SetUniform1i("u_MSGDepth", 6);

		GLCall(glActiveTexture(GL_TEXTURE7));
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSGBufferIDs.m_Normals));
		m_ViewportShader->SetUniform1i("u_MSNormal", 7);

		GLCall(glActiveTexture(GL_TEXTURE8));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Emission));
		m_ViewportShader->SetUniform1i("u_GEmission", 8);

		GLCall(glActiveTexture(GL_TEXTURE9));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PBBTextureMips));
		m_ViewportShader->SetUniform1i("u_PBBTexture", 9);

		GLCall(glActiveTexture(GL_TEXTURE10));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_Normals));
		m_ViewportShader->SetUniform1i("m_GNormals", 10);

		GLCall(glActiveTexture(GL_TEXTURE11));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_GBufferIDs.m_MetalRough));
		m_ViewportShader->SetUniform1i("m_GMetalRough", 11);

		GLCall(glActiveTexture(GL_TEXTURE12));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRHalfDepthMax));
		m_ViewportShader->SetUniform1i("u_GHalfDepthMax", 12);

		GLCall(glActiveTexture(GL_TEXTURE13));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_SSRTextureNormals));
		m_ViewportShader->SetUniform1i("u_SSRNormals", 13);

		GLCall(glActiveTexture(GL_TEXTURE14));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_OcclusionQueryDebugTexture));
		m_ViewportShader->SetUniform1i("u_OcclusionQueryDebugTexture", 14);

		m_ViewportShader->SetUniform2i("u_RenderResolution", m_ScreenSize.x * m_RenderScale, m_ScreenSize.y * m_RenderScale);
		m_ViewportShader->SetUniform2i("u_ViewportResolution", m_ScreenSize.x, m_ScreenSize.y);

		//colorText->Bind(7);
		m_ViewportShader->SetUniform1f("u_GameTime", GMTime::s_Time);

		GLCall(glDisable(GL_DEPTH_TEST));

		GLCall(glBindVertexArray(m_ViewportQuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
		GLCall(glBindVertexArray(0));

		m_ViewportShader->Unbind();

		StopGPUTimer();
	}

	void Renderer::DebugDrawPass(const FPSCamera& camera)
	{
		SCOPED_TIMER("Debug pass");

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>> instances;



		for (auto& dMesh : m_DebugMeshes)
		{
			std::vector<MeshWorldObjectInstance*> mwois;

			MeshWorldObject* mwo = new MeshWorldObject("DebugMesh");
			mwo->m_Meshes.push_back(&dMesh);

			MeshWorldObjectInstance* mwoi = new MeshWorldObjectInstance(mwo);
			mwois.push_back(mwoi);

			const MeshWorldObject::Mesh* cm = &dMesh;

			std::vector<MeshInstancess> insts;
			MeshInstancess inst;
			inst.m_Instance = mwoi;
			//inst.m_MVP = camera.GetProjection() * camera.GetTransform() * glm::mat4();
			inst.m_MVP = glm::mat4(1.0f);

			insts.push_back(inst);

			instances[cm] = insts;
		}
	
	
		GLCall(glViewport(0, 0, m_ViewportWidth, m_ViewportHeight));

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBufferIDs.m_GBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_ViewportWidth * m_RenderScale, m_ViewportHeight * m_RenderScale, 0, 0, m_ViewportWidth, m_ViewportHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//GLCall(glEnable(GL_DEPTH_TEST));


		const std::vector<PointLight> pointlightsToTest;

		m_DebugDrawMeshShader->Bind();
		const glm::mat4 viewProjection = camera.GetProjection() * camera.GetTransform();
		m_DebugDrawMeshShader->SetUniformMat4f("u_VP", &viewProjection);
		RenderScene(*m_World, camera.GetTransform(), camera.GetProjection(), instances, RenderPass::Debug, &pointlightsToTest);
		m_DebugDrawMeshShader->Unbind();


	
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	}