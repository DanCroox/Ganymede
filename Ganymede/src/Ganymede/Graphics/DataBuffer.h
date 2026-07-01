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

	class GANYMEDE_API DataBufferBase
	{
	public:
		virtual ~DataBufferBase() = default;
		virtual const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo() const = 0;
		virtual unsigned int GetElementSize() const = 0;

	protected:
		DataBufferBase() = default;
	};

	template <typename T>
	class GANYMEDE_API DataBuffer : public DataBufferBase
	{
	public:
		virtual void Write(typename T::VertexDataType* data, unsigned int numElements, unsigned int offset) = 0;

		const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo() const override
		{
			return T::GetVertexDataPrimitiveTypeInfo();
		}

		unsigned int GetElementSize() const override { return sizeof(typename T::VertexDataType); }

	protected:
		DataBuffer() = delete;

		DataBuffer(typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType)
		{
			static_assert(std::is_base_of<VertexDataDescriptor<typename T::VertexDataType>, T>::value, "You can only create a DataBuffer with a VertexDataDescriptor derivate.");
		}

		DataBuffer(const DataBuffer<T>&) = delete;
		DataBuffer<T>& operator=(const DataBuffer<T>&) = delete;

		DataBuffer(DataBuffer<T>&& other) noexcept = default;
		DataBuffer<T>& operator=(DataBuffer<T>&& other) noexcept = default;
	};
}