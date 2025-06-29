#include "GPUCommands.h"

#include "RenderTarget.h"
#include "Shader.h"
#include "OGLContext.h"

#include "GL/glew.h"

namespace Ganymede
{
	namespace GPUCommands
	{
		void Compute::Dispatch(Shader& shader, unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ)
		{
			OGLContext::BindShader(shader.GetRendererID());
			glDispatchCompute(numWgX, numWgY, numWgZ);
		}

		void RenderTarget::ClearRenderTarget(
			Ganymede::RenderTarget& renderTarget,
			unsigned int mipLayer,
			unsigned int destX,
			unsigned int destY,
			unsigned int destDepth,
			unsigned int extendX,
			unsigned int extendY,
			unsigned int extendDepth,
			const void* pixelDataBytes)
		{
			glClearTexSubImage(
				renderTarget.GetRenderID(),
				mipLayer,
				destX, destY,
				destDepth,
				extendX, extendY,
				extendDepth,
				OGLContext::ToNativeChannelCount(renderTarget.GetComponentType()),
				OGLContext::ToNativeDataType(renderTarget.GetChannelDataType(), renderTarget.GetChannelPrecision()),
				pixelDataBytes
			);
		}
	}
}