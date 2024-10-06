#include "SSBO.h"

#include <GL/glew.h>

namespace Ganymede
{

	SSBO::SSBO(unsigned int bindingPointID, unsigned int bufferSize) :
		m_BindingPointID(bindingPointID),
		m_BufferSize(bufferSize)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPointID, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
	}

	SSBO::~SSBO()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void SSBO::Write(unsigned int offset, unsigned int byteCount, void* data) const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteCount, data);
	}
}