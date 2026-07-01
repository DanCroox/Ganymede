#include "OGLSSBO.h"

#include "OGLContext.h"
#include <GL/glew.h>

namespace Ganymede
{
	OGLSSBO::OGLSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize) :
		SSBO(bindingPointID, bufferSize, autoResize)
	{
		CreateBuffer(bufferSize);
		MapBuffer();
	}

	OGLSSBO::~OGLSSBO()
	{
		DeleteBuffer();
	}

	OGLSSBO::OGLSSBO(OGLSSBO&& other) noexcept :
		SSBO(std::move(other)),
		m_DirectAccessBuffer(other.m_DirectAccessBuffer),
		m_RenderID(other.m_RenderID)
	{
		other.m_DirectAccessBuffer = 0;
		other.m_RenderID = 0;
	}

	OGLSSBO& OGLSSBO::operator=(OGLSSBO&& other) noexcept
	{
		if (this != &other)
		{
			SSBO::operator=(std::move(other));
			m_DirectAccessBuffer = other.m_DirectAccessBuffer;
			m_RenderID = other.m_RenderID;
			other.m_DirectAccessBuffer = 0;
			other.m_RenderID = 0;
		}
		return *this;
	}

	void OGLSSBO::Barrier()
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void OGLSSBO::CreateBuffer(size_t bufferSize)
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


	void OGLSSBO::MapBuffer()
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

	void OGLSSBO::UnmapBuffer()
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

	void OGLSSBO::DeleteBuffer()
	{
		GM_CORE_ASSERT(m_RenderID != 0, "Invalid handle.");

		UnmapBuffer();
		glDeleteBuffers(1, &m_RenderID);
	}

	void OGLSSBO::ResizeBuffer(size_t newSize)
	{
		const unsigned int oldBufferRenderID = m_RenderID;
		const size_t oldBufferSize = m_BufferSize;
		UnmapBuffer();
		CreateBuffer(newSize);
		glCopyNamedBufferSubData(oldBufferRenderID, m_RenderID, 0, 0, oldBufferSize);
		glDeleteBuffers(1, &oldBufferRenderID);
		MapBuffer();
	}

	void OGLSSBO::Write(unsigned int offset, unsigned int byteCount, void* data)
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

	void OGLSSBO::Read(unsigned int offset, unsigned int byteCount, void* dataOut)
	{
		glGetNamedBufferSubData(m_RenderID, offset, byteCount, dataOut);
	}
}