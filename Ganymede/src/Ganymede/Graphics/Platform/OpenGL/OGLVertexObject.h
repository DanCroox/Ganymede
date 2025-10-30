#pragma once

#include "Ganymede/Graphics/VertexObject.h"

namespace Ganymede
{
	class GANYMEDE_API OGLVertexObjectIndexBuffer
	{
	public:
		OGLVertexObjectIndexBuffer(const unsigned int* indicesData, unsigned int numIndices);
		~OGLVertexObjectIndexBuffer();

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
		OGLVertexObject() = default;
		OGLVertexObject(const unsigned int* indicesData, unsigned int numIndices);
		~OGLVertexObject();

		OGLVertexObject(const OGLVertexObject&) = delete;
		OGLVertexObject& operator=(const OGLVertexObject&) = delete;

		OGLVertexObject(OGLVertexObject&& other) noexcept :
			VertexObject(std::move(other)),
			m_IndexBufferPtr(std::move(other.m_IndexBufferPtr)),
			m_RenderID(other.m_RenderID)
		{
			other.m_RenderID = 0;
		}

		OGLVertexObject& operator=(OGLVertexObject&& other) noexcept
		{
			if (this != &other)
			{
				VertexObject::operator=(std::move(other));
				m_IndexBufferPtr = std::move(other.m_IndexBufferPtr);
				m_RenderID = other.m_RenderID;
				other.m_RenderID = 0;
			}
			return *this;
		}

		unsigned int GetRenderID() const override { return m_RenderID; }
		const OGLVertexObjectIndexBuffer& GetVertexObjectIndexBuffer() const { return *m_IndexBufferPtr; }

		bool IsValid() const override { return m_RenderID != 0; }

	protected:
		void AddVertexAttribPointer(unsigned int numComponents, VertexDataPrimitiveType primitiveType, unsigned int stride, unsigned int byteOffset, unsigned int divisor) override;

		std::unique_ptr<OGLVertexObjectIndexBuffer> m_IndexBufferPtr;
		unsigned int m_RenderID = 0;
	};
}