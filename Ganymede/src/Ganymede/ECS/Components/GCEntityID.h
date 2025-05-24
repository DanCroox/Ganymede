#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/World/MeshWorldObject.h"

namespace Ganymede
{
	// TODO: Use a free-list implementation later
	inline int myStaticVar = 1;

	struct GCEntityID
	{
		size_t m_ID = myStaticVar++;
	};
}