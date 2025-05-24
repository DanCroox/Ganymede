#pragma once
#include "Ganymede/Core/Core.h"

#include "entt/entt.hpp"

namespace Ganymede
{
    struct GCCreature2
    {
        static void bla(World& w, entt::entity e, float t)
        {

        }
    };

	struct GCTickable
	{
        using Function = std::function<void(World&, entt::entity, float)>;

        GCTickable() = delete;

        GCTickable(const GCTickable&) = default;
        GCTickable(GCTickable&&) = default;
        GCTickable& operator=(const GCTickable&) = default;
        GCTickable& operator=(GCTickable&&) = default;

        explicit GCTickable(GCCreature2& stuff)
        {
            m_TickFunctor = stuff.bla;
        }

        explicit GCTickable(Function tickFunctor, Function initFunctor = {})
            : m_InitFunctor(std::move(initFunctor)),
            m_TickFunctor(std::move(tickFunctor)),
            m_IsInitialized(false)
        {
            GM_CORE_ASSERT(m_TickFunctor, "Tick functor invalid");
        }

        // Called once during frame tick when object got created (not called during construction to ensure deterministic call event).
        void Initialize(World& world, entt::entity entity, float deltaTime)
        {
            if (m_IsInitialized)
            {
                return;
            }

            if (m_InitFunctor)
            {
                m_InitFunctor(world, entity, deltaTime);
            }

            m_IsInitialized = true;
        }

        // Called per frame tick. If object was initialized this frame and there is a bound Init-functor, Initialize will also be called once).
        void Tick(World& world, entt::entity entity, float deltaTime)
        {
            m_TickFunctor(world, entity, deltaTime);
        }

    private:
        Function m_InitFunctor;
        Function m_TickFunctor;
        bool m_IsInitialized;
    };
}