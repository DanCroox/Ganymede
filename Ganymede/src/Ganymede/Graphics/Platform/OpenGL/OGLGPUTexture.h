#pragma once

#include "Ganymede/Graphics/GPUTexture.h"

namespace Ganymede
{
    class GANYMEDE_API OGLGPUTexture : public GPUTexture
    {
    public:
        explicit OGLGPUTexture(const Texture& texHandle);
        ~OGLGPUTexture() override;

        OGLGPUTexture(OGLGPUTexture&&) noexcept;
        OGLGPUTexture& operator=(OGLGPUTexture&&) noexcept;

        void Bind(unsigned int slot = 0) const override;
        void Unbind() const override;

    private:
        unsigned int m_RendererID = 0;
    };
}