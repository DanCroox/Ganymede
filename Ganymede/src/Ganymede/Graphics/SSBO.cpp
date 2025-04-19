#include "SSBO.h"

#include <GL/glew.h>

namespace Ganymede
{
	SSBO::SSBO(unsigned int bindingPointID, unsigned int bufferSize) :
		m_BindingPointID(bindingPointID),
		m_BufferSize(bufferSize)
	{
		glCreateBuffers(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID != 0, "Couldn't create buffer.");
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPointID, m_RenderID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
	}

	SSBO::~SSBO()
	{
		glDeleteBuffers(1, &m_RenderID);
	}

	void SSBO::Write(unsigned int offset, unsigned int byteCount, void* data) const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RenderID);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteCount, data);
	}
}