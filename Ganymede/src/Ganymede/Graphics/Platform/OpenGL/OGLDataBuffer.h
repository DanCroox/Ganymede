#pragma once

#include "Ganymede/Graphics/DataBuffer.h"

namespace Ganymede
{
	class OGLDataBufferManager;

	namespace DataBufferNativeFunctions
	{
		unsigned int GenerateBuffer();
		void DeleteBuffer(unsigned int renderID);
		void BindBuffer(unsigned int renderID);
		void UnBindBuffer();
		void Write(unsigned int renderID, const void* data, unsigned int numBytes, unsigned int byteOffset);
		void InitializeBufferData(unsigned int renderID, const void* data, unsigned int numBytes, DataBufferType bufferType);
	};

	class GANYMEDE_API OGLDataBufferManager
	{
	public:
		virtual ~OGLDataBufferManager() = default;
		void Bind() { DataBufferNativeFunctions::BindBuffer(m_RenderID); }
		void Unbind() { DataBufferNativeFunctions::UnBindBuffer(); }

	protected:
		OGLDataBufferManager() = default;

		OGLDataBufferManager(const OGLDataBufferManager&) = delete;
		OGLDataBufferManager& operator=(const OGLDataBufferManager&) = delete;

		OGLDataBufferManager(OGLDataBufferManager&& other) noexcept;
		OGLDataBufferManager& operator=(OGLDataBufferManager&& other) noexcept;

		unsigned int m_RenderID;
	};

	template <typename T>
	class GANYMEDE_API OGLDataBuffer : public DataBuffer<T>, public OGLDataBufferManager
	{
	public:
		OGLDataBuffer(typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType) :
			DataBuffer<T>(data, numElements, bufferType),
			m_BufferType(bufferType)
		{
			m_BufferSize = sizeof(typename T::VertexDataType) * numElements;
			static_assert(std::is_base_of<VertexDataDescriptor<typename T::VertexDataType>, T>::value, "You can only create a DataBuffer with a VertexDataDescriptor derivate.");
			m_RenderID = DataBufferNativeFunctions::GenerateBuffer();
			DataBufferNativeFunctions::InitializeBufferData(m_RenderID, (const void*)data, sizeof(typename T::VertexDataType) * numElements, bufferType);
		}

		OGLDataBuffer(OGLDataBuffer<T>&& other) noexcept :
			OGLDataBufferManager(std::move(other)),
			m_BufferType(other.m_BufferType),
			m_BufferSize(other.m_BufferSize)
		{
			other.m_RenderID = 0;
		}

		OGLDataBuffer<T>& operator=(OGLDataBuffer<T>&& other) noexcept
		{
			if (this != &other)
			{
				OGLDataBufferManager::operator=(std::move(other));
				m_RenderID = other.m_RenderID;
				m_BufferType = other.m_BufferType;
				m_BufferSize = other.m_BufferSize;
				other.m_RenderID = 0;
			}
			return *this;
		}

		void Write(typename T::VertexDataType* data, unsigned int numElements, unsigned int offset)
		{
			GM_CORE_ASSERT(m_BufferType == DataBufferType::Dynamic, "Writing data to a statically initialized data buffer is not possible.");

			const size_t numBytesRequested = (sizeof(typename T::VertexDataType) * numElements) + (sizeof(typename T::VertexDataType) * offset);
			if (numBytesRequested > m_BufferSize)
			{
				DataBufferNativeFunctions::InitializeBufferData(m_RenderID, nullptr, numBytesRequested, m_BufferType);
				m_BufferSize = numBytesRequested;
			}

			DataBufferNativeFunctions::Write(m_RenderID, (const void*)&data[0], sizeof(typename T::VertexDataType) * numElements, sizeof(typename T::VertexDataType) * offset);
		}

	private:
		DataBufferType m_BufferType;
		size_t m_BufferSize;
	};
}