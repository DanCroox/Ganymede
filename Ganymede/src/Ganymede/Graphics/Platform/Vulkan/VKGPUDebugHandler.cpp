#include "Ganymede/Graphics/Platform/Vulkan/VKGPUDebugHandler.h"

namespace Ganymede
{
    void VKGPUDebugHandler::Enable()
    {
        m_IsEnabled = true;
    }

    void VKGPUDebugHandler::Disable()
    {
        if (!m_IsEnabled)
        {
            GM_CORE_ASSERT(false, "Already disabled.");
            return;
        }

        m_IsEnabled = false;
    }
}