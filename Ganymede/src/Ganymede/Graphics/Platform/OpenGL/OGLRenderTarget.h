#pragma once

#include "Ganymede/Graphics/RenderTarget.h"

namespace Ganymede
{
	class GANYMEDE_API OGLRenderTarget : public RenderTarget
	{
	public:
		OGLRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		~OGLRenderTarget() override;

		OGLRenderTarget(OGLRenderTarget&&) noexcept;
		OGLRenderTarget& operator=(OGLRenderTarget&&) noexcept;

		virtual void Bind();
		virtual void UnBind();

		void Clear(
			unsigned int mipLayer,
			unsigned int destX,
			unsigned int destY,
			unsigned int destDepth,
			unsigned int extendX,
			unsigned int extendY,
			unsigned int extendDepth,
			const float* normalizedClearColor) override;

		void SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value) override;
		
		bool IsValid() const override { return m_RenderID != 0; }

		unsigned int GetRenderID() const { return m_RenderID; }

	protected:
		unsigned int m_RenderID = 0;
	};

	class OGLSinglesampleRenderTarget : public OGLRenderTarget
	{
	public:
		OGLSinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;
	};

	class OGLMultisampleRenderTarget : public OGLRenderTarget
	{
	public:
		OGLMultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;
	};

	class OGLCubeMapArrayRenderTarget : public OGLRenderTarget
	{
	public:
		OGLCubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);

		void Bind() override;
		void UnBind() override;
	};
}