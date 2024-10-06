#pragma once

#include "Ganymede/Core/Core.h"

#include <string>

namespace Ganymede
{

	class GANYMEDE_API SSBO
	{
	public:
		SSBO(unsigned int bindingPointID, unsigned int bufferSize);

		~SSBO();

		unsigned int GetBindingPointID() const { return m_BindingPointID; }
		void Write(unsigned int offset, unsigned int byteCount, void* data) const;

	private:
		unsigned int m_RendererID;

		unsigned int m_BufferSize;

		unsigned int m_BindingPointID;
	};
}