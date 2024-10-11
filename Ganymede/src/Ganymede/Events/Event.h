#pragma once

#include "Ganymede/Core/Core.h"

#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>


#define CREATE_EVENT_CLASS_ID(Class) GM_GENERATE_STATIC_CLASS_ID(Class) \
								virtual ClassID GetClassID() const override { return GetStaticClassID(); }

#define EVENT_BIND_TO_MEMBER(func) std::bind(&func, this, std::placeholders::_1)

namespace Ganymede
{
	class GANYMEDE_API Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;

		virtual ClassID GetClassID() const = 0;
	};


	typedef std::function<void(Event&)> GANYMEDE_API EventFunctionType;


	class GANYMEDE_API EventCallbackHandle
	{
	public:
		EventCallbackHandle() = default;
		~EventCallbackHandle() = default;

		inline void SetFunction(EventFunctionType func) { m_Function = func; }

		EventCallbackHandle(const EventCallbackHandle&) = delete;
		EventCallbackHandle& operator=(const EventCallbackHandle&) = delete;

		inline EventFunctionType GetFunction() { return m_Function; }

	private:
		EventFunctionType m_Function;
	};


	class GANYMEDE_API EventSystem
	{
	public:
		EventSystem() = default;
		~EventSystem() = default;

		EventSystem(const EventSystem&) = delete;
		EventSystem& operator=(const EventSystem&) = delete;

		template<typename T>
		void DispatchEvent(Event& event, std::function<void(T&)> func)
		{
			if (event.GetClassID() == T::GetStaticClassID())
			{
				func(*static_cast<T*>(&event));
			}
		}

		template<typename T>
		void SubscribeEvent(EventCallbackHandle& funcHandle, std::function<void(T&)> func)
		{
			auto [it, inserted] = m_SubscribedEvents.emplace(T::GetStaticClassID(), std::vector<EventCallbackHandle*>());
			std::vector<EventCallbackHandle*>& subs = it->second;

			funcHandle.SetFunction([func, this](Event& ev) { DispatchEvent<T>(ev, func); });

			subs.push_back(&funcHandle);
		}

		template<typename T>
		void UnsubscribeEvent(EventCallbackHandle& funcHandle)
		{
			auto eventIterator = m_SubscribedEvents.find(T::GetStaticClassID());
			if (eventIterator != m_SubscribedEvents.end())
			{
				auto& subscribedEvents = eventIterator->second;
				for (auto it = subscribedEvents.begin(); it != subscribedEvents.end(); ++it)
				{
					if ((*it) == &funcHandle)
					{
						subscribedEvents.erase(it);
						break;
					}
				}
			}
		}

		void NotifyEvent(Event& event);
		inline void NotifyEvent(Event&& event) { NotifyEvent(event); }

	private:
		std::unordered_map<ClassID, std::vector<EventCallbackHandle*>> m_SubscribedEvents;
	};
}