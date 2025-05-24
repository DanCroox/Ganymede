#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	struct GCDoor
	{
		float m_DoorCloseRotation;
		float m_DoorOpenRotation = 1.57079633f;
		float m_DoorOpenInterpolator = 0;
		bool m_IsDoorOpen = false;
	};
}