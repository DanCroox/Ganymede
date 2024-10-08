#pragma once

// Single header for all Ganymede functions. Needs to be included by Ganymede-applications.

#include <Ganymede/AI/NavMesh.h>
#include <Ganymede/Core/Application.h>
#include <Ganymede/Data/AssetLoader.h>
#include <Ganymede/Events/Event.h>
#include <Ganymede/Graphics/Renderer.h>
#include <Ganymede/Graphics/WorldPartition.h>
#include <Ganymede/Log/Log.h>
#include <Ganymede/Physics/PhysicsWorld.h>
#include <Ganymede/Platform/Window.h>
#include <Ganymede/Platform/Window.h>
#include <Ganymede/Player/FPSCamera.h>
#include <Ganymede/Player/PlayerCharacter.h>
#include <Ganymede/Runtime/WindowEvents.h>
#include <Ganymede/System/Thread.h>
#include <Ganymede/World/MeshWorldObject.h>
#include <Ganymede/World/MeshWorldObjectInstance.h>
#include <Ganymede/World/SkeletalMeshWorldObjectInstance.h>
#include <Ganymede/World/World.h>
#include <Ganymede/Platform/Input.h>
#include <Ganymede/Input/KeyCodes.h>
#include <Ganymede/Input/MouseButtonCodes.h>

// --------- Application entry point ---------- //
#include "Ganymede/Core/EntryPoint.h"
// ------------------------------------------- //