#ifdef GM_PLATFORM_WINDOWS

#include "VKGLFWWindow.h"

#include "GLFW/glfw3.h"
#include "volk.h"

namespace Ganymede
{
	VKGLFWWindow::~VKGLFWWindow()
	{
	}

	bool VKGLFWWindow::Initialize()
	{
		return false;
	}

	void* VKGLFWWindow::GetNativeWindow()
	{
		return nullptr;
	}

	bool VKGLFWWindow::TryStart()
	{
		return true;
	}

	void VKGLFWWindow::SetVSyncEnabled(bool isEnabled)
	{
	}

	bool VKGLFWWindow::IsVSyncEnabled() const
	{
		return false;
	}
}
#endif //GM_PLATFORM_WINDOWS