#include "WorldPartition.h"

#include "GL/glew.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Common/Helpers.h"
#include "RendererTypes.h"


namespace WorldPartition_Private
{
    static const unsigned int s_LeafNodeLevel = 1;

    bool QueryResultReady(unsigned int queryID)
    {
        GLint result = 0;
        glGetQueryObjectiv(queryID, GL_QUERY_RESULT_AVAILABLE, &result);
        return result == GL_TRUE;
    }

    bool QueryResult(unsigned int queryID)
    {
        GLint result = 0;
        glGetQueryObjectiv(queryID, GL_QUERY_RESULT, &result);
        return result == GL_TRUE;
    }

    const constexpr std::array<unsigned int, 36> p_NodeIndices = {
    1, 2, 5,
    5, 2, 6,
    0, 2, 1,  // Bottom face
    0, 3, 2,
    4, 5, 6,  // Top face
    4, 6, 7,
    0, 5, 4,  // Side faces
    0, 1, 5,
    2, 7, 6,
    2, 3, 7,
    0, 4, 7,  // Front face
    0, 7, 3
    };
}

WorldPartitionNode::WorldPartitionNode(WorldPartitionNodeID id) : m_ID(id)
{
    glGenQueries(1, &m_OcclusionQueryID);

}

WorldPartitionNode::~WorldPartitionNode()
{
    glDeleteQueries(1, &m_OcclusionQueryID);
}

bool WorldPartitionNode::HasWorldPartitionNodeChild(WorldPartitionNodeID nodeID) const
{
    const auto it = m_ChildNodes.find(nodeID);
    return (it != m_ChildNodes.end());
}

void WorldPartitionNode::AddNodeChild(WorldPartitionNodeID nodeID, WorldPartitionNode* node)
{
    m_ChildNodes.insert(std::make_pair(nodeID, node));
}

WorldPartitionNode& WorldPartitionManager::GetOrCreateWorldPartitionNode(const WorldPartitionNodeID& nodeID)
{
    std::unordered_map<WorldPartitionNodeID, WorldPartitionNode*>::iterator it = m_WorldPartitionNodes.find(nodeID);

    if (it != m_WorldPartitionNodes.end())
    {
        return *it->second;
    }

    WorldPartitionNode* node = new WorldPartitionNode(nodeID);
    m_WorldPartitionNodes.insert(std::make_pair(nodeID, node));

    if (m_RootNode == nullptr || m_RootNode->GetID().m_Level < node->GetID().m_Level)
    {
        m_RootNode = node;
    }

    return *node;
}

WorldPartitionNodeID WorldPartitionManager::FindParentNodeWorldLocation(const WorldPartitionNodeID& nodeID) const
{
    const unsigned int objNodeLevel = nodeID.m_Level;
    const glm::i32vec3& objNodeDimension = nodeID.m_Dimension;
    const glm::i32vec3& nodeWorldlocation = nodeID.m_WorldLocation;

    const glm::i32vec3& rootNodeWorldLocation = m_AllMeshesBoundingBoxMin;
    const glm::i32vec3& rootNodeDimension = m_AllMeshesBoundingBoxMax - m_AllMeshesBoundingBoxMin;

    const glm::i32vec3& parentNodeDimension = objNodeDimension * glm::i32vec3(2);
    glm::i32vec3 parentNodeWorldLocation(0);

    for (int i = rootNodeWorldLocation.x; i < rootNodeWorldLocation.x + rootNodeDimension.x; i += parentNodeDimension.x)
    {
        if (i > nodeWorldlocation.x)
            break;
        parentNodeWorldLocation.x = i;
    }

    for (int i = rootNodeWorldLocation.y; i < rootNodeWorldLocation.y + rootNodeDimension.y; i += parentNodeDimension.y)
    {
        if (i > nodeWorldlocation.y)
            break;
        parentNodeWorldLocation.y = i;
    }

    for (int i = rootNodeWorldLocation.z; i < rootNodeWorldLocation.z + rootNodeDimension.z; i += parentNodeDimension.z)
    {
        if (i > nodeWorldlocation.z)
            break;
        parentNodeWorldLocation.z = i;
    }

    return { parentNodeWorldLocation, objNodeDimension * 2, objNodeLevel * 2 };
}

bool WorldPartitionManager::IsInViewFrustum(const WorldPartitionNode* node)
{
    // TODO: Highly inefficient. Only testing code. Improve later.
    glm::mat4 camVp = m_Camera->GetProjection() * m_Camera->GetTransform();

    std::vector<glm::i32vec3> points;
    points.push_back(node->GetID().m_WorldLocation);
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(node->GetID().m_Dimension.x, 0, 0));
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(node->GetID().m_Dimension.x, node->GetID().m_Dimension.y, 0));
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(0, node->GetID().m_Dimension.y, 0));

    points.push_back(node->GetID().m_WorldLocation + node->GetID().m_Dimension);
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(node->GetID().m_Dimension.x, 0, node->GetID().m_Dimension.z));
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(0, node->GetID().m_Dimension.y, node->GetID().m_Dimension.z));
    points.push_back(node->GetID().m_WorldLocation + glm::i32vec3(0, 0, node->GetID().m_Dimension.z));

    int clipSides[6];
    for (glm::i32vec3 vertex : points)
    {
        glm::vec4 clipPoint = camVp * glm::vec4(vertex, 1);

        clipSides[0] += clipPoint.x < -clipPoint.w; //left of Left plane
        clipSides[1] += clipPoint.x > clipPoint.w;  //right of Right plane
        clipSides[2] += clipPoint.y < -clipPoint.w; //below Bottom plane
        clipSides[3] += clipPoint.y > clipPoint.w;  //above Top plane
        clipSides[4] += clipPoint.z < -clipPoint.w; //in front of Near plane
        clipSides[5] += clipPoint.z > clipPoint.w;  //behind Far plane
    }

    const bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
        clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

    return !isOutSideFrustum;

}

bool WorldPartitionManager::IsPointInsideWorldPartitionNode(const glm::vec3& point, const WorldPartitionNode& node)
{
    const WorldPartitionNodeID& nodeID = node.GetID();

    glm::vec3 bbVertMinF = nodeID.m_WorldLocation;
    glm::vec3 bbVertMaxF = bbVertMinF + glm::vec3(nodeID.m_Dimension);

    bbVertMinF = bbVertMinF - glm::vec3(.2f);
    bbVertMaxF = bbVertMaxF + glm::vec3(.2f);

    const bool isInBoundingBox = (point.x >= bbVertMinF.x && point.x <= bbVertMaxF.x) &&
        (point.y >= bbVertMinF.y && point.y <= bbVertMaxF.y) &&
        (point.z >= bbVertMinF.z && point.z <= bbVertMaxF.z);

    return isInBoundingBox;
}

void WorldPartitionManager::IssueOcclusionQuery(WorldPartitionNode* node)
{
    using namespace WorldPartition_Private;

    const auto& it = m_NodeVAOLookup.find(node);
    if (it == m_NodeVAOLookup.end())
    {
        unsigned int vaoID, vboID, eboID = 0;
        GLCall(glGenVertexArrays(1, &vaoID));
        GLCall(glBindVertexArray(vaoID));

        GLCall(glGenBuffers(1, &vboID));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, vboID));
        const glm::vec3 bbLocation = glm::vec3(node->GetID().m_WorldLocation);
        const glm::vec3 bbDimension = glm::vec3(node->GetID().m_Dimension);
        const float offset = .2f;
        std::array<MeshWorldObject::Mesh::BoundingBoxVertex, 8> bboxV;
        bboxV[0] = { bbLocation + glm::vec3(-offset, -offset, -offset), glm::normalize(glm::vec3(-1.f, -1.f, -1.f)) }; // Left, Bottom, Back
        bboxV[1] = { bbLocation + glm::vec3(bbDimension.x, 0, 0) + glm::vec3(offset, -offset, -offset), glm::normalize(glm::vec3(1.f, -1.f, -1.f)) };  // Right, Bottom, Back
        bboxV[2] = { bbLocation + glm::vec3(bbDimension.x, bbDimension.y, 0) + glm::vec3(offset, offset, -offset), glm::normalize(glm::vec3(1.f, 1.f, -1.f)) };   // Right, Top, Back
        bboxV[3] = { bbLocation + glm::vec3(0, bbDimension.y, 0) + glm::vec3(-offset, offset, -offset), glm::normalize(glm::vec3(-1.f, 1.f, -1.f)) };  // Left, Top, Back
        bboxV[4] = { bbLocation + glm::vec3(0, 0, bbDimension.z) + glm::vec3(-offset, -offset, offset), glm::normalize(glm::vec3(-1.f, -1.f, 1.f)) }; // Left, Bottom, Front
        bboxV[5] = { bbLocation + glm::vec3(bbDimension.x, 0, bbDimension.z) + glm::vec3(offset, -offset, offset), glm::normalize(glm::vec3(1.f, -1.f, 1.f)) };  // Right, Bottom, Front
        bboxV[6] = { bbLocation + bbDimension + glm::vec3(offset, offset, offset), glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };   // Right, Top, Front
        bboxV[7] = { bbLocation + glm::vec3(0, bbDimension.y, bbDimension.z) + glm::vec3(-offset, offset, offset), glm::normalize(glm::vec3(-1.f, 1.f, 1.f)) };  // Left, Top, Front
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(MeshWorldObject::Mesh::BoundingBoxVertex) * bboxV.size(), &bboxV[0], GL_STATIC_DRAW));

        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::BoundingBoxVertex), (const void*)offsetof(MeshWorldObject::Mesh::BoundingBoxVertex, m_Position)));

        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshWorldObject::Mesh::BoundingBoxVertex), (const void*)offsetof(MeshWorldObject::Mesh::BoundingBoxVertex, m_Normal)));

        GLCall(glGenBuffers(1, &eboID));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID));
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 36, &p_NodeIndices[0], GL_STATIC_DRAW));

        m_NodeVAOLookup[node] = vaoID;
    }
    else
    {
        GLCall(glBindVertexArray(it->second));
    }
    //m_OcclusionQueryShader->SetUniform1f("visibility", wpMan.m_DebugNode == node ? .2f : 0.f);

    GLCall(glBeginQuery(GL_ANY_SAMPLES_PASSED, node->GetOcclusionQueryID()));
    GLCall(glDrawElements(GL_TRIANGLES, 36 * sizeof(unsigned int), GL_UNSIGNED_INT, nullptr));
    GLCall(glEndQuery(GL_ANY_SAMPLES_PASSED));
};

std::vector<const MeshWorldObjectInstance*> WorldPartitionManager::UpdateVisibility()
{
    using namespace WorldPartition_Private;

    const FPSCamera& camera = *m_Camera;

    std::vector<const MeshWorldObjectInstance*> m_MeshWorldObjectInstances;

    m_TraversalStack.push_back(m_RootNode);
    while (!m_TraversalStack.empty() || !m_QueryQueue.empty())
    {
        // Process finished occlusion queries
        while (!m_QueryQueue.empty() &&
            (QueryResultReady(m_QueryQueue.front()->m_OcclusionQueryID) || m_TraversalStack.empty()))
        {
            WorldPartitionNode* node = m_QueryQueue.front();
            m_QueryQueue.erase(m_QueryQueue.begin()); // Dequeue
            while (!QueryResultReady(node->m_OcclusionQueryID)) {}
            const bool isVisible = QueryResult(node->m_OcclusionQueryID);
            if (isVisible)
            {
                PullUpVisibility(node);
                TraverseNode(node, m_MeshWorldObjectInstances);
            }
        }

        // Tree traversal
        if (!m_TraversalStack.empty())
        {
            WorldPartitionNode* node = m_TraversalStack.back();

            m_TraversalStack.pop_back();
            if (IsInViewFrustum(node)) //TODO: Inside view frustum?
            {
                const bool isCamInsideNode = IsPointInsideWorldPartitionNode(camera.GetPosition(), *node);
                if (isCamInsideNode)
                {
                    node->m_IsVisible = true;
                }

                const bool isLeaf = node->m_ChildNodes.empty();

                const bool wasVisible = node->m_IsVisible && (node->m_LastVisitedFrame == GMTime::s_FrameNumber - 1);
                const bool opened = wasVisible && !isLeaf;
                node->m_IsVisible = false;
                node->m_LastVisitedFrame = GMTime::s_FrameNumber;

                if (!opened && !isCamInsideNode)
                {
                    NAMED_COUNTER("Occlusion Queries");
                    IssueOcclusionQuery(node);
                    m_QueryQueue.push_back(node);
                }

                if (opened) //Checking for leafnode is not part of the original algo, but seems to be necessary! Otherwise traversenode is called which is odd cause its a leaf node anyway but would add meshes to renderlist which also happens when OQ is evaluated!
                {
                    TraverseNode(node, m_MeshWorldObjectInstances);
                }
                if (isCamInsideNode)
                {
                    for (const auto& mwoi : node->m_MeshWorldObjectInstances)
                    {
                        // Render!
                        m_MeshWorldObjectInstances.push_back(mwoi);
                    }
                }
            }
        }
    }

    return m_MeshWorldObjectInstances;
}

WorldPartitionManager::WorldPartitionManager(glm::i32vec3 allMeshesBoundingBoxMin, glm::i32vec3 allMeshesBoundingBoxMax, ShaderManager& shaderManager) :
    m_AllMeshesBoundingBoxMin(allMeshesBoundingBoxMin),
    m_AllMeshesBoundingBoxMax(allMeshesBoundingBoxMax)
{
    m_AllMeshesBoundingDimension = allMeshesBoundingBoxMax - allMeshesBoundingBoxMin;

    m_OcclusionQueryShader = shaderManager.RegisterAndLoadShader("occlusion");
}

void WorldPartitionManager::Prepare(const FPSCamera& camera)
{
    m_Camera = &camera;
    m_OcclusionQueryShader->Bind();
    const glm::mat4 viewProjection = m_Camera->GetProjection() * m_Camera->GetTransform();
    m_OcclusionQueryShader->SetUniformMat4f("u_VP", &viewProjection);
}

void WorldPartitionManager::Finish()
{
    GLCall(glBindVertexArray(0));
    m_OcclusionQueryShader->Unbind();
}

void WorldPartitionManager::PullUpVisibility(WorldPartitionNode* node)
{
    while (node != nullptr && !node->m_IsVisible)
    {
        node->m_IsVisible = true;
        node = node->GetParentNode();
    }
}

void WorldPartitionManager::TraverseNode(WorldPartitionNode* node, std::vector<const MeshWorldObjectInstance*>& meshWorldObjectInstances)
{
    for (const auto& mwoi : node->m_MeshWorldObjectInstances)
    {
        // Render!
        meshWorldObjectInstances.push_back(mwoi);
    }

    if (node->m_ChildNodes.empty())
    {
        return;
    }

    for (const auto& pair : node->m_ChildNodes)
    {
        WorldPartitionNode* childNode = pair.second;
        m_TraversalStack.push_back(childNode);
    }
}

void WorldPartitionManager::AddWorldObjectInstance(const MeshWorldObjectInstance* mwoi)
{
    // TODO: create bounding box for entire mwoi which includes all meshes. Besides bb on per-mesh level, maybe add a bb on per mwoi instance
    glm::vec3 minBB = glm::vec3(9999999);
    glm::vec3 maxBB = glm::vec3(-9999999);

    for (const MeshWorldObject::Mesh* mesh : mwoi->GetMeshWorldObject()->m_Meshes)
    {
        const glm::vec3& min = mesh->m_BoundingBoxVertices[7].m_Position;
        const glm::vec3& max = mesh->m_BoundingBoxVertices[1].m_Position;

        minBB.x = glm::min(min.x, minBB.x);
        minBB.y = glm::min(min.y, minBB.y);
        minBB.z = glm::min(min.z, minBB.z);

        maxBB.x = glm::max(max.x, maxBB.x);
        maxBB.y = glm::max(max.y, maxBB.y);
        maxBB.z = glm::max(max.z, maxBB.z);
    }

    glm::vec3 bbVertMinF2 = mwoi->GetTransform() * glm::vec4(minBB, 1.0f);
    glm::vec3 bbVertMaxF2 = mwoi->GetTransform() * glm::vec4(maxBB, 1.0f);

    glm::vec3 bbVertMinF(99999);
    glm::vec3 bbVertMaxF(-99999);

    bbVertMinF.x = glm::min(bbVertMinF2.x, bbVertMaxF2.x);
    bbVertMinF.y = glm::min(bbVertMinF2.y, bbVertMaxF2.y);
    bbVertMinF.z = glm::min(bbVertMinF2.z, bbVertMaxF2.z);
    bbVertMaxF.x = glm::max(bbVertMinF2.x, bbVertMaxF2.x);
    bbVertMaxF.y = glm::max(bbVertMinF2.y, bbVertMaxF2.y);
    bbVertMaxF.z = glm::max(bbVertMinF2.z, bbVertMaxF2.z);

    const glm::i32vec3& bbVertMin = glm::floor(bbVertMinF); // Left Bottom Back
    const glm::i32vec3& bbVertMax = glm::ceil(bbVertMaxF); // Right Top Front

    WorldPartitionNodeID nodeId{ glm::i32vec3(bbVertMin), glm::i32vec3(WorldPartition_Private::s_LeafNodeLevel), WorldPartition_Private::s_LeafNodeLevel };
    while (
        (bbVertMaxF.x > static_cast<float>((nodeId.m_WorldLocation.x) + static_cast<float>(nodeId.m_Dimension.x))) ||
        (bbVertMaxF.y > static_cast<float>((nodeId.m_WorldLocation.y) + static_cast<float>(nodeId.m_Dimension.y))) ||
        (bbVertMaxF.z > static_cast<float>((nodeId.m_WorldLocation.z) + static_cast<float>(nodeId.m_Dimension.z)))
        )
    {
        nodeId = FindParentNodeWorldLocation(nodeId);
    }

    WorldPartitionNode& node = *static_cast<WorldPartitionNode*>(&GetOrCreateWorldPartitionNode(nodeId));

    std::unordered_map<const MeshWorldObjectInstance*, WorldPartitionNode*>::iterator it = m_MeshWorldObjectInstanceToWorldPartitionNodeLookup.find(mwoi);

    if (nodeId.m_Level != 1)
    {
        WorldPartitionNodeID customNodeId = { bbVertMin, bbVertMax - bbVertMin, 0 };
        WorldPartitionNode* customNode = new WorldPartitionNode(customNodeId);
        node.AddNodeChild(customNodeId, customNode);
        customNode->SetParentNode(&node);
        customNode->AddMeshWorldObjectInstance(mwoi);
        m_MeshWorldObjectInstanceToWorldPartitionNodeLookup.insert(std::make_pair(mwoi, customNode));
    }
    else
    {
        node.AddMeshWorldObjectInstance(mwoi);
        m_MeshWorldObjectInstanceToWorldPartitionNodeLookup.insert(std::make_pair(mwoi, &node));
    }

    // Create node hierachy up to root node dimension
    WorldPartitionNodeID currentNodeID = nodeId;
    WorldPartitionNode* currentNode = &node;
    while (currentNodeID.m_Dimension.x < m_AllMeshesBoundingDimension.x || currentNodeID.m_Dimension.y < m_AllMeshesBoundingDimension.y || currentNodeID.m_Dimension.z < m_AllMeshesBoundingDimension.z)
    {
        const WorldPartitionNodeID parentNodeID = FindParentNodeWorldLocation(currentNodeID);
        WorldPartitionNode& parentNode = GetOrCreateWorldPartitionNode(parentNodeID);
        if (parentNode.HasWorldPartitionNodeChild(currentNodeID))
        {
            break;
        }

        currentNode->SetParentNode(&parentNode);
        parentNode.AddNodeChild(currentNodeID, currentNode);

        currentNodeID = parentNodeID;
        currentNode = &parentNode;
    }

}
WorldPartitionNode* WorldPartitionManager::FindWorldPartitionNodesByMeshWorldObjectInstance(const MeshWorldObjectInstance* mwoi) const
{
    std::unordered_map<const MeshWorldObjectInstance*, WorldPartitionNode*>::const_iterator it = m_MeshWorldObjectInstanceToWorldPartitionNodeLookup.find(mwoi);

    if (it != m_MeshWorldObjectInstanceToWorldPartitionNodeLookup.end())
    {
        return it->second;
    }

    return nullptr;
}