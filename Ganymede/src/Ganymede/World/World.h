#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/System/Thread.h"
#include "entt/entt.hpp"
#include <optional>

namespace Ganymede
{
	/// <summary>
	/// Tag type to define components white-list when searching for entities. Take a look at the EntityView type.
	/// </summary>
	/// <typeparam name="...Types"></typeparam>
	template<typename... Types>	struct Include {};

	/// <summary>
	/// Tag type to define components black-list when searching for entities. Take a look at the EntityView type.
	/// </summary>
	/// <typeparam name="...Types"></typeparam>
	template<typename... Types>	struct Exclude {};

	/// <summary>
	/// Base type alias for world entities.
	/// </summary>
	using Entity = entt::entity;

	class GANYMEDE_API World
	{
	private:
		template<typename... Packs> struct EntityViewWrapper;

		template<typename... CIncluded, typename... CExcluded>
		struct EntityViewWrapper<Include<CIncluded...>, Exclude<CExcluded...>>
		{
			using type = decltype(std::declval<entt::registry>().template view<CIncluded...>(entt::exclude_t<CExcluded...>{}));
		};

		template<typename... CIncluded>
		struct EntityViewWrapper<Include<CIncluded...>>
		{
			using type = decltype(std::declval<entt::registry>().template view<CIncluded...>());
		};

	public:
		template<typename IncludePack, typename ExcludePack = Exclude<>>
		using EntityView = typename EntityViewWrapper<IncludePack, ExcludePack>::type;

		World();
		~World() = default;

		/// <summary>
		/// Initializes (on first tick) and Ticks tickable components.
		/// </summary>
		/// <param name="deltaTime">Frame delta time.</param>
		void Tick(double deltaTime);

		/// <summary>
		/// Creates a new empty entity in the world.
		/// </summary>
		/// <returns>New empty entity.</returns>
		Entity CreateEntity()
		{
			return m_EntityRegistry.create();
		}

		/// <summary>
		/// Destroys given entity entirely.
		/// </summary>
		/// <returns>Entity to destroy.</returns>
		void DestroyEntity(Entity entity)
		{
			m_EntityRegistry.destroy(entity);
		}

		/// <summary>
		/// Adds a new component to an existing entity. A component can only be added once to an entity.
		/// </summary>
		/// <typeparam name="CType">Desired component type to add.</typeparam>
		/// <param name="entity">Entity to add the new component to.</param>
		/// <param name="...args">Desired component type constructor properties (if needed).</param>
		/// <returns>Newly created component reference or void (depending if component type has data - empty tag-components will cause void-return.</returns>
		template<typename CType, typename... Args>
		std::conditional_t<std::is_empty_v<CType>, void, CType&> AddComponent(Entity entity, Args&&... args)
		{
			if constexpr (std::is_empty_v<CType>)
			{
				m_EntityRegistry.emplace<CType>(entity, std::forward<Args>(args)...);
			}
			else
			{
				return m_EntityRegistry.emplace<CType>(entity, std::forward<Args>(args)...);
			}
		}

		/// <summary>
		/// Removes number of components from given entity.
		/// </summary>
		/// <typeparam name="...CType">Component types to remove.</typeparam>
		/// <param name="entity">Entity from which the given components shall be removed.</param>
		template<typename... CType>
		void RemoveComponents(Entity entity)
		{
			m_EntityRegistry.remove<CType...>(entity);
		}

		/// <summary>
		/// Returns world entities with given component-setup.
		/// 
		/// Usage:
		/// EntityView<Include<Tickable, Transform>> r = GetEntities(Include<Tickable, Transform>{});
		/// -> Returns all entities containing Tickable- and Transform- component.
		/// 
		/// EntityView<Include<Tickable, Transform>, Exclude<Invisible>> r = GetEntities(Include<Tickable, Transform>{}, Exclude<Invisible>{});
		/// -> Returns all entities containing Tickable- and Transform-component and excludes those with Invisible-component.
		/// </summary>
		/// <param name="">(Required) Include entities with these component-types.</param>
		/// <param name="">(Optional) Exclude entities with these component-types.</param>
		/// <returns>Found entities</returns>
		template<typename... Included, typename... Excluded>
		auto GetEntities(Include<Included...>, Exclude<Excluded...> = {})
		{
			if constexpr (sizeof...(Excluded) == 0)
			{
				return m_EntityRegistry.view<Included...>();
			}
			else
			{
				return m_EntityRegistry.view<Included...>(entt::exclude<Excluded...>);
			}
		}

		/// <summary>
		/// Returns world entities with given component-setup.
		/// 
		/// Usage:
		/// EntityView<Include<Tickable, Transform>> r = GetEntities(Include<Tickable, Transform>{});
		/// -> Returns all entities containing Tickable- and Transform- component.
		/// 
		/// EntityView<Include<Tickable, Transform>, Exclude<Invisible>> r = GetEntities(Include<Tickable, Transform>{}, Exclude<Invisible>{});
		/// -> Returns all entities containing Tickable- and Transform-component and excludes those with Invisible-component.
		/// </summary>
		/// <param name="">(Required) Include entities with these component-types.</param>
		/// <param name="">(Optional) Exclude entities with these component-types.</param>
		/// <returns>Found entities</returns>
		template<typename... Included, typename... Excluded>
		auto GetEntities(Include<Included...>, Exclude<Excluded...> = {}) const
		{
			if constexpr (sizeof...(Excluded) == 0)
			{
				return m_EntityRegistry.template view<Included...>();
			}
			else
			{
				return m_EntityRegistry.template view<Included...>(entt::exclude<Excluded...>);
			}
		}

		/// <summary>
		/// Tries to retrieve a component from an existing entity. Optionally use "HasComponent" before.
		/// </summary>
		/// <typeparam name="CType">Component type to search for.</typeparam>
		/// <param name="entity">Entity to retrieve the component from.</param>
		/// <returns>Has value reference if component was found. Otherwise invalid.</returns>
		template<typename CType>
		std::optional<std::reference_wrapper<CType>> GetComponentFromEntity(Entity entity)
		{
			if (CType* c = m_EntityRegistry.try_get<CType>(entity))
			{
				return std::ref(*c);
			}
			
			return std::nullopt;
		}

		/// <summary>
		/// Tries to retrieve a component from an existing entity. Optionally use "HasComponent" before.
		/// </summary>
		/// <typeparam name="CType">Component type to search for.</typeparam>
		/// <param name="entity">Entity to retrieve the component from.</param>
		/// <returns>Has value reference if component was found. Otherwise invalid.</returns>
		template<typename CType>
		std::optional<std::reference_wrapper<const CType>> GetComponentFromEntity(Entity entity) const
		{
			if (const CType* c = m_EntityRegistry.try_get<CType>(entity))
			{
				return std::ref(*c);
			}

			return std::nullopt;
		}

		/// <summary>
		/// Exclusive check if entity has given components.
		/// </summary>
		/// <typeparam name="...CType">Components to search for.</typeparam>
		/// <param name="entity">Entity to search in.</param>
		/// <returns>True when entity has all given components.</returns>
		template<typename... CType>
		bool HasComponents(Entity entity) const
		{
			return m_EntityRegistry.all_of<CType...>(entity);
		}

		/// <summary>
		/// Sorts components by given prerdiacate.
		/// </summary>
		/// <typeparam name="CType">Component types to sort.</typeparam>
		/// <param name="compare">Predicate</param>
		template<typename CType, typename Compare>
		void SortComponents(Compare&& compare)
		{
			m_EntityRegistry.sort<CType>(std::forward<Compare>(compare));
		}

	private:
		std::vector<Thread> m_TickThreadPool;
		entt::registry m_EntityRegistry;
	};

	/// <summary>
	/// Entity type to define entities of certain type. This wraps around the internal ECS.
	/// Entites
	/// </summary>
	/// <typeparam name="IncludePack">(Required) Search for entities with given component. Example: EntityView<Include<CompA, CompB, CompC>></typeparam>
	/// <typeparam name="ExcludePack">(Optional) Excludes entities whith given component. Example: EntityView<Include<CompA, CompB, CompC>, Exclude<CompD>></typeparam>
	template<typename IncludePack, typename ExcludePack = Exclude<>>
	using EntityView = typename World::EntityView<IncludePack, ExcludePack>;
}