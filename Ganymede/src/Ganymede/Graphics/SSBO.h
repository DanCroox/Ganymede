#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API SSBO
	{
	public:
		SSBO(unsigned int bindingPointID, unsigned int bufferSize);
		~SSBO();

		unsigned int GetBindingPointID() const { return m_BindingPointID; }
		void Write(unsigned int offset, unsigned int byteCount, void* data) const;

		inline bool IsValid() const { return m_RenderID != 0; }

	private:
		unsigned int m_RenderID;
		unsigned int m_BufferSize;
		unsigned int m_BindingPointID;
	};
}