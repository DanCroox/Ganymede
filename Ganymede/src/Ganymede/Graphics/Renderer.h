#pragma once
#include "Ganymede/System/Types.h"

#include "glm/glm.hpp"
#include "WorldPartition.h"
#include <vector> 
#include <sstream> 
#include <array> 
#include "RenderBatch.h"
#include "RendererTypes.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/System/Thread.h"

class World;
class FPSCamera;
class Shader;
class ShaderManager;
class Texture;
class PointlightWorldObjectInstance;
class WorldPartitionManager;


class Renderer
{
public:
    glm::mat4 m_CameraTransformLastFrame;

    struct DebugDrawMesh
    {
        std::vector<glm::vec3> m_VertexPositions;
        std::vector<unsigned int> m_VertexIndices;
        glm::mat4 m_WorldTransform;
    };

    void AddDebugDrawMesh(const MeshWorldObject::Mesh& drawMesh) { m_DebugMeshes.push_back(drawMesh); }
    void RemoveDebugDrawMesh(const MeshWorldObject::Mesh& drawMesh)
    {
        /*
        for(int i = m_DebugMeshes.size() - 1; i >= 0; --i)
        {
            if (m_DebugMeshes[i] == drawMesh)
            {
                m_DebugMeshes.erase(m_DebugMeshes.begin() + i);
            }
        }
        */
    }

    Renderer() = delete;
    Renderer(ShaderManager& shaderManager);
    ~Renderer();

    void Draw(const World& world, WorldPartitionManager& worldPartitionManager, const FPSCamera& camera);

    Shader* GetDefaultMeshShader() { return m_GBufferShader; }

    bool m_DebugDisableOcclusionCulling = false;
private:

    void RenderScene(const World& world, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances, RenderPass renderPass, const std::vector<PointLight>* pointlightsToTest = nullptr, bool debug = false);
    
    void GetSortedPointlightsByDistanceToCamera(std::vector<PointlightWorldObjectInstance*>& pointlights) const;
    void RenderCubeShadowMaps(const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances, const std::vector<PointLight>& pointlightsForOcclusionTest);

    void UpdatePointlightShadowMaps(const std::vector<PointlightWorldObjectInstance*>& pointlightsClosestToCamera, std::vector<PointLight>& pointlightsForOcclusionTest);
    void CollectGeometryDataForRendering(const std::vector<PointLight>& pointlightsForOcclusionTest, std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& pointlightRenderData, std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& cameraRenderData);

    void CreateFrameBuffersIfRequested();
    void GBufferPass(const World& world, const FPSCamera& camera, const std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>& meshInstances);
    void LightVolumetricPass(const World& world, const FPSCamera& camera, const std::vector<PointlightWorldObjectInstance*>& pointlightWorldObjects);
    void SSAOPass();
    void LightingPass(const World& world, const FPSCamera& camera, const std::vector<PointlightWorldObjectInstance*>& pointlightWorldObjects);
    void SSRPass();
    void PBBPass();
    void CompositePass(const FPSCamera& camera);
    void DebugDrawPass(const FPSCamera& camera);

    void BlurTexture(unsigned int anInputTextureID, unsigned int anOutputTextureID, unsigned int m_ImmediateTextureCacheID, glm::vec2 textureSize, unsigned int numIterations, bool isSSRBlur = false);
    
    std::vector<const MeshWorldObjectInstance*> m_OcclusionCulledInstances;
    
    const Shader* m_ViewportShader;
    unsigned int m_ViewportQuadVAO = 0;
    unsigned int m_ViewportQuadVertexBuffer = 0;
    unsigned int m_ViewportQuadUVsBuffer = 0;
    unsigned int m_ViewportQuadIndexBuffer = 0;

    unsigned int m_FrameBuffer = 0;
    unsigned int m_FrameTexture = 0;
    unsigned int m_DepthTexture = 0;

    SSBO* m_PointLightSortedToCamDistanceUBO;
    SSBO* m_PointLightSortedToCamDistanceOcclusionCheckUBO;

    unsigned int m_DepthCubemapFBO = 0;
    unsigned int m_DepthCubemapTexture = 0;
    unsigned int m_PositionCubemapTexture = 0;
    float m_PointLightNearClip = 0.01f;
    float m_PointLightFarClip = 1000.f;
    glm::mat4 m_PointLightProjectionMatrix;

    unsigned int m_OcclusionQueryFBO = 0;
    unsigned int m_OcclusionQueryDebugTexture = 0;

    unsigned int m_VolumetricLightFBO = 0;
    unsigned int m_VolumetricLightTexture = 0;

    unsigned int m_BlurFBO = 0;
    unsigned int m_BlurTextureCache = 0;
    unsigned int m_BlurredVolumetricTexture = 0;
    unsigned int m_BlurSSRTexture = 0;

    unsigned int m_SSAOFBO = 0;
    unsigned int m_SSAOTexture = 0;
    unsigned int m_SSAOTextureBlurred = 0;
    unsigned int m_SSAONoiseTexture = 0;
    unsigned int m_SSAOPositionTexture = 0;
    unsigned int m_SSAONormalTexture = 0;

    unsigned int m_SSRFBO = 0;
    unsigned int m_SSRTexture = 0;
    unsigned int m_SSRTextureNormals = 0;
    unsigned int m_SSRTextureMips = 0;
    unsigned int m_SSRTextureBlurred = 0;
    unsigned int m_SSRHalfDepthMax = 0;

    unsigned int m_PBBFBO = 0;
    unsigned int m_PBBTextureMips = 0;
    unsigned int m_NumPBBMips = 8;

    unsigned int m_NumGNormalMips = 8;
    unsigned int m_GNormalMips = 0;

    unsigned int m_ComplexFragmentTexture = 0;

    std::vector<glm::vec3> mySSAOKernel;

    LightsManager m_LightsManager;
    WorldPartitionManager* m_WorldPartitionManager;
    const World* m_World;
    const FPSCamera* m_Camera;
    glm::u32vec2 m_ScreenSize = glm::u32vec2(1920, 1080);
    float m_RenderScale = 1;

    glm::ivec2 m_SSAOTextureResolution = glm::ivec2(1920 * m_RenderScale, 1080 * m_RenderScale);

    unsigned int m_LightingFBO = 0;
    unsigned int m_LightingTexture = 0;

    bool m_CreateFrameBuffer = true;

    unsigned int m_ViewportWidth=0;
    unsigned int m_ViewportHeight=0;

    Shader* m_GBufferShader;
    Shader* m_OmnidirectionalShadowShader;
    Shader* m_OmnidirectionalShadowInstancesShader;
    Shader* m_VolumetricLightShader;
    Shader* m_BlurShader;
    Shader* m_BlurSSRShader;
    Shader* m_LightingShader;
    Shader* m_SSAOShader;
    Shader* m_SSRShader;
    Shader* m_DebugDrawMeshShader;
    Shader* m_OcclusionQueryShaderOmni;
    Shader* m_MSAAResolver;
    Shader* m_DownsampleMaxShader;
    Shader* m_PBBDownsampleShader;
    Shader* m_PBBUpsampleShader;

    glm::vec2 m_SSRPassSize;

    const float m_ShadowMapSize = 512;
    const glm::ivec2 m_VolumetricLightMapSize = glm::ivec2(512, 512);

    //RenderBatch m_RenderBatch;

    struct GBufferIDs
    {
        unsigned int m_GBuffer;

        unsigned int m_Normals;
        unsigned int m_Positions;
        unsigned int m_Depth;
        unsigned int m_Albedos;
        unsigned int m_MetalRough;
        unsigned int m_Emission;
    };

    GBufferIDs m_GBufferIDs;
    GBufferIDs m_MSGBufferIDs;

    unsigned int m_HZFramebuffer;
    unsigned int m_HZTexture;

    std::vector<MeshWorldObject::Mesh> m_DebugMeshes;

    // Gpu timers
#define RegisterGPUTimers(...)\
       enum class GPUTimerType {\
        __VA_ARGS__, count\
    };\
    struct GPUTimerNames {\
        static std::string ToString(GPUTimerType timer) {\
            const std::string allNames = #__VA_ARGS__ ;\
            std::stringstream ss(allNames);\
            std::vector<std::string> result;\
            while (ss.good()) {\
                std::string substr;\
                std::getline(ss, substr, ',');\
                result.push_back(substr);\
            }\
            return result[(int)timer]; \
        }\
    };

    RegisterGPUTimers(GeometryPass, PointlightShadowMapPass, LightingPass, VolumetricsPass, CompositePass, SSRPass, SSRBlur, PBBPass, OcclusionCullingPass);
   
    struct GPUTimer {
        GPUTimerType m_Timer;
        std::string m_Name;
        unsigned int m_QueryID;
    };

    std::vector<GPUTimer> m_GPUTimers;
    std::vector<Thread> m_RendererThreadPool;

    struct RenderData
    {
        std::vector<std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>> m_CameraRenderDataBuffer;
        std::vector<std::unordered_map<const MeshWorldObject::Mesh*, std::vector<MeshInstancess>>> m_PointlightsRenderDataBuffer;

        void Initialize(unsigned int size)
        {
            m_PointlightsRenderDataBuffer.resize(size);
            m_CameraRenderDataBuffer.resize(size);
        }
    };

    RenderData m_RenderDataBuffer;

// #define  USE_GPU_TIMER //REWORK: Implment gpu timer in cpp file. otherwise wonot be possible due to glew being forward declared
    void InitGPUTimers()
    {
#ifdef USE_GPU_TIMER
        for (int timerIndex = 0; timerIndex < (int)GPUTimerType::count; ++timerIndex)
        {
            GPUTimer timer;
            timer.m_Timer = (GPUTimerType)timerIndex;
            timer.m_Name =  GPUTimerNames::ToString(timer.m_Timer);
            GLCall(glGenQueries(1, &timer.m_QueryID));
            m_GPUTimers.push_back(timer);
        }
#endif // USE_GPU_TIMER
    }

    void PrintGPUTimers()
    {
#ifdef USE_GPU_TIMER
        for (const GPUTimer& timer : m_GPUTimers)
        {
            GLuint64 result;
            glGetQueryObjectui64v(timer.m_QueryID, GL_QUERY_RESULT, &result);
            result /= (1000);
            NUMBERED_NAMED_COUNTER(timer.m_Name.c_str(), result);
        }
#endif // USE_GPU_TIMER
    }

    void DeleteGPUTimers()
    {
#ifdef USE_GPU_TIMER
        for (const GPUTimer& timer : m_GPUTimers)
        {
            GLCall(glDeleteQueries(1, &timer.m_QueryID));
        }
#endif // USE_GPU_TIMER
    }

    void StartGPUTimer(GPUTimerType timer) const
    {
#ifdef USE_GPU_TIMER
        GLCall(glBeginQuery(GL_TIME_ELAPSED, m_GPUTimers[(int)timer].m_QueryID));
#endif // USE_GPU_TIMER
    }
    void StopGPUTimer() const
    {
#ifdef USE_GPU_TIMER
        GLCall(glEndQuery(GL_TIME_ELAPSED));
#endif // USE_GPU_TIMER
    }
};