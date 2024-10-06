#pragma once

#include "Ganymede/Core/Core.h"

#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <algorithm>

namespace Ganymede
{

	class GANYMEDE_API Thread
	{
	public:
		typedef std::function<void()> WorkerFunction;

		Thread();
		~Thread();

		Thread(const Thread&) = delete;
		Thread& operator=(const Thread&) = delete;

		Thread(Thread&& other) noexcept;

		Thread& operator=(Thread&& other) noexcept;

		void Loop();
		void Start();
		void Join();
		inline bool IsRunning() const { return m_IsRunning; }
		inline bool IsAlive() const { return m_Thread.joinable(); }

		void Terminate();

		WorkerFunction m_WorkerFunction;
	private:
		std::thread m_Thread;
		std::mutex m_Mutex;
		std::condition_variable m_CV;
		volatile bool m_IsRunning; //TODO: Technically only this line needs volatile. For safety we use for both lines. No 100% sure yet why it is necessary. Need more investigation later.
		volatile bool m_DoTerminate;
	};
}