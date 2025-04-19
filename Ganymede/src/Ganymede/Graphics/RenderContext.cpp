#include "RenderContext.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Player/FPSCamera.h"

namespace Ganymede
{
    RenderContext::RenderContext(const World& world, const FPSCamera& camera) :
        m_World(world),
        m_Camera(camera)
    {}

    const World& RenderContext::GetWorld() const
    {
        return m_World;
    }

    const FPSCamera& RenderContext::GetCamera() const
    {
        return m_Camera;
    }

    FrameBuffer* RenderContext::CreateFrameBuffer(const std::string& name, glm::u32vec2 renderDimension, bool isHardwareBuffer)
    {
        auto [it, inserted] = m_FrameBuffers.try_emplace(name, renderDimension, isHardwareBuffer);
        GM_CORE_ASSERT(inserted, "Tried to create framebuffer which already existed. Using from cache.");
        FrameBuffer* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_FrameBuffers.erase(name);
            GM_CORE_ASSERT(false, "Framebuffer is invalid.");
            return nullptr;
        }
        return ptr;
    }

    SinglesampleRenderTarget* RenderContext::CreateSingleSampleRenderTarget(const std::string& name, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
    {
        auto [it, inserted] = m_SingleSampleRenderTargets.try_emplace(name, componentType, dataType, precision, size);
        GM_CORE_ASSERT(inserted, "Tried to create single sample render target which already existed. Using from cache.");
        SinglesampleRenderTarget* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_SingleSampleRenderTargets.erase(name);
            GM_CORE_ASSERT(false, "Single sample render target is invalid.");
            return nullptr;
        }
        return ptr;
    }

    MultisampleRenderTarget* RenderContext::CreateMultiSampleRenderTarget(const std::string& name, unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
    {
        auto [it, inserted] = m_MultiSampleRenderTargets.try_emplace(name, sampleCount, componentType, dataType, precision, size);
        GM_CORE_ASSERT(inserted, "Tried to create multi sample render target which already existed. Using from cache.");
        MultisampleRenderTarget* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_MultiSampleRenderTargets.erase(name);
            GM_CORE_ASSERT(false, "Multi sample render target is invalid.");
            return nullptr;
        }
        return ptr;
    }

    CubeMapArrayRenderTarget* RenderContext::CreateCubeMapArrayRenderTarget(const std::string& name, unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
    {
        auto [it, inserted] = m_CubeMapArrayRenderTargets.try_emplace(name, numTextures, componentType, dataType, precision, size);
        GM_CORE_ASSERT(inserted, "Tried to create cube map array render target which already existed. Using from cache.");
        CubeMapArrayRenderTarget* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_CubeMapArrayRenderTargets.erase(name);
            GM_CORE_ASSERT(false, "Cube map array render target is invalid.");
            return nullptr;
        }
        return ptr;
    }

    VertexObject* RenderContext::CreateVertexObject(const std::string& name, const unsigned int* indicesData, unsigned int numIndices)
    {
        auto [it, inserted] = m_VertexObjects.try_emplace(name, indicesData, numIndices);
        GM_CORE_ASSERT(inserted, "Tried to create VertexBbject which already existed. Using from cache.");
        VertexObject* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_VertexObjects.erase(name);
            GM_CORE_ASSERT(false, "VertexObject is invalid.");
            return nullptr;
        }
        return ptr;
    }

    SSBO* RenderContext::CreateSSBO(const std::string& name, unsigned int bindingID, unsigned int numBytes)
    {
        auto [it, inserted] = m_SSBOs.try_emplace(name, bindingID, numBytes);
        GM_CORE_ASSERT(inserted, "Tried to create ssbo which already existed. Using from cache.");
        SSBO* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_SSBOs.erase(name);
            GM_CORE_ASSERT(false, "SSBO is invalid.");
            return nullptr;
        }
        return ptr;
    }

    Shader* RenderContext::LoadShader(const std::string& name, const std::string& path)
    {
        auto [it, inserted] = m_Shaders.try_emplace(name, path.c_str());
        GM_CORE_ASSERT(inserted, "Tried to load shader which already existed. Using from cache.");
        Shader* ptr = &it->second;
        if (!ptr->IsValid())
        {
            m_Shaders.erase(name);
            GM_CORE_ASSERT(false, "Shader is invalid.");
            return nullptr;
        }
        return ptr;
    }

    FrameBuffer* RenderContext::GetFrameBuffer(const std::string& name)
    {
        auto it = m_FrameBuffers.find(name);
        if (it == m_FrameBuffers.end())
        {
            GM_CORE_ASSERT(false, "Framebuffer does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    SinglesampleRenderTarget* RenderContext::GetSingleSampleRenderTarget(const std::string& name)
    {
        auto it = m_SingleSampleRenderTargets.find(name);
        if (it == m_SingleSampleRenderTargets.end())
        {
            GM_CORE_ASSERT(false, "Single sample render target does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    MultisampleRenderTarget* RenderContext::GetMultiSampleRenderTarget(const std::string& name)
    {
        auto it = m_MultiSampleRenderTargets.find(name);
        if (it == m_MultiSampleRenderTargets.end())
        {
            GM_CORE_ASSERT(false, "Multi sample render target does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    CubeMapArrayRenderTarget* RenderContext::GetCubeMapArrayRenderTarget(const std::string& name)
    {
        auto it = m_CubeMapArrayRenderTargets.find(name);
        if (it == m_CubeMapArrayRenderTargets.end())
        {
            GM_CORE_ASSERT(false, "Cube map array render target does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    VertexObject* RenderContext::GetVertexObject(const std::string& name)
    {
        auto it = m_VertexObjects.find(name);
        if (it == m_VertexObjects.end())
        {
            GM_CORE_ASSERT(false, "VertexObject does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    SSBO* RenderContext::GetSSBO(const std::string& name)
    {
        auto it = m_SSBOs.find(name);
        if (it == m_SSBOs.end())
        {
            GM_CORE_ASSERT(false, "SSBO does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    Shader* RenderContext::GetShader(const std::string& name)
    {
        auto it = m_Shaders.find(name);
        if (it == m_Shaders.end())
        {
            GM_CORE_ASSERT(false, "Shader does not exist.");
            return nullptr;
        }
        return &it->second;
    }

    void RenderContext::DeleteFrameBuffer(const std::string& name)
    {
        GM_CORE_ASSERT(m_FrameBuffers.contains(name), "Tried to delete framebuffer which does not exist.");
        m_FrameBuffers.erase(name);
    }

    void RenderContext::DeleteSingleSampleRenderTarget(const std::string& name)
    {
        GM_CORE_ASSERT(m_SingleSampleRenderTargets.contains(name), "Tried to delete single sample rendertarget which does not exist.");
        m_SingleSampleRenderTargets.erase(name);
    }

    void RenderContext::DeleteMultiSampleRenderTarget(const std::string& name)
    {
        GM_CORE_ASSERT(m_MultiSampleRenderTargets.contains(name), "Tried to delete multi sample rendertarget which does not exist.");
        m_MultiSampleRenderTargets.erase(name);
    }

    void RenderContext::DeleteCubeMapArrayRenderTarget(const std::string& name)
    {
        GM_CORE_ASSERT(m_CubeMapArrayRenderTargets.contains(name), "Tried to delete cube map array rendertarget which does not exist.");
        m_CubeMapArrayRenderTargets.erase(name);
    }

    void RenderContext::DeleteVertexObject(const std::string& name)
    {
        GM_CORE_ASSERT(m_VertexObjects.contains(name), "Tried to delete vertexobject which does not exist.");
        m_VertexObjects.erase(name);
    }

    void RenderContext::UnloadShader(const std::string& name)
    {
        GM_CORE_ASSERT(m_Shaders.contains(name), "Tried to delete shader which does not exist.");
        m_Shaders.erase(name);
    }

    void RenderContext::DeleteDataBuffer(const std::string& name)
    {
        GM_CORE_ASSERT(m_DataBuffers.contains(name), "Tried to delete databuffer which does not exist.");
        m_DataBuffers.erase(name);
    }
}