#pragma once

#include "Ganymede/Core/Core.h"


#include "MeshWorldObjectInstance.h"

namespace Ganymede
{

	class GANYMEDE_API MeshWorldObjectInstanceDoor : public MeshWorldObjectInstance
	{
		using MeshWorldObjectInstance::MeshWorldObjectInstance;

	public:
		GM_GENERATE_CLASSTYPEINFO(MeshWorldObjectInstanceDoor, MeshWorldObjectInstance);

		void OnCreate() override;
		void Tick(float deltaTime) override;

		void SetDoorOpen(bool aDoorOpen) { m_IsDoorOpen = aDoorOpen; }
		bool IsDoorOpen() const { return m_IsDoorOpen; }
	private:
		float m_DoorCloseRotation;
		float m_DoorOpenRotation = 1.57079633f;
		float m_DoorOpenInterpolator = 0;
		bool m_IsDoorOpen = false;
	};
}