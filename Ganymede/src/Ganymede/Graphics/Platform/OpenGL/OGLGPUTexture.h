#pragma once

#include "Ganymede/Graphics/GPUTexture.h"

namespace Ganymede
{
    class GANYMEDE_API OGLGPUTexture : public GPUTexture
    {
    public:
        explicit OGLGPUTexture(const Texture& texHandle);
        ~OGLGPUTexture() override;

        virtual void Bind(unsigned int slot = 0) const override;
        virtual void Unbind() const override;

    protected:
        OGLGPUTexture(OGLGPUTexture&&) noexcept;
        OGLGPUTexture& operator=(OGLGPUTexture&&) noexcept;

    private:
        unsigned int m_RendererID = 0;
    };
}