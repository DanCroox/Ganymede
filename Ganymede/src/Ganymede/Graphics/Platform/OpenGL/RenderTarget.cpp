#include "OGLRenderTarget.h"

namespace Ganymede
{
	RenderTarget::RenderTarget(RenderTarget&& other) noexcept :
		m_Size(other.m_Size),
		m_ComponentType(other.m_ComponentType),
		m_ChannelDataType(other.m_ChannelDataType),
		m_ChannelPrecision(other.m_ChannelPrecision)
	{}

	RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept
	{
		if (this != &other)
		{
			m_Size = other.m_Size;
			m_ComponentType = other.m_ComponentType;
			m_ChannelDataType = other.m_ChannelDataType;
			m_ChannelPrecision = other.m_ChannelPrecision;
		}

		return *this;
	}
}