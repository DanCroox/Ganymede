#pragma once

#include "Ganymede/Core/Core.h"

#include "SerializerTypes.h"
#include "SerializerTraits.h"
#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <vector>

namespace Ganymede
{
	class GANYMEDE_API Serializer
	{
	public:
		using OutputAdapter = bitsery::OutputBufferAdapter<std::vector<uint8_t>>;
		using InputAdapter = bitsery::InputBufferAdapter<std::vector<uint8_t>>;

		template<typename T>
		static std::vector<uint8_t> Serialize(T& object)
		{
			static_assert(is_serializable<T>::value, "Type must be marked as GM_SERIALIZABLE");
			static_assert(std::is_default_constructible<T>::value, "Type needs a default constructor (can also be private or protected.");

			std::vector<uint8_t> buffer;
			const size_t bytesWritten = bitsery::quickSerialization<OutputAdapter>(buffer, object);
			buffer.resize(bytesWritten);
			return buffer;
		}

		template<typename T>
		static T Deserialize(const std::vector<uint8_t>& buffer)
		{
			static_assert(is_serializable<T>::value, "Type must be marked as GM_SERIALIZABLE");
			static_assert(std::is_default_constructible<T>::value, "Type needs a default constructor (can also be private or protected.");

			T object;
			auto result = bitsery::quickDeserialization<InputAdapter>(
				{ buffer.begin(), buffer.begin() + buffer.size()}, object);

			GM_CORE_ASSERT(result.first == bitsery::ReaderError::NoError && result.second, "Error or wrong data length during deserializing");

			return object;
		}
	};
}