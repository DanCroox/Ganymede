#include "PhysicsWorld.h"

#include <iostream>
#include <vector>
#include "Ganymede/System/Types.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/matrix_decompose.hpp"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "btBulletDynamicsCommon.h"

PhysicsWorld::PhysicsWorld()
{
    m_CollisionConfiguration = new btDefaultCollisionConfiguration();
    m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);

    //m_OverlappingPairCache = new btDbvtBroadphase();
    m_OverlappingPairCache = new btAxisSweep3({ -1000,-1000 ,-1000 }, {1000,1000 ,1000 });
    m_Solver = new btSequentialImpulseConstraintSolver;
    m_DynamicsWorld = new btDiscreteDynamicsWorld(m_Dispatcher, m_OverlappingPairCache, m_Solver, m_CollisionConfiguration);
    m_DynamicsWorld->setGravity(btVector3(0, -9.81, 0));

    m_DynamicsWorld->setForceUpdateAllAabbs(false);
}

PhysicsWorld::~PhysicsWorld()
{
    delete m_DynamicsWorld;
    delete m_Dispatcher;
    delete m_OverlappingPairCache;
    delete m_Solver;
    delete m_CollisionConfiguration;
}

void PhysicsWorld::Step(float time)
{
    m_DynamicsWorld->stepSimulation(time, 10);
}

KinematicCharacterController PhysicsWorld::CreateCapsule(float radius, float height, float mass, glm::vec3 startPosition)
{
    btTransform groundTransform = btTransform::getIdentity();
    groundTransform.setOrigin({ startPosition.x, startPosition.y, startPosition.z });

    btPairCachingGhostObject* m_ghostObject = new btPairCachingGhostObject();
    m_ghostObject->setWorldTransform(groundTransform);
    m_OverlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    btCapsuleShape* capsule = new btCapsuleShape(radius, height);

    m_ghostObject->setCollisionShape(capsule);
    m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

    btKinematicCharacterController* character = new btKinematicCharacterController(m_ghostObject, capsule, .35, {0, 1, 0});
    character->setMaxSlope(0.25f);

    m_DynamicsWorld->addCollisionObject(m_ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

    m_DynamicsWorld->addAction(character);

    return character;

    //btVector3 localInertia(0, 0, 0);
    //shape->calculateLocalInertia(mass, localInertia);
    


    //btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    //btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

    /*
    btRigidBody* body = new btRigidBody(cInfo);

    //Constrain all rotation. Capsule does need it
    body->setAngularFactor({ 0, 0, 0 });

    body->setFriction(3);

    body->setActivationState(DISABLE_DEACTIVATION);

    body->setUserIndex(-1);
    m_DynamicsWorld->addRigidBody(body);

    return body;
    */
    
}

/*
btCollisionObject* PhysicsWorld::AddStaticBodyFromMeshWorldObject(MeshWorldObjectInstance& mwoi, float mass)
{
    //btTriangleMesh* trimesh = new btTriangleMesh();
    const MeshWorldObject* mwo = mwoi.GetMeshWorldObject();
    //mass = 0;
    const bool isDynamic = (mass != 0.f);


    btTransform groundTransform;

    glm::mat4 trans = mwoi.GetTransform();
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(trans, scale, rotation, translation, skew, perspective);

    trans = glm::mat4(1.0f);
    // trans = glm::scale(trans, m_Scale);

    trans = glm::translate(trans, translation);
    trans = trans * glm::mat4(rotation);

    float dArray[16];
    const float* pSource = (const float*)glm::value_ptr(trans);
    for (int i = 0; i < 16; ++i)
        dArray[i] = pSource[i];
    groundTransform.setFromOpenGLMatrix((float*)dArray);

    const std::vector<MeshWorldObject::Mesh*>& meshes = mwo->m_Meshes;
    //const std::vector<unsigned int>& indices = mwo->m_VertexIndicies;
    //const std::vector<Vector3f>& vertices = mwo->m_VertexPositions;

    btCollisionShape* shape = nullptr;
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
    {
        btConvexHullShape* s = new btConvexHullShape();
        for (MeshWorldObject::Mesh* mesh : meshes)
        {
            const std::vector<unsigned int>& indices = mesh->m_VertexIndicies;
            const std::vector<MeshWorldObject::Mesh::Vertex>& vertices = mesh->m_Vertices;
            for (int i = 0; i < mesh->m_VertexIndicies.size(); i += 3)
            {
                btVector3 p1 = { vertices[indices[i]].m_Position.x, vertices[indices[i]].m_Position.y, vertices[indices[i]].m_Position.z };
                btVector3 p2 = { vertices[indices[i + 1]].m_Position.x, vertices[indices[i + 1]].m_Position.y, vertices[indices[i + 1]].m_Position.z };
                btVector3 p3 = { vertices[indices[i + 2]].m_Position.x, vertices[indices[i + 2]].m_Position.y, vertices[indices[i + 2]].m_Position.z };

                s->addPoint(p1);
                s->addPoint(p2);
                s->addPoint(p3);
            }
        }
        shape = s;

        // only on non static objects or dynamic objects which are convex
        shape->calculateLocalInertia(mass, localInertia);
    }
    else
    {
        btTriangleMesh* triangleMeshTerrain = new btTriangleMesh();
        for (MeshWorldObject::Mesh* mesh : meshes)
        {
            const std::vector<unsigned int>& indices = mesh->m_VertexIndicies;
            const std::vector<MeshWorldObject::Mesh::Vertex>& vertices = mesh->m_Vertices;
            for (int i = 0; i < indices.size(); i += 3)
            {
                btVector3 p1 = { vertices[indices[i]].m_Position.x,vertices[indices[i]].m_Position.y, vertices[indices[i]].m_Position.z };
                btVector3 p2 = { vertices[indices[i + 1]].m_Position.x, vertices[indices[i + 1]].m_Position.y, vertices[indices[i + 1]].m_Position.z };
                btVector3 p3 = { vertices[indices[i + 2]].m_Position.x, vertices[indices[i + 2]].m_Position.y, vertices[indices[i + 2]].m_Position.z };

                triangleMeshTerrain->addTriangle(p1, p2, p3, true);
            }
        }
        shape = new btBvhTriangleMeshShape(triangleMeshTerrain, true, true);

    }
    shape->setLocalScaling({ scale.x, scale.y, scale.z });

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

    btCollisionObject* body = new btCollisionObject();
    body->setCollisionShape(shape);

#else
    btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
    body->setWorldTransform(groundTransform);
#endif  //

    body->setUserIndex(-1);
    m_DynamicsWorld->addCollisionObject(body);
    return body;
}
*/

RigidBody PhysicsWorld::AddRigidBodyFromMeshWorldObject(MeshWorldObjectInstance& mwoi, float mass)
{
    //btTriangleMesh* trimesh = new btTriangleMesh();
    const MeshWorldObject* mwo = mwoi.GetMeshWorldObject();
    //mass = 0;
    const bool isDynamic = (mass != 0.f);
    
   
    btTransform groundTransform;

    glm::mat4 trans = mwoi.GetTransform();
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(trans, scale, rotation, translation, skew, perspective);

    trans = glm::mat4(1.0f);
   // trans = glm::scale(trans, m_Scale);
    
    trans = glm::translate(trans, translation);
    trans = trans * glm::mat4(rotation);

    float dArray[16];
    const float* pSource = (const float*)glm::value_ptr(trans);
    for (int i = 0; i < 16; ++i)
        dArray[i] = pSource[i];
    groundTransform.setFromOpenGLMatrix((float*)dArray);

    const std::vector<MeshWorldObject::Mesh*>& meshes = mwo->m_Meshes;
    //const std::vector<unsigned int>& indices = mwo->m_VertexIndicies;
    //const std::vector<Vector3f>& vertices = mwo->m_VertexPositions;

    btCollisionShape* shape = nullptr;
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
    {
        btConvexHullShape* s = new btConvexHullShape();
        for (MeshWorldObject::Mesh* mesh : meshes)
        {
            const std::vector<unsigned int>& indices = mesh->m_VertexIndicies;
            const std::vector<MeshWorldObject::Mesh::Vertex>& vertices = mesh->m_Vertices;
            for (int i = 0; i < mesh->m_VertexIndicies.size(); i += 3)
            {
                btVector3 p1 = { vertices[indices[i]].m_Position.x, vertices[indices[i]].m_Position.y, vertices[indices[i]].m_Position.z };
                btVector3 p2 = { vertices[indices[i + 1]].m_Position.x, vertices[indices[i + 1]].m_Position.y, vertices[indices[i + 1]].m_Position.z };
                btVector3 p3 = { vertices[indices[i + 2]].m_Position.x, vertices[indices[i + 2]].m_Position.y, vertices[indices[i + 2]].m_Position.z };

                s->addPoint(p1);
                s->addPoint(p2);
                s->addPoint(p3);
            }
        }
        shape = s;

        // only on non static objects or dynamic objects which are convex
        shape->calculateLocalInertia(mass, localInertia);
        shape->setMargin(0.0f);
    }
    else
    {
        btTriangleMesh* triangleMeshTerrain = new btTriangleMesh();
        for (MeshWorldObject::Mesh* mesh : meshes)
        {
            const std::vector<unsigned int>& indices = mesh->m_VertexIndicies;
            const std::vector<MeshWorldObject::Mesh::Vertex>& vertices = mesh->m_Vertices;
            for (int i = 0; i < indices.size(); i += 3)
            {
                btVector3 p1 = { vertices[indices[i]].m_Position.x,vertices[indices[i]].m_Position.y, vertices[indices[i]].m_Position.z };
                btVector3 p2 = { vertices[indices[i + 1]].m_Position.x, vertices[indices[i + 1]].m_Position.y, vertices[indices[i + 1]].m_Position.z };
                btVector3 p3 = { vertices[indices[i + 2]].m_Position.x, vertices[indices[i + 2]].m_Position.y, vertices[indices[i + 2]].m_Position.z };

                triangleMeshTerrain->addTriangle(p1, p2, p3, false);
            }
        }

        shape = new btBvhTriangleMeshShape(triangleMeshTerrain, true, true);

    }
    shape->setLocalScaling({ scale .x, scale .y, scale .z});

    //shape->setLocalScaling(btVector3(.85f, .85f, .85f));


    //btCollisionShape* box = new btBvhTriangleMeshShape(trimesh, false);

    //btBoxShape* box = new btBoxShape(btVector3(5,5,5));

//    btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    

    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

    btRigidBody* body = new btRigidBody(cInfo);
    //body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
    btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
    body->setWorldTransform(groundTransform);
#endif  //

    body->setUserPointer(&mwoi);

    body->setUserIndex(-1);
    m_DynamicsWorld->addRigidBody(body);
    // REWORK: Do proper pointer handling!
    return RigidBody(body);
}

void PhysicsWorld::RemoveRigidBody(RigidBody body)
{
    // REWORK: DO proper pointer handling!!!
    m_DynamicsWorld->removeRigidBody(body.m_RigidBody);
}

RayResult PhysicsWorld::RayCast(glm::vec3 fromWorld, glm::vec3 toWorld)
{
    RayResult rayResult;

    btVector3 from = { fromWorld.x, fromWorld.y, fromWorld.z };
    btVector3 to = { toWorld.x, toWorld.y, toWorld.z };
    btCollisionWorld::ClosestRayResultCallback result(from, to);
    result.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
    result.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

    m_DynamicsWorld->rayTest(from, to, result);

    rayResult.m_HasHit = result.hasHit();
    if (rayResult.m_HasHit)
    {
        rayResult.m_CollisionObject = result.m_collisionObject->getUserPointer();
        rayResult.m_HitFraction = result.m_collisionObject->getHitFraction();
        rayResult.m_HitWorldLocation = { result.m_hitPointWorld.getX(), result.m_hitPointWorld.getY(), result.m_hitPointWorld.getZ() };
        rayResult.m_HitNormalWorld = { result.m_hitNormalWorld.getX(), result.m_hitNormalWorld.getY(), result.m_hitNormalWorld.getZ() };
    }

    return rayResult;
}


RigidBody::RigidBody(btRigidBody* rigidBody)
{
    m_RigidBody = rigidBody;
}

bool RigidBody::IsValid() const
{
    return m_RigidBody != nullptr;
}

void RigidBody::Delete()
{
    delete m_RigidBody;
}

void RigidBody::SetDamping(float linearDamping, float angularDamping)
{
    m_RigidBody->setDamping(linearDamping, angularDamping);
}

glm::mat4 RigidBody::GetCenterOfMassTransform() const
{
    return Helpers::BulletMatrixToGLM(m_RigidBody->getCenterOfMassTransform());
}

void RigidBody::SetCenterOfMassTransform(glm::mat4 transform)
{
    m_RigidBody->setCenterOfMassTransform(Helpers::GLMMatrixToBullet(transform));
}

glm::mat4 RigidBody::GetWorldTransform() const
{
    return Helpers::BulletMatrixToGLM(m_RigidBody->getWorldTransform());
}

glm::vec3 RigidBody::GetCollisionShapeLocalScaling() const
{
    const btVector3 scale = m_RigidBody->getCollisionShape()->getLocalScaling();
    return { scale.getX(), scale.getY(), scale.getZ() };
}

void RigidBody::SetRestitution(float restitution)
{
    m_RigidBody->setRestitution(restitution);
}

void RigidBody::ApplyImpulse(glm::vec3 impulse, glm::vec3 position)
{
    m_RigidBody->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3(position.x, position.y, position.z));
}


KinematicCharacterController::KinematicCharacterController(btKinematicCharacterController* btCharacterController)
{
    m_btCharacterController = btCharacterController;
}

void KinematicCharacterController::Delete()
{
    // REWORK: Do proper poiner handling!!!
    delete m_btCharacterController;
}

glm::mat4 KinematicCharacterController::GetGhostObjectWorldTransform() const
{
    return Helpers::BulletMatrixToGLM(m_btCharacterController->getGhostObject()->getWorldTransform());
}

bool KinematicCharacterController::CanJump() const
{
    return m_btCharacterController->canJump();
}

void KinematicCharacterController::Jump(glm::vec3 force)
{
    m_btCharacterController->jump({ force.x, force.y, force.z });
}

void KinematicCharacterController::SetWalkDirection(glm::vec3 direction)
{
    m_btCharacterController->setWalkDirection({ direction.x, direction.y, direction.z });
}