#include "GPUTexture.h"

#include "Texture.h"

namespace Ganymede
{
    GPUTexture::GPUTexture(const Texture& texture) :
        m_Width(texture.GetWidth()),
        m_Height(texture.GetHeight()),
        m_ChannelCount(texture.GetNumChannels()),
        m_BitDepth(texture.GetBitDepth())
    {};

    GPUTexture::GPUTexture(GPUTexture&& other) noexcept
        : m_Width(other.m_Width),
        m_Height(other.m_Height),
        m_ChannelCount(other.m_ChannelCount),
        m_BitDepth(other.m_BitDepth)
    {};

    GPUTexture& GPUTexture::operator=(GPUTexture&& other) noexcept
    {
        if (this != &other)
        {
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_ChannelCount = other.m_ChannelCount;
            m_BitDepth = other.m_BitDepth;
        }

        return *this;
    }
}