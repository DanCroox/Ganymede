#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/SSBO.h"

namespace Ganymede
{
	class GANYMEDE_API OGLSSBO : public SSBO
	{
	public:
		OGLSSBO() = delete;
		OGLSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		~OGLSSBO();

		void Write(unsigned int offset, unsigned int byteCount, void* data) override;
		void Read(unsigned int offset, unsigned int byteCount, void* dataOut) override;
		bool IsValid() const override { return m_RenderID != 0; }
		void Barrier() override;

		unsigned int GetRenderID() const { return m_RenderID; }

	private:
		void CreateBuffer(size_t bufferSize);
		void MapBuffer();
		void UnmapBuffer();
		void DeleteBuffer();
		void ResizeBuffer(size_t newSize);

		char* m_DirectAccessBuffer = nullptr;
		unsigned int m_RenderID;
	};
}