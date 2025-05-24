#pragma once
#include "Ganymede/Core/Core.h"

#include <entt/entt.hpp>

namespace Ganymede
{
	struct GCRelation
	{
		entt::entity m_Parent{ entt::null };
		entt::entity m_FirstChild{ entt::null };
		entt::entity m_PrevSibling{ entt::null };
		entt::entity m_NextSibling{ entt::null };

		std::size_t children{ 0 };
	};
}