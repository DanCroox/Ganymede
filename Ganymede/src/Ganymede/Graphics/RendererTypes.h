#pragma once

#include "Ganymede/Core/Core.h"

#define GLCall(x) x;


namespace Ganymede
{
    class MeshWorldObject;
    class MeshWorldObjectInstance;
    
    enum class GANYMEDE_API RenderPass
    {
        Geometry,
        LightDepth,
        Debug
    };

    struct GANYMEDE_API PointLight
    {
        glm::vec4 m_LightColor;
        glm::vec3 lightPos;
        int u_LightID = -1;
    };

    struct GANYMEDE_API MeshInstances
    {
        const MeshWorldObject* m_MeshWorldObject;
        const std::vector<MeshWorldObjectInstance*>* m_Instances;
        float m_DistanceToCamera;
    };

    struct GANYMEDE_API MeshInstancess
    {
        const MeshWorldObjectInstance* m_Instance;
        glm::mat4 m_MVP;
        int m_TargetLayerID = 0;
        int m_LightIndex = 0;
    };
}