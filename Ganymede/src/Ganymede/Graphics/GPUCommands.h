#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class RenderTarget;
	class Shader;
	class VertexObject;

	namespace GPUCommands
	{
		namespace Rendering
		{
			void BindShader(const Shader& shader);
			void BindVertexObject(const VertexObject& vo);
		}

		namespace Compute
		{
			GANYMEDE_API void Dispatch(Shader& shader, unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ);
		}

		namespace RenderTarget
		{
			GANYMEDE_API void ClearRenderTarget(
				Ganymede::RenderTarget& renderTarget,
				unsigned int mipLayer,
				unsigned int destX,
				unsigned int destY,
				unsigned int destDepth,
				unsigned int extendX,
				unsigned int extendY,
				unsigned int extendDepth,
				const void* pixelDataBytes);
		}
	};
}