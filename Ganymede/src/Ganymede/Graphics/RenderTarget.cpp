#include "RenderTarget.h"

#include "GL/glew.h"

namespace Ganymede
{
	namespace RenderTarget_Private
	{
		static inline int ToNativeInternalFormat(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision)
		{
			GM_CORE_ASSERT(!(dataType == RenderTargetTypes::ChannelDataType::Float && precision == RenderTargetTypes::ChannelPrecision::B8), "8 bit float per channel is not allowed.");
			GM_CORE_ASSERT(!(dataType == RenderTargetTypes::ChannelDataType::UInt && precision == RenderTargetTypes::ChannelPrecision::B32), "32 bit integer per channel is not allowed.");

			if (componentType == RenderTargetTypes::ComponentType::RGBA)
			{
				switch (precision)
				{
					case RenderTargetTypes::ChannelPrecision::B8: return GL_RGBA8;
					case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RGBA16F : GL_RGBA16;
					case RenderTargetTypes::ChannelPrecision::B32: return GL_RGBA32F;
					default:
						GM_CORE_ASSERT(false, "Unsupported channel precision.");
						break;
				}
			}
			else if (componentType == RenderTargetTypes::ComponentType::RGB)
			{
				switch (precision)
				{
				case RenderTargetTypes::ChannelPrecision::B8: return GL_RGB8;
				case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RGB16F : GL_RGB16;
				case RenderTargetTypes::ChannelPrecision::B32: return GL_RGB32F;
				default:
					GM_CORE_ASSERT(false, "Unsupported channel precision.");
					break;
				}
			}
			else if (componentType == RenderTargetTypes::ComponentType::RG)
			{
				switch (precision)
				{
				case RenderTargetTypes::ChannelPrecision::B8: return GL_RG8;
				case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RG16F : GL_RG16;
				case RenderTargetTypes::ChannelPrecision::B32: return GL_RG32F;
				default:
					GM_CORE_ASSERT(false, "Unsupported channel precision.");
					break;
				}
			}
			else if (componentType == RenderTargetTypes::ComponentType::R)
			{
				switch (precision)
				{
				case RenderTargetTypes::ChannelPrecision::B8: return GL_R8;
				case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_R16F : GL_R16;
				case RenderTargetTypes::ChannelPrecision::B32: return GL_R32F;
				default:
					GM_CORE_ASSERT(false, "Unsupported channel precision.");
					break;
				}
			}
			else if (componentType == RenderTargetTypes::ComponentType::Depth)
			{
				GM_CORE_ASSERT(!(precision == RenderTargetTypes::ChannelPrecision::B8), "8 bit depth component are not allowed.");
				GM_CORE_ASSERT(!(precision == RenderTargetTypes::ChannelPrecision::B16 && dataType == RenderTargetTypes::ChannelDataType::Float), "16 bit float depth component are not allowed.");
				switch (precision)
				{
				case RenderTargetTypes::ChannelPrecision::B16: return GL_DEPTH_COMPONENT16;
				case RenderTargetTypes::ChannelPrecision::B32: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT32;
				default:
					GM_CORE_ASSERT(false, "Unsupported channel precision.");
					break;
				}
			}

			GM_CORE_ASSERT(false, "Unsupported component format.");
			return -1;
		}

		static inline int ToNativeChannelCount(RenderTargetTypes::ComponentType componentType)
		{
			switch (componentType)
			{
			case RenderTargetTypes::ComponentType::RGBA : return GL_RGBA;
			case RenderTargetTypes::ComponentType::RGB : return GL_RGB;
			case RenderTargetTypes::ComponentType::RG : return GL_RG;
			case RenderTargetTypes::ComponentType::R : return GL_R;
			case RenderTargetTypes::ComponentType::Depth: return GL_DEPTH_COMPONENT;
				default:
					GM_CORE_ASSERT(false, "Unsupported channel count.");
					return -1;
			}
		}

		static inline int ToNativeDataType(RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision)
		{
			if (dataType == RenderTargetTypes::ChannelDataType::Float)
			{
				return GL_FLOAT;
			}
			else if (dataType == RenderTargetTypes::ChannelDataType::UInt)
			{
				switch (precision)
				{
					case RenderTargetTypes::ChannelPrecision::B8: return GL_UNSIGNED_BYTE;
					case RenderTargetTypes::ChannelPrecision::B16: return GL_UNSIGNED_SHORT;
					case RenderTargetTypes::ChannelPrecision::B32: return GL_UNSIGNED_INT;
					default:
						GM_CORE_ASSERT(false, "Unsupported channel precision for integer type");
						return -1;
				}
			}

			GM_CORE_ASSERT(false, "Unsupported channel data type.");
			return -1;
		}

		static inline int ToNativeParameterKey(RenderTargetTypes::ParameterKey key)
		{
			switch (key)
			{
			case RenderTargetTypes::ParameterKey::MinFilter: return GL_TEXTURE_MIN_FILTER;
			case RenderTargetTypes::ParameterKey::MagFilter: return GL_TEXTURE_MAG_FILTER;
			case RenderTargetTypes::ParameterKey::HorizontalWrap: return GL_TEXTURE_WRAP_S;
			case RenderTargetTypes::ParameterKey::VerticalWrap: return GL_TEXTURE_WRAP_T;
			default:
				GM_CORE_ASSERT(false, "Unsupported parameter key.");
				return -1;
			}
		}

		static inline int ToNativeParameterValue(RenderTargetTypes::ParameterValue value)
		{
			switch (value)
			{
			case RenderTargetTypes::ParameterValue::Nearest: return GL_NEAREST;
			case RenderTargetTypes::ParameterValue::Linear: return GL_LINEAR;
			case RenderTargetTypes::ParameterValue::MipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
			case RenderTargetTypes::ParameterValue::ClampToEdge: return GL_CLAMP_TO_EDGE;
			case RenderTargetTypes::ParameterValue::Repeat: return GL_REPEAT;
			default:
				GM_CORE_ASSERT(false, "Unsupported parameter key.");
				return -1;
			}
		}
	}

	RenderTarget::RenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		m_ComponentType(componentType),
		m_ChannelDataType(dataType),
		m_ChannelPrecision(precision),
		m_Size(size)
	{
		GM_CORE_ASSERT(size.x > 0 && size.y > 0, "Size needs to be at least 1x1 pixels.");

		glGenTextures(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID > 0, "Couldn't create texture.");
	}

	RenderTarget::~RenderTarget()
	{
		glDeleteTextures(1, &m_RenderID);
	}

	void RenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void RenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void RenderTarget::SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value)
	{
		Bind();
		glTexParameteri(GL_TEXTURE_2D, RenderTarget_Private::ToNativeParameterKey(key), RenderTarget_Private::ToNativeParameterValue(value));
		UnBind();
	}

	SinglesampleRenderTarget::SinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size)
	{
		using namespace RenderTarget_Private;

		Bind();
		glTexImage2D(GL_TEXTURE_2D, 0, ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, 0, ToNativeChannelCount(componentType), ToNativeDataType(dataType, precision), NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		UnBind();
	}

	void SinglesampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void SinglesampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	MultisampleRenderTarget::MultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size),
		m_SampleCount(sampleCount)
	{
		GM_CORE_ASSERT(sampleCount >= 2 && sampleCount <= 16, "Number of samples needs to be between 2 and 16.");

		Bind();
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, RenderTarget_Private::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, GL_TRUE);
		UnBind();
	}

	void MultisampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RenderID);
	}

	void MultisampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}

	CubeMapArrayRenderTarget::CubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size)
	{
		Bind();
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, RenderTarget_Private::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, numTextures, 0, RenderTarget_Private::ToNativeChannelCount(componentType), RenderTarget_Private::ToNativeDataType(dataType, precision), NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		UnBind();
	}

	void CubeMapArrayRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_RenderID);
	}

	void CubeMapArrayRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}
}