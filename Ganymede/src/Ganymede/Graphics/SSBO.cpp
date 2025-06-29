#include "SSBO.h"
#include "OGLContext.h"

#include <GL/glew.h>

namespace Ganymede
{
	SSBO::SSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize) :
		m_BindingPointID(bindingPointID),
		m_AutoResize(autoResize)
	{
		CreateBuffer(bufferSize);
		MapBuffer();
	}

	SSBO::~SSBO()
	{
		DeleteBuffer();
	}

	void SSBO::Barrier()
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void SSBO::CreateBuffer(size_t bufferSize)
	{
		GLint ssboAlign = 0;
		glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &ssboAlign);
		m_BufferSize = ((bufferSize + ssboAlign - 1) / ssboAlign) * ssboAlign;

		glCreateBuffers(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID != 0, "Couldn't create buffer.");

		glNamedBufferStorage(
			m_RenderID,
			m_BufferSize,
			nullptr,
			GL_MAP_WRITE_BIT
			| GL_MAP_PERSISTENT_BIT
			| GL_MAP_COHERENT_BIT
			| GL_DYNAMIC_STORAGE_BIT
		);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPointID, m_RenderID);
	}


	void SSBO::MapBuffer()
	{
		GM_CORE_ASSERT(m_RenderID != 0, "Invalid handle.");
		GM_CORE_ASSERT(m_DirectAccessBuffer == nullptr, "Buffer already mapped.");

		m_DirectAccessBuffer = static_cast<char*>(glMapNamedBufferRange(
			m_RenderID,
			0,
			m_BufferSize,
			GL_MAP_WRITE_BIT
			| GL_MAP_PERSISTENT_BIT
			| GL_MAP_COHERENT_BIT
		));
	}

	void SSBO::UnmapBuffer()
	{
		GM_CORE_ASSERT(m_RenderID != 0, "Invalid handle.");
		GM_CORE_ASSERT(m_DirectAccessBuffer != nullptr, "Buffer not mapped.");

		if (m_DirectAccessBuffer == nullptr)
		{
			return;
		}

		glUnmapNamedBuffer(m_RenderID);
		m_DirectAccessBuffer = nullptr;
	}

	void SSBO::DeleteBuffer()
	{
		GM_CORE_ASSERT(m_RenderID != 0, "Invalid handle.");

		UnmapBuffer();
		glDeleteBuffers(1, &m_RenderID);
	}

	void SSBO::ResizeBuffer(size_t newSize)
	{
		const unsigned int oldBufferRenderID = m_RenderID;
		const size_t oldBufferSize = m_BufferSize;
		UnmapBuffer();
		CreateBuffer(newSize);
		glCopyNamedBufferSubData(oldBufferRenderID, m_RenderID, 0, 0, oldBufferSize);
		glDeleteBuffers(1, &oldBufferRenderID);
		MapBuffer();
	}

	void SSBO::Write(unsigned int offset, unsigned int byteCount, void* data)
	{
		const size_t numRequestedBytes = offset + byteCount;

		GM_CORE_ASSERT(m_AutoResize || numRequestedBytes <= m_BufferSize, "Exceeded buffer boundry.");

		if (m_AutoResize && (numRequestedBytes > m_BufferSize))
		{
			size_t newSize = std::max(m_BufferSize * 2, numRequestedBytes);
			ResizeBuffer(newSize);
		}

		memcpy(m_DirectAccessBuffer + offset, data, byteCount);
	}

	void SSBO::Read(unsigned int offset, unsigned int byteCount, void* dataOut)
	{
		glGetNamedBufferSubData(m_RenderID, offset, byteCount, dataOut);
	}
}