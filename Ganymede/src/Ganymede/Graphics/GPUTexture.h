#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
    class Texture;

    class GANYMEDE_API GPUTexture
    {
    public:
        virtual ~GPUTexture() = default;

        virtual void Bind(unsigned int slot = 0) const = 0;
        virtual void Unbind() const = 0;

        unsigned int GetWidth() const { return m_Width; };
        unsigned int GetHeight() const { return m_Height; };
        unsigned int GetChannelCount() const { return m_ChannelCount; };
        unsigned int GetBitDepth() const { return m_BitDepth; };

    protected:
        GPUTexture() = delete;
        explicit GPUTexture(const Texture& texture);

        GPUTexture(const GPUTexture&) = delete;
        GPUTexture& operator=(const GPUTexture&) = delete;

        GPUTexture(GPUTexture&& other) noexcept;
        GPUTexture& operator=(GPUTexture&& other) noexcept;

        unsigned int m_Width = 0;
        unsigned int m_Height = 0;
        unsigned int m_ChannelCount = 0;
        unsigned int m_BitDepth = 0;
    };
}