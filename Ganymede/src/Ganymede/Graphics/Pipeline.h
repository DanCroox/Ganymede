#pragma once

#include <vector>

namespace Ganymede
{
	class FrameBuffer;
	class ShaderBinary;
	struct VertexDataPrimitiveTypeInfo;

	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;
		const FrameBuffer& GetFrameBuffer() const { return m_FrameBuffer; }

	protected:
		Pipeline(
			const ShaderBinary& shaderBinary,
			uint32_t vertexInputDataStride,
			const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints):
			m_FrameBuffer(frameBuffer)
		{}

		Pipeline(
			const ShaderBinary& shaderBinary,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints) :
			m_FrameBuffer(frameBuffer)
		{}

		const FrameBuffer& m_FrameBuffer;
	};
}