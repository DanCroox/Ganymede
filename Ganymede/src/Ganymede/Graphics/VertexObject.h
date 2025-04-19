#pragma once

#include "Ganymede/Core/Core.h"

#include "VertexDataTypes.h"
#include "OGLBindingHelper.h"
#include "DataBuffer.h"

namespace Ganymede
{
	class GANYMEDE_API VertexObjectIndexBuffer
	{
	public:
		VertexObjectIndexBuffer(const unsigned int* indicesData, unsigned int numIndices);
		~VertexObjectIndexBuffer();

		void Bind();
		void UnBind();

		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		unsigned int m_RenderID = 0;
		unsigned int m_NumIndices;
	};

	class GANYMEDE_API VertexObject
	{
	public:
		VertexObject(const unsigned int* indicesData, unsigned int numIndices);
		~VertexObject();

		VertexObject(const VertexObject&) = delete;
		VertexObject& operator=(const VertexObject&) = delete;

		VertexObject(VertexObject&& other) noexcept :
			m_RenderID(other.m_RenderID),
			m_CurrentVertexAttribPointer(other.m_CurrentVertexAttribPointer)
		{
			other.m_RenderID = 0;
		}

		VertexObject& operator=(VertexObject&& other) noexcept
		{
			if (this != &other)
			{
				m_RenderID = other.m_RenderID;
				m_CurrentVertexAttribPointer = other.m_CurrentVertexAttribPointer;
				other.m_RenderID = 0;
			}
			return *this;
		}

		inline unsigned int GetRenderID() const { return m_RenderID; }

		inline bool IsValid() const { return m_RenderID != 0; }

		template <typename T>
		void LinkBuffer(DataBuffer<T>& dataBuffer, bool isMultiInstanceDataBuffer = false)
		{
			const std::vector<VertexDataPrimitiveTypeInfo>& typeInfos = dataBuffer.GetVertexDataPrimitiveTypeInfo();

			OGLBindingHelper::BindVertexArrayObject(m_RenderID);
			dataBuffer.Bind();

			for (const auto& typeInfo : typeInfos)
			{
				AddVertexAttribPointer(typeInfo.m_NumComponents, typeInfo.m_PrimitiveType, dataBuffer.GetElementSize(), typeInfo.m_ByteOffset, isMultiInstanceDataBuffer ? 1 : 0);
			}

			dataBuffer.UnBind();
		}

		inline const VertexObjectIndexBuffer& GetVertexObjectIndexBuffer() const { return *m_IndexBufferPtr; }

	private:
		void AddVertexAttribPointer(unsigned int numComponents, VertexDataPrimitiveType primitiveType, unsigned int stride, unsigned int byteOffset, unsigned int divisor);

		unsigned int m_RenderID;
		bool m_IsBound;
		unsigned int m_CurrentVertexAttribPointer;
		std::unique_ptr<VertexObjectIndexBuffer> m_IndexBufferPtr;
	};
}