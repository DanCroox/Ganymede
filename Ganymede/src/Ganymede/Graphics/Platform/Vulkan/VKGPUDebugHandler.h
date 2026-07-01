#pragma once

#include "Ganymede/Graphics/GPUDebugHandler.h"

namespace Ganymede
{
	class GANYMEDE_API VKGPUDebugHandler : public GPUDebugHandler
	{
	public:
		VKGPUDebugHandler() = default;

		void Enable() override;
		void Disable() override;

		bool IsEnabled() const override { return m_IsEnabled; }

	private:
		bool m_IsEnabled;
	};
}