#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/Shader.h"

#include <glm/glm.hpp>

namespace Ganymede
{
	namespace RenderTargetTypes
	{
		enum class GANYMEDE_API ParameterKey
		{
			MinFilter, MagFilter,
			HorizontalWrap, VerticalWrap
		};

		enum class GANYMEDE_API ParameterValue
		{
			Nearest, Linear, MipMapLinear,
			ClampToEdge, Repeat
		};

		enum class GANYMEDE_API ComponentType
		{
			R,
			RG,
			RGB,
			RGBA,
			Depth
		};

		enum class GANYMEDE_API ChannelDataType
		{
			Float,
			UInt
		};

		enum class GANYMEDE_API ChannelPrecision
		{
			B8,
			B16,
			B32
		};
	}

	class GANYMEDE_API RenderTarget
	{
	public:
		RenderTarget() = delete;
		RenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		virtual ~RenderTarget();

		virtual void Bind();
		virtual void UnBind();

		void SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value);

		inline unsigned int GetRenderID() const { return m_RenderID; }

		inline bool IsValid() const { return m_RenderID != 0; }

	protected:
		unsigned int m_RenderID = 0;

	private:
		glm::uvec2 m_Size;
		RenderTargetTypes::ComponentType m_ComponentType = RenderTargetTypes::ComponentType::RGBA;
		RenderTargetTypes::ChannelDataType m_ChannelDataType = RenderTargetTypes::ChannelDataType::UInt;
		RenderTargetTypes::ChannelPrecision m_ChannelPrecision = RenderTargetTypes::ChannelPrecision::B8;
	};

	class SinglesampleRenderTarget : public RenderTarget
	{
	public:
		using Super = RenderTarget;
		
		SinglesampleRenderTarget() = delete;
		SinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;
	};

	class MultisampleRenderTarget : public RenderTarget
	{
	public:
		using Super = RenderTarget;
		
		MultisampleRenderTarget() = delete;
		MultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;

	private:
		unsigned int m_SampleCount = 1;
	};

	class CubeMapArrayRenderTarget : public RenderTarget
	{
	public:
		using Super = RenderTarget;

		CubeMapArrayRenderTarget() = delete;
		CubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;
	};
}