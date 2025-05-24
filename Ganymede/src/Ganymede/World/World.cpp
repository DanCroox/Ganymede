#include "World.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCTickable.h"
#include "Ganymede/Log/Log.h"
#include <functional>

namespace Ganymede
{
	World::World()
	{
		unsigned int threadCount = std::thread::hardware_concurrency();
		GM_CORE_ASSERT(threadCount > 0, "Couldn't read number of hardware threads. Entity-Tick functions will be computed in main thread!");

		GM_CORE_INFO("Thread count set to %d", threadCount);
		m_TickThreadPool.resize(threadCount);
	}

	void World::Tick(double deltaTime)
	{
		SCOPED_TIMER("World Tick");
		const auto tickableEntities = GetEntities(Include<GCTickable>{});

		if (m_TickThreadPool.size() == 0)
		{
			for (auto [entity, gcTickable] : tickableEntities.each())
			{
				gcTickable.Initialize(*this, entity, deltaTime);
				gcTickable.Tick(*this, entity, deltaTime);
			}
		}
		else
		{
			const unsigned int numThreads = m_TickThreadPool.size();
			for (auto [entity, gcTickable] : tickableEntities.each())
			{
				Thread* thread;
				int threadIndex = 0;
				while (true)
				{
					thread = &(m_TickThreadPool[threadIndex]);
					if (!thread->IsRunning())
					{
						break;
					}
					// Check if next thread is available
					threadIndex = (threadIndex + 1) % numThreads;
				}

				thread->m_WorkerFunction = [this, &gcTickable, entity, deltaTime]()
					{
						gcTickable.Initialize(*this, entity, deltaTime);
						gcTickable.Tick(*this, entity, deltaTime);
					};
				thread->Start();
			}
		}

		// Wait for all threads to finish
		for (Thread& thread : m_TickThreadPool)
		{
			thread.Join();
		}
	}
}