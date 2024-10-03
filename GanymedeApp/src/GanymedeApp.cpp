#include "GanymedeApp.h"

#include <Ganymede/Log/Log.h>
#include <Ganymede/Runtime/ApplicationWindow.h>

#include <iostream>


void GanymedeApp::Run()
{
	GM_INFO("GanymedeApp started");
	
	ApplicationWindow window;
	EventSystem& eventSystem = EventSystem::GetInstance();

	EventCallbackHandle windowInitEventCBHandle;
	eventSystem.SubscribeEvent<WindowInitializeEvent>(windowInitEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameInit));
	EventCallbackHandle windowTickEventCBHandle;
	eventSystem.SubscribeEvent<WindowTickEvent>(windowTickEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameTick));
	EventCallbackHandle windowEndEventCBHandle;
	eventSystem.SubscribeEvent<WindowCloseEvent>(windowEndEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameEnd));

	if (!window.TryStart())
	{
		GM_INFO("Cannot initialize GLEW");
		std::cin.get();
	}

	eventSystem.UnsubscribeEvent<WindowInitializeEvent>(windowInitEventCBHandle);
	eventSystem.UnsubscribeEvent<WindowTickEvent>(windowTickEventCBHandle);
	eventSystem.UnsubscribeEvent<WindowCloseEvent>(windowEndEventCBHandle);

	GM_INFO("GanymedeApp shutdown");
	std::cin.get();
}

void GanymedeApp::GameInit(WindowInitializeEvent&)
{
	GM_INFO("Game Initialized");
}

void GanymedeApp::GameTick(WindowTickEvent& event)
{
	GM_INFO("GanymedeApp Tick: {} - {}", event.GetFrameDelta(), event.GetGameTime());
}

void GanymedeApp::GameEnd(WindowCloseEvent&)
{
	GM_INFO("Game Ended");
}