#pragma once

#include "Ganymede/Core/Core.h"
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
		static void Write(const void* data, unsigned int numBytes, unsigned int byteOffset);
		static void InitializeBufferData(const void* data, unsigned int numBytes, DataBufferType bufferType);
	};

	template <typename T>
	class DataBuffer
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
			m_RenderID(0),
			m_IsBound(false),
			m_BufferType(bufferType)
		{
			static_assert(std::is_base_of<VertexDataDescriptor, T>::value, "You can only create a DataBuffer with a VertexDataDescriptor derivate.");
			m_RenderID = DataBufferNativeFunctions::GenerateBuffer();
			Bind();
			DataBufferNativeFunctions::InitializeBufferData((const void*) data, sizeof(T::VertexDataType) * numElements, bufferType);
			UnBind();
		}

		~DataBuffer()
		{
			DataBufferNativeFunctions::DeleteBuffer(m_RenderID);
		}

		void Bind()
		{
			DataBufferNativeFunctions::BindBuffer(m_RenderID);
			m_IsBound = true;
		}

		void UnBind()
		{
			DataBufferNativeFunctions::UnBindBuffer();
			m_IsBound = false;
		}

		inline bool IsBound() const { return m_IsBound; }

		void Write(T::VertexDataType* data, unsigned int numElements, unsigned int offset)
		{
			GM_CORE_ASSERT(m_BufferType == DataBufferType::Dynamic, "Writing data to a statically initialized data buffer is not possible.");
			Bind();
			DataBufferNativeFunctions::Write((const void*)&data[0], sizeof(T::VertexDataType) * numElements, offset);
			UnBind();
		}

		const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo()
		{
			return T::GetVertexDataPrimitiveTypeInfo();
		}

		unsigned int GetElementSize() { return sizeof(T::VertexDataType); }

	private:
		unsigned int m_RenderID;
		bool m_IsBound;
		DataBufferType m_BufferType;
	};
}