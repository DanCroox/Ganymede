#pragma once

#include "Ganymede/Core/TypeOrderedList.h"

namespace Ganymede
{
	class WorldObjectInstance;

	using WorldObjectInstanceList = TypeOrderedList<WorldObjectInstance*>;

	template <typename TransformType>
	using ListSlice = WorldObjectInstanceList::TransformView<TransformType, WorldObjectInstanceList::iterator>;

	template <typename TransformType>
	using ConstListSlice = WorldObjectInstanceList::TransformView<TransformType, WorldObjectInstanceList::const_iterator>;
}