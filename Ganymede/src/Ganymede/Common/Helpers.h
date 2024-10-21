#pragma once

#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

class btTransform;
class btQuaternion;

namespace Ganymede
{
	namespace Helpers
	{
		using namespace std::chrono;

		class GANYMEDE_API Random
		{
		public:
			static float RandomFloatInRange(float from, float to);
		};

		class GANYMEDE_API ScopedTimer
		{
		public:
			ScopedTimer(const char* message)
			{
				std::scoped_lock(m_Mutex);
				m_StartMicros = duration_cast<microseconds>(
					system_clock::now().time_since_epoch()
				).count();
				m_Message = message;
			};

			~ScopedTimer()
			{
				std::scoped_lock(m_Mutex);
				const uint64_t endMicros = duration_cast<microseconds>(
					system_clock::now().time_since_epoch()
				).count();

				const float msTakenFloat = static_cast<float>(endMicros - m_StartMicros);
				const auto [it, inserted] = m_Timings.try_emplace(m_Message);
				float& time = it->second;
				if (inserted)
				{
					time = 0.0f;
				}
				time += msTakenFloat;
			}

			static inline const std::unordered_map<const char*, float>& GetData() { return m_Timings; }
			static inline void ClearData() { m_Timings.clear(); }

		private:
			static std::unordered_map<const char*, float> m_Timings;
			double m_StartMicros;
			const char* m_Message;
			static std::mutex m_Mutex;
		};

		class GANYMEDE_API NamedCounter
		{
		public:
			static void Increment(const char* name, unsigned int number)
			{
				std::scoped_lock(m_Mutex);
				const auto [it, inserted] = m_NamedCounts.try_emplace(name);
				unsigned int& counts = it->second;
				if (inserted)
				{
					counts = 0;
				}
				counts += number;
			}

			static void Increment(const char* name)
			{
				Increment(name, 1);
			}

			static inline const std::unordered_map<const char*, unsigned int>& GetData() { return m_NamedCounts; }
			static inline void ClearData() { m_NamedCounts.clear(); }

		private:
			static std::unordered_map<const char*, unsigned int> m_NamedCounts;
			static std::mutex m_Mutex;
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
}