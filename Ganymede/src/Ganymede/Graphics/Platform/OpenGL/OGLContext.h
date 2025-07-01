#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;
	class Shader;
	class SSBO;
	class VertexObject;

	namespace RenderTargetTypes
	{
		enum class ComponentType;
		enum class ChannelPrecision;
		enum class ChannelDataType;
		enum class ParameterKey;
		enum class ParameterValue;
	}

	class GANYMEDE_API OGLContext
	{
	public:
		enum class FrameBufferTarget
		{
			Read,
			Draw
		};

		static void BindFrameBuffer(const FrameBuffer& frameBuffer);
		static void UnbindFrameBuffer();
		static void BindShader(const Shader& shader);
		static void BindVertexArrayObject(const VertexObject& vo);
		static void BindIndirectDrawBuffer(SSBO& buffer);

		static void UnbindVertexArrayObject();

		static int ToNativeInternalFormat(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision);
		static int ToNativeDataType(RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision);
		static int ToNativeChannelCount(RenderTargetTypes::ComponentType componentType);
		static int ToNativeParameterKey(RenderTargetTypes::ParameterKey key);
		static int ToNativeParameterValue(RenderTargetTypes::ParameterValue value);

	private:
		static unsigned int m_BoundFrameBuffer;
		static unsigned int m_BoundShader;
		static unsigned int m_BoundVertexArrayObject;
		static unsigned int m_BoundIndirectDrawBuffer;
		static glm::u32vec2 m_CurrentViewportDimension;
	};
}