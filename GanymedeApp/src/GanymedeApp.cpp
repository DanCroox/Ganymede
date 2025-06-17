#include "GanymedeApp.h"

#include "ECS/Components/GCCreature.h"
#include "ECS/Systems/GSCreature.h"
#include "EntityHelpers.h"
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>

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

int numlights = 0;
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
	m_RenderPipeline->AddRenderPass<ComputePass>();
	m_RenderPipeline->AddRenderPass<UpdateDrawDataPass>();
	m_RenderPipeline->AddRenderPass<GeometryRenderPass>();
	m_RenderPipeline->AddRenderPass<ShadowMappingRenderPass>();
	m_RenderPipeline->AddRenderPass<LightingRenderPass>();
	m_RenderPipeline->AddRenderPass<CompositeRenderPass>();

	m_Camera = std::make_unique<FPSCamera>(m_RenderContext->CreateRenderView({1920, 1080}, 55.0f, 0.01f, 1000.0f, 0));
	m_PlayerCharacter = std::make_unique<PlayerCharacter>(*m_World, *m_PhysicsWorld, *m_Camera);
	
	//m_AssetLoader->LoadFromPath("res/models/ShadowMappingTest.glb");
	m_AssetLoader->LoadFromPath("res/models/animationtest.glb");
	StaticData::Instance = &m_AssetLoader->m_StaticData;
	m_RenderPipeline->Initialize();

	//m_AssetLoader->LoadFromPath("res/models/backroom2.glb");
	GM_INFO("WorldObjects loaded from glb.");

	glm::vec3 worldBoundsMin(Numbers::MAX_FLOAT);
	glm::vec3 worldBoundsMax(Numbers::MIN_FLOAT);

	SCOPED_TIMER("Initializing static world objects");
	for (const auto& smwo : StaticData::Instance->m_SkeletalMeshWorldObjects)
	{
		if (smwo.GetName().find("Matschkopf") == 0)
		{
			for (int i = 0; i < 100; ++i)
			{
				entt::entity entity = EntityHelpers::CreateMeshEntity(*m_World, smwo, WorldObjectInstance::Mobility::Dynamic);
				m_World->AddComponent<GCTickable>(entity, GSCreature::Tick, GSCreature::Initialize);
				m_World->AddComponent<GCCreature>(entity, *m_World, *m_PhysicsWorld, *m_NavMesh, *m_PlayerCharacter, *m_AssetLoader);
				m_World->AddComponent<GCSkeletal>(entity);
			}
			continue;
		}
		else
		{
			EntityHelpers::CreateMeshEntity(*m_World, smwo, WorldObjectInstance::Mobility::Static);
		}
	}

	for (const auto& mwo : StaticData::Instance->m_MeshWorldObjects)
	{
		WorldObjectInstance::Mobility mobility;
		float mass = 0;
		if (mwo.GetPreferredPhysicsState() == MeshWorldObject::PreferredPhysicsState::Dynamic)
		{
			mobility = WorldObjectInstance::Mobility::Dynamic;
			mass = 10;
		}
		else
		{
			mobility = WorldObjectInstance::Mobility::Static;
		}
		entt::entity entity = EntityHelpers::CreateMeshEntityWithPhysics(*m_World, mwo, *m_PhysicsWorld, mass, mobility);

		if (mwo.GetName().find("Plane001_door_0") == 0)
		{
			m_World->AddComponent<GCTickable>(entity, GSDoor::Tick, GSDoor::Initialize);
			m_World->AddComponent<GCDoor>(entity);
			continue;
		}

		const glm::mat4 transform = mwo.GetTransform();
		const MeshWorldObject::Mesh& mesh = m_AssetLoader->m_StaticData.m_Meshes[mwo.m_Meshes[0].GetID()];

		const glm::vec3& bbVertMin = transform * glm::vec4(mesh.m_BoundingBoxVertices[7].m_Position, 1.0f); //Left Bottom Front
		const glm::vec3& bbVertMax = transform * glm::vec4(mesh.m_BoundingBoxVertices[1].m_Position, 1.0f); //Right Top Back

		worldBoundsMin = glm::min(worldBoundsMin, bbVertMin);
		worldBoundsMax = glm::max(worldBoundsMax, bbVertMax);
	}

	for (const auto& plwo : StaticData::Instance->m_PointlightWorldObjects)
	{
		entt::entity entity = EntityHelpers::CreateWorldEntity(*m_World, plwo, WorldObjectInstance::Mobility::Dynamic);
		GCPointlight& gcPointlight = m_World->AddComponent<GCPointlight>(entity);
		gcPointlight.m_Brightness = plwo.GetBrightness();
		gcPointlight.m_Color = plwo.GetColor();
		for (int i = 0; i < 6; ++i)
		{
			RenderView& rv = m_RenderContext->CreateRenderView({ 1024, 1024 }, 90.0f, 0.01, 1000.0, 1);
			rv.SetPosition(glm::vec3(plwo.GetTransform()[3]));
			rv.m_FaceIndex = (numlights * 6) + i;

			switch (i)
			{
				// +X
			case 0:
				rv.SetFrontVector({ 1, 0, 0 });
				rv.SetUpVector({ 0, -1, 0 });
				break;
				// -X
			case 1:
				rv.SetFrontVector({ -1, 0, 0 });
				rv.SetUpVector({ 0, -1, 0 });
				break;
				// +Y
			case 2:
				rv.SetFrontVector({ 0, 1, 0 });
				rv.SetUpVector({ 0, 0, 1 });
				break;
				// -Y
			case 3:
				rv.SetFrontVector({ 0, -1, 0 });
				rv.SetUpVector({ 0, 0, -1 });
				break;
				// +Z
			case 4:
				rv.SetFrontVector({ 0, 0, 1 });
				rv.SetUpVector({ 0, -1, 0 });
				break;
				// -Z
			case 5:
				rv.SetFrontVector({ 0, 0, -1 });
				rv.SetUpVector({ 0, -1, 0 });
				break;
			}

			gcPointlight.m_CubemapViews[i] = &rv;
		}
		++numlights;
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
	GMTime::s_FrameNumber = event.GetFrameIndex();
	m_PhysicsWorld->Step(event.GetFrameDelta());
	m_PlayerCharacter->Tick(event.GetFrameDelta());
	m_World->Tick(event.GetFrameDelta());
	m_RenderPipeline->Execute();
}

void GanymedeApp::GameEnd(Ganymede::WindowCloseEvent&)
{
	GM_INFO("Game Ended");
	std::cin.get();
}