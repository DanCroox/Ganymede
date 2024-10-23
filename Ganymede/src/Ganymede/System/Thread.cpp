#include "Thread.h"
#include <chrono>
#include <iostream>
#include <atomic>

namespace Ganymede
{
	Thread::Thread() :
		m_WorkerFunction(nullptr),
		m_IsRunning(false),
		m_DoTerminate(false)
	{
		m_Thread = std::thread([this]() { Loop(); });
	}

	Thread::~Thread()
	{
		Terminate();
	}

	Thread::Thread(Thread&& other) noexcept
	{
		Join();
		m_Thread = std::move(other.m_Thread);
		m_IsRunning = other.m_IsRunning.load();
		m_DoTerminate = other.m_DoTerminate.load();
	}

	Thread& Thread::operator=(Thread&& other) noexcept
	{
		Join();
		m_Thread = std::move(other.m_Thread);
		m_IsRunning = other.m_IsRunning.load();
		m_DoTerminate = other.m_DoTerminate.load();
		return *this;
	}

	void Thread::Loop()
	{
		while (!m_DoTerminate)
		{
			std::unique_lock<std::mutex> l(m_Mutex);
			m_CV.wait(l, [this] { return m_IsRunning || m_DoTerminate; });
			if (m_DoTerminate)
				break;

			if (m_WorkerFunction != nullptr)
				m_WorkerFunction();

			m_IsRunning = false;
			m_CV.notify_all();
		}
	}

	void Thread::Start()
	{
		{
			std::unique_lock<decltype(m_Mutex)> l(m_Mutex);
			m_IsRunning = true;
		}
		m_CV.notify_one();
	}

	void Thread::Join()
	{
		std::unique_lock<std::mutex> l(m_Mutex);
		m_CV.wait(l, [this] { return !m_IsRunning.load(); });
	}

	void Thread::Terminate() {
		{
			std::unique_lock<std::mutex> l(m_Mutex);
			m_DoTerminate = true;
			m_WorkerFunction = nullptr;
			if (!m_IsRunning.load()) {
				Start();
			}
		}
		m_CV.notify_all();
		m_Thread.join();
	}
}