#include "GanymedeApp.h"
#include "Ganymede/World/CreatureMeshWorldObjectInstance.h"

#include <iostream>
#include <glm/glm.hpp>
#include <chrono>

void GanymedeApp::Run()
{
	GM_INFO("GanymedeApp started");

	Window& window = Application::Get().GetRenderWindow();
	EventSystem& eventSystem = Application::Get().GetEventSystem();

	EventCallbackHandle windowInitEventCBHandle;
	eventSystem.SubscribeEvent<WindowInitializeEvent>(windowInitEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameInit));
	EventCallbackHandle windowTickEventCBHandle;
	eventSystem.SubscribeEvent<WindowTickEvent>(windowTickEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameTick));
	EventCallbackHandle windowEndEventCBHandle;
	eventSystem.SubscribeEvent<WindowCloseEvent>(windowEndEventCBHandle, EVENT_BIND_TO_MEMBER(GanymedeApp::GameEnd));

	if (!window.TryStart())
	{
		GM_INFO("Cannot initialize GLEW");
	}

	eventSystem.UnsubscribeEvent<WindowInitializeEvent>(windowInitEventCBHandle);
	eventSystem.UnsubscribeEvent<WindowTickEvent>(windowTickEventCBHandle);
	eventSystem.UnsubscribeEvent<WindowCloseEvent>(windowEndEventCBHandle);

	GM_INFO("GanymedeApp shutdown");
}

void GanymedeApp::GameInit(WindowInitializeEvent&)
{
	GM_INFO("Initializing Game");
	
	Application& app = Application::Get();
	m_AssetLoader = std::make_unique<AssetLoader>();
	m_PhysicsWorld = std::make_unique<PhysicsWorld>();
	m_World = std::make_unique<World>(*m_AssetLoader);
	m_Renderer = std::make_unique<Renderer>(m_AssetLoader->GetShaderManager());
	m_Camera = std::make_unique<FPSCamera>();
	m_PlayerCharacter = std::make_unique<PlayerCharacter>(*m_World, *m_PhysicsWorld, *m_Camera);
	m_NavMesh = std::make_unique<NavMesh>(*m_Renderer);

	std::vector<const WorldObject*> loadedAssets;
	loadedAssets = m_AssetLoader->LoadFromPath("res/models/animationtest.glb");
	GM_INFO("WorldObjects loaded");

	glm::vec3 worldBoundsMin(Numbers::MAX_FLOAT);
	glm::vec3 worldBoundsMax(Numbers::MIN_FLOAT);

	SCOPED_TIMER("Initializing static world objects");
	for (auto asset : loadedAssets)
	{
		if (const SkeletalMeshWorldObject* smeshwo = dynamic_cast<const SkeletalMeshWorldObject*>(asset))
		{
			SkeletalMeshWorldObjectInstance* instance;
			if (asset->GetName().find("Matschkopf") == 0)
			{
				// In the glb file there is an object with above name. We create multiple autonomous intances of this npc
					for (int i = 0; i < 800; ++i)
				{
					instance = new CreatureMeshWorldObjectInstance(smeshwo, *m_NavMesh, *m_PlayerCharacter, *m_PhysicsWorld, *m_World, *m_AssetLoader);
					instance->SetMobility(WorldObjectInstance::Mobility::Dynamic);
					m_World->AddToWorld(instance);
					m_World->AddWorldObjectInstance(instance);
				}
			}
			else
			{
				// Skeletal meshes are objects that are likely to change location and/or change bounding box dimension so set to mobility to "Dynamic"
				// TODO: Skeletal meshes dont have physics boddies yet
				if (instance = (SkeletalMeshWorldObjectInstance*)m_World->CreateWorldObjectInstance(asset->GetName()))
				{
					// REWORK: SkeletalMeshWorldObjectInstance wont be stored automatically right now! Needs proper impelmenting (the entire type loading needs some touch up to be more generic) 
					instance->SetMobility(WorldObjectInstance::Mobility::Dynamic);
					m_World->AddWorldObjectInstance(instance);
				}
			}
		}
		else if (const MeshWorldObject* meshwo = dynamic_cast<const MeshWorldObject*>(asset))
		{
			MeshWorldObjectInstance* instance = (MeshWorldObjectInstance*)m_World->CreateWorldObjectInstance(asset->GetName());
			m_World->AddWorldObjectInstance(instance);

			const glm::vec3& bbVertMin = instance->GetTransform() * glm::vec4(instance->GetMeshWorldObject()->m_Meshes[0]->m_BoundingBoxVertices[7].m_Position, 1.0f); //Left Bottom Front
			const glm::vec3& bbVertMax = instance->GetTransform() * glm::vec4(instance->GetMeshWorldObject()->m_Meshes[0]->m_BoundingBoxVertices[1].m_Position, 1.0f); //Right Top Back

			worldBoundsMin = glm::min(worldBoundsMin, bbVertMin);
			worldBoundsMax = glm::max(worldBoundsMax, bbVertMax);
				
			instance->SetPhysicsWorld(*m_PhysicsWorld);

			if (meshwo->GetPreferredPhysicsState() == MeshWorldObject::PreferredPhysicsState::Dynamic)
			{
				instance->MakeRigidBody(10);
				instance->GetRigidBody().SetFriction(300.f);
				instance->GetRigidBody().SetRestitution(.001f); // bouncyness ... less is less bouncy
				instance->GetRigidBody().SetSleepingThresholds(5.f, 5.f);
				instance->SetMobility(WorldObjectInstance::Mobility::Dynamic);
			}
			else if (meshwo->GetPreferredPhysicsState() == MeshWorldObject::PreferredPhysicsState::Static &&
				!meshwo->GetExcludeFromNavigationMesh())
			{
				instance->MakeRigidBody(0);
			}
		}
		else if (dynamic_cast<const PointlightWorldObject*>(asset) != nullptr)
		{
			// Pointlights are also "WorldObjects" which can be used to create an instance of this pointlight data into the world. 
			PointlightWorldObjectInstance* pointlight = (PointlightWorldObjectInstance*)m_World->CreateWorldObjectInstance(asset->GetName());

			m_World->AddWorldObjectInstance(pointlight);
		}
	}

	GM_INFO("World loaded. Size: {}, {}, {} x {}, {}, {}", worldBoundsMin.x, worldBoundsMin.y, worldBoundsMin.z, worldBoundsMax.x, worldBoundsMax.y, worldBoundsMax.z);

	// Generate world partition octree
	m_WorldPartitionManager = std::make_unique<WorldPartitionManager>(glm::floor(worldBoundsMin), glm::ceil(worldBoundsMax), m_AssetLoader->GetShaderManager());
	GM_INFO("World partitioning done.");

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	m_NavMesh->Generate(m_World->GetWorldObjectInstances<MeshWorldObjectInstance>());
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - duration;
	GM_INFO("Nav mesh generation done in {} ms.", dur.count());

	const std::vector<MeshWorldObjectInstance*>& allMeshWorldObjectsInstances = *m_World->GetWorldObjectInstancesByType<MeshWorldObjectInstance>();
	for (const auto& mwoi : allMeshWorldObjectsInstances)
	{
		m_WorldPartitionManager->AddWorldObjectInstance(mwoi);
	}
	GM_INFO("All world instances spawned.");
}

void GanymedeApp::GameTick(Ganymede::WindowTickEvent& event)
{
	GMTime::s_DeltaTime = event.GetFrameDelta();
	GMTime::s_Time = event.GetGameTime();
	m_PhysicsWorld->Step(event.GetFrameDelta());
	m_PlayerCharacter->Tick(event.GetFrameDelta());
	m_World->Tick(event.GetFrameDelta());
	m_Renderer->Draw(*m_World, *m_WorldPartitionManager, *m_Camera);
}

void GanymedeApp::GameEnd(Ganymede::WindowCloseEvent&)
{
	GM_INFO("Game Ended");
	std::cin.get();
}