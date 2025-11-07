#include "OGLComputeShader.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include "GL/glew.h"
#include "OGLContext.h"

namespace Ganymede
{
	OGLComputeShader::OGLComputeShader(const ShaderBinary& shaderBinary) : ComputeShader(shaderBinary)
	{
		const ShaderBinary::Binary& programBinary = shaderBinary.m_BinaryContainer[0];

		GM_CORE_ASSERT(programBinary.m_ShaderTypeBits == ShaderBinaryTypeBits::COMPUTE, "Program binary includes graphics shader binary. Must only include compute shader binary.");

		m_RendererID = glCreateProgram();
		GLenum binaryFormat = 0;
		glProgramBinary(m_RendererID,
			programBinary.m_DataFormat,
			programBinary.m_Data.data(),
			programBinary.m_Data.size());

		GLint linked = GL_FALSE;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &linked);
		GM_CORE_ASSERT(linked == GL_TRUE, "Program not linked");
	}

	OGLComputeShader::~OGLComputeShader()
	{
		glDeleteProgram(m_RendererID);
	}

	OGLComputeShader::OGLComputeShader(OGLComputeShader&& other) noexcept :
		ComputeShader(std::move(other)),
		m_RendererID(other.m_RendererID)
	{
		other.m_RendererID = 0;
	}

	OGLComputeShader& OGLComputeShader::operator=(OGLComputeShader&& other) noexcept
	{
		if (this != &other)
		{
			ComputeShader::operator=(std::move(other));
			m_RendererID = other.m_RendererID;
			other.m_RendererID = 0;
		}

		return *this;
	}

	void OGLComputeShader::Bind()
	{
		OGLContext::BindShader(m_RendererID);
	}

	void OGLComputeShader::Unbind()
	{
		OGLContext::UnbindShader(m_RendererID);
	}

	void OGLComputeShader::Dispatch(unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ)
	{
		Bind();
		glDispatchCompute(numWgX, numWgY, numWgZ);
	}
}