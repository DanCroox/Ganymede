#pragma once

#include "Ganymede/Graphics/GPUDebugHandler.h"

namespace Ganymede
{
	class GANYMEDE_API OGLGPUDebugHandler : public GPUDebugHandler
	{
	public:
		OGLGPUDebugHandler() = default;

		void Enable() override;
		void Disable() override;

		bool IsEnabled() const override { return m_IsEnabled; }

	private:
		bool m_IsEnabled;
	};
}