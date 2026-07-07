# Ganymede Game Engine — Project Guide

## Critical Rules (READ FIRST)

- **NEVER commit or stage code.** User handles all commits. Highest priority.
- **After adding new .cpp/.h files, run `GenerateProject.bat` before building** — Premake must regenerate `.vcxproj` or the linker won't find them.
- **ALWAYS build after changes:** `msbuild Ganymede.sln /p:Configuration=Debug` (also `Release`/`Retail`). Fix all errors and warnings until clean. Never report "done" without a clean build.
- **No tests exist** — "tested" means "builds cleanly, no errors or warnings."
- **After a clean build, do a second review pass** for correctness, edge cases, and consistency.
- **Write efficient, idiomatic code.** Be critical; replace inefficient or bad-practice solutions with better ones. Anticipate broader context and future needs.
- **Be precise and honest.** Only state things you are highly confident are true. Think carefully and thoroughly before answering. When uncertain, say so explicitly instead of guessing.

## Overview

C++17 game engine, deferred HDR PBR renderer. Mid-refactor: separating engine core (DLL) from client (EXE), abstracting the renderer backend.

Stack: C++17 · MSVC · Premake5 · VS 2022 · Windows · requires `VULKAN_SDK`.

## Layout

```
Ganymede/                     # repo root
├── GenerateProject.bat       # builds vendor libs + generates .sln/.vcxproj
├── Ganymede/src/             # ENGINE CORE → DLL
│   ├── Ganymede.h            #   umbrella header — CLIENT code only
│   └── Ganymede/             #   engine modules (see below)
├── GanymedeApp/              # CLIENT APP → EXE
│   ├── src/GanymedeApp.cpp   #   entry via CreateApplication()
│   └── res/                  #   shaders, models, assets
├── vendor/                   # dependencies (submodules + manual)
└── _bin/  _intermediate/     # build output (gitignored)
```

Modules under `Ganymede/src/Ganymede/`: **Core** (base types, `GANYMEDE_API`, `EntryPoint.h`) · **Common** (Handle system, serialization macros) · **ECS** (`entt`) · **World** (scene graph, octree) · **Graphics** · **Platform** (GLFW windowing) · **Input** · **Events** (`EVENT_BIND_TO_MEMBER`) · **Filesystem** · **Log** (spdlog) · **Physics** (Bullet3) · **AI** (Recast/Detour nav) · **Data** · **System** (`Thread`) · **Player**.

## Architecture

- **Entry:** `Core/EntryPoint.h` calls `CreateApplication()` from the client DLL.
- **Graphics:** `Renderer` interface → Vulkan/OpenGL/Deko3D/DX12 via `GM_ActiveBackend`; API must support all 4.
- **Pipeline:** `RenderPipeline` manages `RenderPass2` objects via `AddRenderPass<T>()`.
- **Factory:** `GraphicsFactory` namespace creates all GPU resources.
- **Serialization:** `bitsery` via `GM_SERIALIZABLE` / `GM_SERIALIZABLE_TEMPLATED`.
- **Handles:** template `Handle<T>`, index-based access.

## Conventions

- **DLL export:** `GANYMEDE_API` on public engine classes; add new public headers to `Ganymede/src/Ganymede.h`.
- **Includes:** specific headers inside engine modules; the `Ganymede.h` umbrella only in `GanymedeApp/`.
- **Naming:** PascalCase classes, `m_` members, `s_` statics. Files: `.h` + `.cpp` (no `.hpp`).
- **Memory:** smart pointers only — no raw pointers, no custom allocators.
- **Errors:** no exceptions — error codes and asserts only.
- **Threading:** `System/Thread.h`, limited use; don't over-parallelize.
- **Logging:** `GM_CORE_TRACE/DEBUG/INFO/WARNING/ERROR/FATAL` and `GM_*` equivalents.
- **Defines:** `GM_PLATFORM_WINDOWS`, `GM_DEBUG`, `GM_RELEASE`, `GM_RETAIL`.
- **Comments:** exclusively English, ASCII-only characters.

## Notes

- Mid-refactor: some features may not build or run.
- Backends: Vulkan is the active priority; OpenGL stable but disabled; Deko3D/DX12 planned.
- `RenderPass2` is current (refactored from `RenderPass`); `Deko3D/` and `System/Runtime/` are empty (planned Switch support).
- Ignore in diffs: `_bin/`, `_intermediate/`, `vendor/*/build/`, `*.sln`, `*.vcxproj`, `.vs/`.