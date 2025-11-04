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
		RenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
			m_ComponentType(componentType),
			m_ChannelDataType(dataType),
			m_ChannelPrecision(precision),
			m_Size(size) {};

		virtual ~RenderTarget() = default;

		virtual void SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value) = 0;
		virtual bool IsValid() const = 0;

		RenderTargetTypes::ComponentType GetComponentType() const { return m_ComponentType; }
		RenderTargetTypes::ChannelDataType GetChannelDataType() const { return m_ChannelDataType; }
		RenderTargetTypes::ChannelPrecision GetChannelPrecision() const { return m_ChannelPrecision; }

	protected:
		RenderTarget() = delete;

		RenderTarget(const RenderTarget&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;

		RenderTarget(RenderTarget&&) noexcept;
		RenderTarget& operator=(RenderTarget&&) noexcept;

		glm::uvec2 m_Size;
		RenderTargetTypes::ComponentType m_ComponentType = RenderTargetTypes::ComponentType::RGBA;
		RenderTargetTypes::ChannelDataType m_ChannelDataType = RenderTargetTypes::ChannelDataType::UInt;
		RenderTargetTypes::ChannelPrecision m_ChannelPrecision = RenderTargetTypes::ChannelPrecision::B8;
	};
}