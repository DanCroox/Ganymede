#pragma once

#include <array>

#include <Ganymede.h>

using namespace Ganymede;

class GanymedeApp : public Ganymede::Application
{
public:
	GanymedeApp() = default;
	~GanymedeApp() = default;

	void Run() override;

	void GameInit(Ganymede::WindowInitializeEvent&);
	void GameTick(Ganymede::WindowTickEvent& event);
	void GameEnd(Ganymede::WindowCloseEvent&);

private:
	std::unique_ptr<AssetLoader> m_AssetLoader = nullptr;
	std::unique_ptr<PlayerCharacter> m_PlayerCharacter = nullptr;
	std::unique_ptr<World> m_World = nullptr;
	std::unique_ptr<Renderer> m_Renderer = nullptr;
	std::unique_ptr<FPSCamera> m_Camera = nullptr;
	std::unique_ptr<WorldPartitionManager> m_WorldPartitionManager = nullptr;
	std::unique_ptr<PhysicsWorld> m_PhysicsWorld = nullptr;
	std::unique_ptr<NavMesh> m_NavMesh = nullptr;
};

// This is an external function, invoked by the main function within the Ganymede core.
// That is the main entry point of your Ganymede application.
Ganymede::Application* Ganymede::CreateApplication()
{
	return new GanymedeApp();
}