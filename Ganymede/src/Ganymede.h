#pragma once

// Single header for all Ganymede functions. Needs to be included by Ganymede-applications.

#include <Ganymede/AI/NavMesh.h>
#include <Ganymede/Common/Helpers.h>
#include <Ganymede/Core/Application.h>
#include <Ganymede/Core/TypeOrderedList.h>
#include <Ganymede/Data/AssetLoader.h>
#include <Ganymede/Events/Event.h>
#include <Ganymede/Graphics/CompositeRenderPass.h>
#include <Ganymede/Graphics/PrepareFrameRenderPass.h>
#include <Ganymede/Graphics/DataBuffer.h>
#include <Ganymede/Graphics/FrameBuffer.h>
#include <Ganymede/Graphics/CollectGeometryPass.h>
#include <Ganymede/Graphics/GeometryRenderPass.h>
#include <Ganymede/Graphics/GPUDebugHandler.h>
#include <Ganymede/Graphics/LightingRenderPass.h>
#include <Ganymede/Graphics/RenderContext.h>
#include <Ganymede/Graphics/Renderer.h>
#include <Ganymede/Graphics/Renderer2.h>
#include <Ganymede/Graphics/RenderPass.h>
#include <Ganymede/Graphics/RenderPipeline.h>
#include <Ganymede/Graphics/RenderTarget.h>
#include <Ganymede/Graphics/Shader.h>
#include <Ganymede/Graphics/ShadowMappingRenderPass.h>
#include <Ganymede/Graphics/SSBO.h>
#include <Ganymede/Graphics/VertexObject.h>
#include <Ganymede/Graphics/WorldPartition.h>
#include <Ganymede/Input/KeyCodes.h>
#include <Ganymede/Input/MouseButtonCodes.h>
#include <Ganymede/Log/Log.h>
#include <Ganymede/Physics/PhysicsWorld.h>
#include <Ganymede/Platform/Input.h>
#include <Ganymede/Platform/Window.h>
#include <Ganymede/Platform/Window.h>
#include <Ganymede/Player/FPSCamera.h>
#include <Ganymede/Player/PlayerCharacter.h>
#include <Ganymede/Runtime/GMTime.h>
#include <Ganymede/Runtime/WindowEvents.h>
#include <Ganymede/System/Thread.h>
#include <Ganymede/World/MeshWorldObject.h>
#include <Ganymede/World/MeshWorldObjectInstance.h>
#include <Ganymede/World/SkeletalMeshWorldObjectInstance.h>
#include <Ganymede/World/World.h>

// --------- Application entry point ---------- //
#include "Ganymede/Core/EntryPoint.h"
// ------------------------------------------- //