#include "Helpers.h"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <random>
#include <btBulletDynamicsCommon.h>
#include "glm/gtc/quaternion.hpp"

namespace Ganymede
{
	thread_local std::mt19937 generator{ std::random_device{}() };

	float Helpers::Random::RandomFloatInRange(float from, float to)
	{
		std::uniform_real_distribution<float> distribution(from, to);
		return distribution(generator);
	}

	std::unordered_map<const char*, float> Helpers::ScopedTimer::m_Timings;
	std::unordered_map<const char*, unsigned int> Helpers::NamedCounter::m_NamedCounts;

	// REWORK: Check if really works! Not tested yet
	btTransform Helpers::GLMMatrixToBullet(const glm::mat4& mat) {
		btTransform transform;

		// Setze die Position (Translation)
		transform.setOrigin(btVector3(mat[3][0], mat[3][1], mat[3][2]));

		// Setze die Rotation
		// Die Rotationskomponenten direkt aus der Matrix extrahieren
		transform.setBasis(btMatrix3x3(
			mat[0][0], mat[0][1], mat[0][2],
			mat[1][0], mat[1][1], mat[1][2],
			mat[2][0], mat[2][1], mat[2][2]
		));

		return transform;
	}

	glm::mat4 Helpers::BulletMatrixToGLM(const btTransform& bulletMatrix)
	{
		float glMat[16];
		// Transpose matrix and use for glm matrix
		bulletMatrix.getOpenGLMatrix(&glMat[0]);
		return glm::mat4(
			glMat[0], glMat[1], glMat[2], glMat[3],
			glMat[4], glMat[5], glMat[6], glMat[7],
			glMat[8], glMat[9], glMat[10], glMat[11],
			glMat[12], glMat[13], glMat[14], glMat[15]
		);
	}

	glm::quat Helpers::btQuaternionToGLMQuat(const btQuaternion& btQuat)
	{
		return glm::quat(btQuat.w(), btQuat.x(), btQuat.y(), btQuat.z());
	}
}