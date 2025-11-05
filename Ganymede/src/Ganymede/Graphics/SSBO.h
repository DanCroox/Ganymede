#pragma once

namespace Ganymede
{
	class SSBO
	{
	public:
		virtual ~SSBO() = default;

		virtual void Write(unsigned int offset, unsigned int byteCount, void* data) = 0;
		virtual void Read(unsigned int offset, unsigned int byteCount, void* dataOut) = 0;
		virtual bool IsValid() const = 0;
		virtual void Barrier() = 0;

		size_t GetSize() const { return m_BufferSize; }

	protected:
		SSBO() = delete;

		SSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize) :
			m_BindingPointID(bindingPointID),
			m_BufferSize(bufferSize),
			m_AutoResize(autoResize)
		{}

		SSBO(const SSBO&) = delete;
		SSBO& operator=(const SSBO&) = delete;

		SSBO(SSBO&& other) noexcept = default;
		SSBO& operator=(SSBO&& other) noexcept = default;

		unsigned int m_BindingPointID;
		size_t m_BufferSize;
		bool m_AutoResize;
	};
}