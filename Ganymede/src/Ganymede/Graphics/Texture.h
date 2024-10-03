#pragma once
#include "Ganymede/Core/Core.h"

#include <string>

class GANYMEDE_API Texture
{
public:
	Texture(const std::string& path);
	Texture(unsigned char* data, int width, int height, unsigned char channelCount);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	unsigned int m_RendererID;
private:
	void PushTextureToGPU(unsigned char* data);

	
	std::string m_FilePath;
	int m_Width, m_Height, m_ChannelCount;
};