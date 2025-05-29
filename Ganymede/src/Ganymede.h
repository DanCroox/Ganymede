#pragma once

// Single header for all Ganymede functions. Needs to be included by Ganymede-applications.

#include <Ganymede/AI/NavMesh.h>
#include <Ganymede/Common/Helpers.h>
#include <Ganymede/Core/Application.h>
#include <Ganymede/Data/AssetLoader.h>
#include <Ganymede/ECS/Components/GCCreature.h>
#include <Ganymede/ECS/Components/GCDoor.h>
#include <Ganymede/ECS/Components/GCDynamicMobility.h>
#include <Ganymede/ECS/Components/GCEntityID.h>
#include <Ganymede/ECS/Components/GCIgnoreForNavMesh.h>
#include <Ganymede/ECS/Components/GCMesh.h>
#include <Ganymede/ECS/Components/GCName.h>
#include <Ganymede/ECS/Components/GCPointlight.h>
#include <Ganymede/ECS/Components/GCRenderObject.h>
#include <Ganymede/ECS/Components/GCRigidBody.h>
#include <Ganymede/ECS/Components/GCSkeletal.h>
#include <Ganymede/ECS/Components/GCStaticMobility.h>
#include <Ganymede/ECS/Components/GCTickable.h>
#include <Ganymede/ECS/Components/GCTransform.h>
#include <Ganymede/ECS/Systems/GSCreature.h>
#include <Ganymede/ECS/Systems/GSDoor.h>
#include <Ganymede/Events/Event.h>
#include <Ganymede/Graphics/CullingSystem.h>
#include <Ganymede/Graphics/DataBuffer.h>
#include <Ganymede/Graphics/FrameBuffer.h>
#include <Ganymede/Graphics/GPUDebugHandler.h>
#include <Ganymede/Graphics/RenderContext.h>
#include <Ganymede/Graphics/Renderer.h>
#include <Ganymede/Graphics/RenderPass.h>
#include <Ganymede/Graphics/RenderPasses/CollectGeometryPass.h>
#include <Ganymede/Graphics/RenderPasses/CompositeRenderPass.h>
#include <Ganymede/Graphics/RenderPasses/GeometryRenderPass.h>
#include <Ganymede/Graphics/RenderPasses/LightingRenderPass.h>
#include <Ganymede/Graphics/RenderPasses/PrepareFrameRenderPass.h>
#include <Ganymede/Graphics/RenderPasses/ComputePass.h>
#include <Ganymede/Graphics/RenderPasses/ShadowMappingRenderPass.h>
#include <Ganymede/Graphics/RenderPipeline.h>
#include <Ganymede/Graphics/RenderTarget.h>
#include <Ganymede/Graphics/Shader.h>
#include <Ganymede/Graphics/SSBO.h>
#include <Ganymede/Graphics/VertexObject.h>
#include <Ganymede/Input/KeyCodes.h>
#include <Ganymede/Input/MouseButtonCodes.h>
#include <Ganymede/Log/Log.h>
#include <Ganymede/Physics/PhysicsWorld.h>
#include <Ganymede/Platform/Input.h>
#include <Ganymede/Platform/Window.h>
#include <Ganymede/Player/FPSCamera.h>
#include <Ganymede/Player/PlayerCharacter.h>
#include <Ganymede/Runtime/GMTime.h>
#include <Ganymede/Runtime/WindowEvents.h>
#include <Ganymede/System/Thread.h>
#include <Ganymede/System/Types.h>
#include <Ganymede/World/MeshWorldObject.h>
#include <Ganymede/World/PointlightWorldObject.h>
#include <Ganymede/World/World.h>

// --------- Application entry point ---------- //
#include "Ganymede/Core/EntryPoint.h"
// ------------------------------------------- //