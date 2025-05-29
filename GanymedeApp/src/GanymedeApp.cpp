#include "GanymedeApp.h"
#include "EntityHelpers.h"

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

void GanymedeApp::GameInit(Ganymede::WindowInitializeEvent&)
{
	GM_INFO("Initializing Game");
	
	Application& app = Application::Get();
	m_AssetLoader = std::make_unique<AssetLoader>();
	m_PhysicsWorld = std::make_unique<PhysicsWorld>();
	m_World = std::make_unique<World>();
	m_NavMesh = std::make_unique<NavMesh>();

	m_RenderContext = std::make_unique<RenderContext>(*m_World);
	m_RenderPipeline = std::make_unique<RenderPipeline>(*m_RenderContext);
	m_RenderPipeline->AddRenderPass<PrepareFrameRenderPass>();
	//m_RenderPipeline->AddRenderPass<CollectGeometryPass>();
	m_RenderPipeline->AddRenderPass<ComputePass>();
	m_RenderPipeline->AddRenderPass<GeometryRenderPass>();
	//m_RenderPipeline->AddRenderPass<ShadowMappingRenderPass>();
	m_RenderPipeline->AddRenderPass<LightingRenderPass>();
	m_RenderPipeline->AddRenderPass<CompositeRenderPass>();
	m_RenderPipeline->Initialize();

	m_Camera = std::make_unique<FPSCamera>(m_RenderContext->CreateRenderView());
	m_PlayerCharacter = std::make_unique<PlayerCharacter>(*m_World, *m_PhysicsWorld, *m_Camera);
	
	std::vector<const WorldObject*> loadedAssets;
	loadedAssets = m_AssetLoader->LoadFromPath("res/models/animationtest.glb");
	//loadedAssets = m_AssetLoader->LoadFromPath("res/models/physicstest.glb");
	//loadedAssets = m_AssetLoader->LoadFromPath("res/models/backroom2.glb");
	GM_INFO("WorldObjects loaded");

	glm::vec3 worldBoundsMin(Numbers::MAX_FLOAT);
	glm::vec3 worldBoundsMax(Numbers::MIN_FLOAT);

	SCOPED_TIMER("Initializing static world objects");
	for (auto asset : loadedAssets)
	{
		if (const SkeletalMeshWorldObject* smeshwo = dynamic_cast<const SkeletalMeshWorldObject*>(asset))
		{
			if (asset->GetName().find("Matschkopf") == 0)
			{
				for (int i = 0; i < 1000; ++i)
				{
					entt::entity entity = EntityHelpers::CreateMeshEntity(*m_World, *smeshwo, WorldObjectInstance::Mobility::Dynamic);
					m_World->AddComponent<GCTickable>(entity, GSCreature::Tick, GSCreature::Initialize);
					m_World->AddComponent<GCCreature>(entity, *m_World, *m_PhysicsWorld, *m_NavMesh, *m_PlayerCharacter, *m_AssetLoader);
					m_World->AddComponent<GCSkeletal>(entity);
				}
				continue;
			}
			entt::entity entity = EntityHelpers::CreateMeshEntity(*m_World, *smeshwo, WorldObjectInstance::Mobility::Static);
		}
		else if (const MeshWorldObject* meshwo = dynamic_cast<const MeshWorldObject*>(asset))
		{
			WorldObjectInstance::Mobility mobility;
			float mass = 0;
			if (meshwo->GetPreferredPhysicsState() == MeshWorldObject::PreferredPhysicsState::Dynamic)
			{
				mobility = WorldObjectInstance::Mobility::Dynamic;
				mass = 10;
			}
			else
			{
				mobility = WorldObjectInstance::Mobility::Static;
			}
			entt::entity entity = EntityHelpers::CreateMeshEntityWithPhysics(*m_World, *meshwo, *m_PhysicsWorld, mass, mobility);

			if (asset->GetName().find("Plane001_door_0") == 0)
			{
				m_World->AddComponent<GCTickable>(entity, GSDoor::Tick, GSDoor::Initialize);
				m_World->AddComponent<GCDoor>(entity);
				continue;
			}

			const glm::mat4 transform = meshwo->GetTransform();
			const MeshWorldObject::Mesh& mesh = *meshwo->m_Meshes[0];

			const glm::vec3& bbVertMin = transform * glm::vec4(mesh.m_BoundingBoxVertices[7].m_Position, 1.0f); //Left Bottom Front
			const glm::vec3& bbVertMax = transform * glm::vec4(mesh.m_BoundingBoxVertices[1].m_Position, 1.0f); //Right Top Back

			worldBoundsMin = glm::min(worldBoundsMin, bbVertMin);
			worldBoundsMax = glm::max(worldBoundsMax, bbVertMax);
		}
		else if (const PointlightWorldObject* plwo = dynamic_cast<const PointlightWorldObject*>(asset))
		{
			EntityHelpers::CreatePointlightEntity(*m_World, *plwo, WorldObjectInstance::Mobility::Dynamic);
		}
	}

	GM_INFO("World loaded. Size: {}, {}, {} x {}, {}, {}", worldBoundsMin.x, worldBoundsMin.y, worldBoundsMin.z, worldBoundsMax.x, worldBoundsMax.y, worldBoundsMax.z);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	m_NavMesh->Generate(m_World->GetEntities(Include<GCMesh, GCTransform, GCStaticMobility>{}, Exclude<GCIgnoreForNavMesh>{}));
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - duration;
	GM_INFO("Nav mesh generation done in {} ms.", dur.count());
	GM_INFO("All world instances spawned.");
}

void GanymedeApp::GameTick(Ganymede::WindowTickEvent& event)
{
	GMTime::s_DeltaTime = event.GetFrameDelta();
	GMTime::s_Time = event.GetGameTime();
	m_PhysicsWorld->Step(event.GetFrameDelta());
	m_PlayerCharacter->Tick(event.GetFrameDelta());
	m_World->Tick(event.GetFrameDelta());
	Render();
}

void GanymedeApp::Render()
{
	if (m_RenderContext == nullptr)
	{
		// init render pipeline


	}
	
	m_RenderPipeline->Execute();
}

void GanymedeApp::GameEnd(Ganymede::WindowCloseEvent&)
{
	GM_INFO("Game Ended");
	std::cin.get();
}