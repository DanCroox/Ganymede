#include "RigidBody.h"

#include "btBulletDynamicsCommon.h"


#include "Ganymede/Common/Helpers.h"

namespace Ganymede
{
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

    void RigidBody::SetFriction(float friction)
    {
        m_RigidBody->setFriction(friction);
    }

    void RigidBody::SetSleepingThresholds(float linear, float angular)
    {
        m_RigidBody->setSleepingThresholds(linear, angular);
    }
}