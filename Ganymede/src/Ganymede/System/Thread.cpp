#include "Thread.h"
#include <chrono>
#include <iostream>

//Thread
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
	m_IsRunning = other.m_IsRunning;
	m_DoTerminate = other.m_DoTerminate;
}

Thread& Thread::operator=(Thread&& other) noexcept
{
	Join();
	m_Thread = std::move(other.m_Thread);
	m_IsRunning = other.m_IsRunning;
	m_DoTerminate = other.m_DoTerminate;
	return *this;
}

void Thread::Loop()
{
	while (!m_DoTerminate)
	{
		std::unique_lock<std::mutex> l(m_Mutex);
		m_CV.wait(l, [this] { return m_IsRunning; });
		if (m_WorkerFunction != nullptr)
			m_WorkerFunction();
		
		m_IsRunning = false;
	}
}

void Thread::Start()
{
	std::unique_lock<decltype(m_Mutex)> l(m_Mutex);
	m_IsRunning = true;
	m_CV.notify_one();
}

void Thread::Join()
{
	std::unique_lock<std::mutex> l(m_Mutex);
	m_CV.wait(l, [this] { return !m_IsRunning; });
}

void Thread::Terminate() {
	if (!m_Thread.joinable())
		return;

	m_DoTerminate = true;
	m_WorkerFunction = nullptr;
	if (!IsRunning()) {
		Start();
	}
	m_Thread.join();
}