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
		virtual ~VertexObject() = default;

		virtual bool IsValid() const = 0;

		virtual void LinkBuffer(DataBufferBase& dataBuffer, bool isMultiInstanceDataBuffer = false) = 0;
		virtual void LinkAndOwnBuffer(std::unique_ptr<DataBufferBase> dataBufferPtr, bool isMultiInstanceDataBuffer = false) = 0;

	protected:
		VertexObject() = delete;
		VertexObject(const unsigned int* indicesData, unsigned int numIndices) {};

		VertexObject(const VertexObject&) = delete;
		VertexObject& operator=(const VertexObject&) = delete;

		VertexObject(VertexObject&& other) noexcept = default;
		VertexObject& operator=(VertexObject&& other) noexcept = default;
	};
}