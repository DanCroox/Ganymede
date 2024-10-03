#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;
public:
	VertexBuffer(const float* data, unsigned int count);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};