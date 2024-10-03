#pragma once

#define GLCall(x) x;

enum class RenderPass
{
    Geometry,
    LightDepth,
    Debug
};

struct PointLight
{
    glm::mat4 u_ShadowMatrices[6];
    glm::vec4 m_LightColor;
    glm::vec3 lightPos;
    int u_LightID = -1;
    glm::vec4 updateShadowMap = glm::vec4(0);
};

struct MeshInstances
{
    const MeshWorldObject* m_MeshWorldObject;
    const std::vector<MeshWorldObjectInstance*>* m_Instances;
    float m_DistanceToCamera;
};

struct MeshInstancess
{
    const MeshWorldObjectInstance* m_Instance;
    glm::mat4 m_MVP;
    int m_TargetLayerID = 0;
    int m_LightIndex = 0;
};