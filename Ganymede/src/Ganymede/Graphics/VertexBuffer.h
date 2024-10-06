#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API VertexBuffer
	{
	private:
		unsigned int m_RendererID;
	public:
		VertexBuffer(const float* data, unsigned int count);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;
	};
}