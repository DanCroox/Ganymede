#pragma once

#include "Ganymede/Graphics/VertexObject.h"

namespace Ganymede
{
	class GANYMEDE_API OGLVertexObjectIndexBuffer
	{
	public:
		OGLVertexObjectIndexBuffer() = delete;

		OGLVertexObjectIndexBuffer(const unsigned int* indicesData, unsigned int numIndices);
		~OGLVertexObjectIndexBuffer();

		OGLVertexObjectIndexBuffer(const OGLVertexObjectIndexBuffer&) = delete;
		OGLVertexObjectIndexBuffer& operator=(const OGLVertexObjectIndexBuffer&) = delete;

		OGLVertexObjectIndexBuffer(OGLVertexObjectIndexBuffer&& other) noexcept;
		OGLVertexObjectIndexBuffer& operator=(OGLVertexObjectIndexBuffer&& other) noexcept;

		void Bind();
		void UnBind();

		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		unsigned int m_RenderID = 0;
		unsigned int m_NumIndices;
	};

	class GANYMEDE_API OGLVertexObject : public VertexObject
	{
	public:
		OGLVertexObject(const unsigned int* indicesData, unsigned int numIndices);
		~OGLVertexObject() override;

		OGLVertexObject(OGLVertexObject&& other) noexcept;
		OGLVertexObject& operator=(OGLVertexObject&& other) noexcept;

		bool IsValid() const override { return m_RenderID != 0; }

		void LinkBuffer(DataBufferBase& dataBuffer, bool isMultiInstanceDataBuffer) override;
		void LinkAndOwnBuffer(std::unique_ptr<DataBufferBase> dataBufferPtr, bool isMultiInstanceDataBuffer) override;

		unsigned int GetRenderID() const { return m_RenderID; }
		const OGLVertexObjectIndexBuffer& GetVertexObjectIndexBuffer() const { return *m_IndexBufferPtr; }

	private:
		void AddVertexAttribPointer(unsigned int numComponents, VertexDataPrimitiveType primitiveType, unsigned int stride, unsigned int byteOffset, unsigned int divisor);

		unsigned int m_RenderID = 0;
		unsigned int m_CurrentVertexAttribPointer = 0;
		std::unique_ptr<OGLVertexObjectIndexBuffer> m_IndexBufferPtr;
		std::vector<std::unique_ptr<DataBufferBase>> m_LinkedBuffers;
	};
}