#include "Event.h"

namespace Ganymede
{
	void EventSystem::NotifyEvent(Event& event)
	{
		auto eventIterator = m_SubscribedEvents.find(event.GetClassID());
		if (eventIterator != m_SubscribedEvents.end())
		{
			auto& subscribedEvents = eventIterator->second;
			std::for_each(subscribedEvents.begin(), subscribedEvents.end(),
				[&event](EventCallbackHandle* funcHandle)
				{
					funcHandle->GetFunction()(event);
				});
		}
	}
}