#include "PhysicsWorld.h"

#include <iostream>
#include <vector>
#include "Ganymede/System/Types.h"

#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/matrix_decompose.hpp"
#include "Ganymede/Common/Helpers.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "btBulletDynamicsCommon.h"

namespace Ganymede
{
    PhysicsWorld::PhysicsWorld()
    {
        m_CollisionConfiguration = new btDefaultCollisionConfiguration();
        m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);

        //m_OverlappingPairCache = new btDbvtBroadphase();
        m_OverlappingPairCache = new btAxisSweep3({ -1000,-1000 ,-1000 }, { 1000,1000 ,1000 });
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

    KinematicCharacterController PhysicsWorld::CreateCapsule(float radius, float height, float /*mass*/, glm::vec3 startPosition)
    {
        btTransform groundTransform = btTransform::getIdentity();
        groundTransform.setOrigin({ startPosition.x, startPosition.y, startPosition.z });

        btPairCachingGhostObject* m_ghostObject = new btPairCachingGhostObject();
        m_ghostObject->setWorldTransform(groundTransform);
        m_OverlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        btCapsuleShape* capsule = new btCapsuleShape(radius, height);

        m_ghostObject->setCollisionShape(capsule);
        m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

        btKinematicCharacterController* character = new btKinematicCharacterController(m_ghostObject, capsule, .35, { 0, 1, 0 });
        character->setMaxSlope(0.25f);

        // We use a collider, not a rigidbody. This is not part of the physics simulation, but is still able to collide with things.
        // Since its a kinematic object by nature, we need to set its collision flag to "Kinematic", no matter what the mass- property gives.
        // This property is unused in this implementation.
        m_ghostObject->setCollisionFlags(m_ghostObject->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

        m_DynamicsWorld->addCollisionObject(m_ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
        m_DynamicsWorld->addAction(character);

        return character;
    }

    RigidBody PhysicsWorld::AddRigidBodyFromMeshWorldObject(MeshWorldObjectInstance& mwoi, float mass)
    {
        //btTriangleMesh* trimesh = new btTriangleMesh();
        const MeshWorldObject* mwo = mwoi.GetMeshWorldObject();
        //mass = 0;
        const bool isDynamic = (mass > 0.f);


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
        shape->setLocalScaling({ scale.x, scale.y, scale.z });

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
#endif  //USE_MOTIONSTATE

        if (!isDynamic)
        {
            //Flag is important to not degredate performance. Bulletphysics will also assert in debug builds with wrong flagging.
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        }

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
}