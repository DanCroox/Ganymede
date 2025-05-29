#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API SSBO
	{
	public:
		SSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		~SSBO();

		unsigned int GetBindingPointID() const { return m_BindingPointID; }
		void Write(unsigned int offset, unsigned int byteCount, void* data);

		void Read(unsigned int offset, unsigned int byteCount, void* dataOut);

		inline bool IsValid() const { return m_RenderID != 0; }

		size_t GetSize() const { return m_BufferSize; }

		char* m_DirectAccessBuffer = nullptr;
		unsigned int m_RenderID;
	private:
		void CreateBuffer(size_t bufferSize);
		void MapBuffer();
		void UnmapBuffer();
		void DeleteBuffer();
		void ResizeBuffer(size_t newSize);


		size_t m_BufferSize;
		unsigned int m_BindingPointID;
		bool m_AutoResize;

	};
}