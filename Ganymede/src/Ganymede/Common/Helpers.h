#pragma once

#include "Ganymede/Core/Core.h"

#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include <chrono>
#include <unordered_map>

class btTransform;
class btQuaternion;

namespace Helpers
{
	using namespace std::chrono;

	class Random
	{
	public:
		static float RandomFloatInRange(float from, float to);
	};

	class GANYMEDE_API ScopedTimer
	{
	public:
		ScopedTimer(const char* message)
		{
			
			m_StartMicros  = duration_cast<microseconds>(
				system_clock::now().time_since_epoch()
				).count();
			m_Message = message;
		};

		~ScopedTimer()
		{
			const uint64_t endMicros = duration_cast<microseconds>(
				system_clock::now().time_since_epoch()
				).count();

			const float msTakenFloat = static_cast<float>(endMicros - m_StartMicros);

			std::unordered_map<const char*, float>::const_iterator it = m_Timings.find(m_Message);

			float* time;
			if (it == m_Timings.end())
			{
				m_Timings[m_Message] = 0.f;
			}
			time = &m_Timings[m_Message];

			(*time) += msTakenFloat;
		}

		static std::unordered_map<const char*, float> m_Timings;

	private:
		double m_StartMicros;
		const char* m_Message;
	};

	class GANYMEDE_API NamedCounter
	{
	public:
		static void Increment(const char* name, unsigned int number)
		{
			std::unordered_map<const char*, unsigned int>::const_iterator it = m_NamedCounts.find(name);

			unsigned int* counts;
			if (it == m_NamedCounts.end())
			{
				m_NamedCounts[name] = 0;
			}
			counts = &m_NamedCounts[name];

			(*counts) += number;
		}

		static void Increment(const char* name)
		{
			Increment(name, 1);
		}

		static std::unordered_map<const char*, unsigned int> m_NamedCounts;
	};

#define SCOPED_TIMER(msg) \
Helpers::ScopedTimer timer = Helpers::ScopedTimer(msg); \
static_cast<void>(timer); \

#define NAMED_COUNTER(msg)\
Helpers::NamedCounter::Increment(msg);
#define NUMBERED_NAMED_COUNTER(msg, number)\
Helpers::NamedCounter::Increment(msg, number); 

	static std::string ParseFileNameFromPath(const std::string& path)
	{
		return path.substr(path.find_last_of("/\\") + 1);
	}

	btTransform GLMMatrixToBullet(const glm::mat4& mat);
	glm::mat4 BulletMatrixToGLM(const btTransform& bulletMatrix);
	glm::quat btQuaternionToGLMQuat(const btQuaternion& btQuat);
};