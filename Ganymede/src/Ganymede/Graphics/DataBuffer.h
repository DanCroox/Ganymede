#pragma once

#include "Ganymede/Core/Core.h"
#include "OGLBindingHelper.h"
#include "VertexDataTypes.h"

namespace Ganymede
{
	enum class GANYMEDE_API DataBufferType
	{
		Dynamic,
		Static
	};

	// TODO: Since there is no abstraction for native render-api calls, we need to deal with it explicitly here.
	// We don't want to expose glew header in this Ganymede header so we create this set of helper functions.
	class GANYMEDE_API DataBufferNativeFunctions
	{
	public:
		static unsigned int GenerateBuffer();
		static void DeleteBuffer(unsigned int renderID);
		static void BindBuffer(unsigned int renderID);
		static void UnBindBuffer();
		static void Write(unsigned int renderID, const void* data, unsigned int numBytes, unsigned int byteOffset);
		static void InitializeBufferData(unsigned int renderID, const void* data, unsigned int numBytes, DataBufferType bufferType);
	};

	class GANYMEDE_API DataBufferBase
	{
	public:
		virtual ~DataBufferBase() { DataBufferNativeFunctions::DeleteBuffer(m_RenderID); }

		void Bind()
		{
			DataBufferNativeFunctions::BindBuffer(m_RenderID);
		}

		void UnBind()
		{
			DataBufferNativeFunctions::UnBindBuffer();
		}

	protected:
		unsigned int m_RenderID;
	};

	template <typename T>
	class GANYMEDE_API DataBuffer : public DataBufferBase
	{
	public:
		DataBuffer() = delete;
		DataBuffer(const DataBuffer<T>&) = delete;
		DataBuffer<T>& operator=(const DataBuffer<T>&) = delete;

		DataBuffer(DataBuffer<T>&& other) noexcept :
			m_RenderID(other.m_RenderID),
			m_BufferType(other.m_BufferType)
		{
			other.m_RenderID = 0;
		}

		DataBuffer<T>& operator=(DataBuffer<T>&& other) noexcept
		{
			if (this != &other)
			{
				m_RenderID = other.m_RenderID;
				m_BufferType = other.m_BufferType;
				other.m_RenderID = 0;
			}
			return *this;
		}

		DataBuffer(T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType) :
			m_BufferType(bufferType)
		{
			m_BufferType = bufferType;
			m_BufferSize = sizeof(T::VertexDataType) * numElements;
			static_assert(std::is_base_of<VertexDataDescriptor, T>::value, "You can only create a DataBuffer with a VertexDataDescriptor derivate.");
			m_RenderID = DataBufferNativeFunctions::GenerateBuffer();
			DataBufferNativeFunctions::InitializeBufferData(m_RenderID, (const void*) data, sizeof(T::VertexDataType) * numElements, bufferType);
		}

		void Write(T::VertexDataType* data, unsigned int numElements, unsigned int offset)
		{
			GM_CORE_ASSERT(m_BufferType == DataBufferType::Dynamic, "Writing data to a statically initialized data buffer is not possible.");
			
			const size_t numBytesRequested = (sizeof(T::VertexDataType) * numElements) + (sizeof(T::VertexDataType) * offset);
			if (numBytesRequested > m_BufferSize)
			{
				DataBufferNativeFunctions::InitializeBufferData(m_RenderID, nullptr, numBytesRequested, m_BufferType);
				m_BufferSize = numBytesRequested;
			}

			DataBufferNativeFunctions::Write(m_RenderID, (const void*)&data[0], sizeof(T::VertexDataType) * numElements, sizeof(T::VertexDataType) * offset);
		}

		const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo()
		{
			return T::GetVertexDataPrimitiveTypeInfo();
		}

		unsigned int GetElementSize() { return sizeof(T::VertexDataType); }

	private:
		DataBufferType m_BufferType;
		size_t m_BufferSize;
	};
}