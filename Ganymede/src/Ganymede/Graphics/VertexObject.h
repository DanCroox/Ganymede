#pragma once

#include "Ganymede/Core/Core.h"

#include "DataBuffer.h"
#include "GPUCommands.h"
#include "VertexDataTypes.h"

namespace Ganymede
{
	class GANYMEDE_API VertexObject
	{
	public:
		VertexObject() = default;
		VertexObject(const unsigned int* indicesData, unsigned int numIndices) {};
		virtual ~VertexObject() {};

		VertexObject(const VertexObject&) = delete;
		VertexObject& operator=(const VertexObject&) = delete;

		VertexObject(VertexObject&& other) noexcept :
			m_CurrentVertexAttribPointer(other.m_CurrentVertexAttribPointer)
		{
			other.m_CurrentVertexAttribPointer = 0;
		}

		VertexObject& operator=(VertexObject&& other) noexcept
		{
			if (this != &other)
			{
				m_CurrentVertexAttribPointer = other.m_CurrentVertexAttribPointer;
				other.m_CurrentVertexAttribPointer = 0;
			}
			return *this;
		}

		virtual unsigned int GetRenderID() const = 0;
		virtual bool IsValid() const = 0;

		template <typename T>
		void LinkBuffer(DataBuffer<T>& dataBuffer, bool isMultiInstanceDataBuffer = false)
		{
			const std::vector<VertexDataPrimitiveTypeInfo>& typeInfos = dataBuffer.GetVertexDataPrimitiveTypeInfo();
			GPUCommands::Rendering::BindVertexObject(*this);
			dataBuffer.Bind();

			for (const auto& typeInfo : typeInfos)
			{
				AddVertexAttribPointer(typeInfo.m_NumComponents, typeInfo.m_PrimitiveType, dataBuffer.GetElementSize(), typeInfo.m_ByteOffset, isMultiInstanceDataBuffer ? 1 : 0);
			}

			dataBuffer.UnBind();
		}

		// Sometimes you want to link a buffer which has a different lifetime than this buffer.
		// If you link and own it, the linked buffer will be destructed automatically once this buffer dies.
		// No need for manual lifetime adjustment.
		template <typename T>
		void LinkAndOwnBuffer(std::unique_ptr<T> dataBufferPtr, bool isMultiInstanceDataBuffer = false)
		{
			const std::vector<VertexDataPrimitiveTypeInfo>& typeInfos = dataBufferPtr->GetVertexDataPrimitiveTypeInfo();
			GPUCommands::Rendering::BindVertexObject(*this);
			dataBufferPtr->Bind();

			for (const auto& typeInfo : typeInfos)
			{
				AddVertexAttribPointer(typeInfo.m_NumComponents, typeInfo.m_PrimitiveType, dataBufferPtr->GetElementSize(), typeInfo.m_ByteOffset, isMultiInstanceDataBuffer ? 1 : 0);
			}

			dataBufferPtr->UnBind();

			m_LinkedBuffers.push_back(std::move(dataBufferPtr));
		}

	protected:
		virtual void AddVertexAttribPointer(unsigned int numComponents, VertexDataPrimitiveType primitiveType, unsigned int stride, unsigned int byteOffset, unsigned int divisor) = 0;

		unsigned int m_CurrentVertexAttribPointer = 0;
		std::vector<std::unique_ptr<DataBufferBase>> m_LinkedBuffers;
	};
}