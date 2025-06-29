#include "VertexObject.h"

#include "gl/glew.h"

namespace Ganymede
{
	namespace VertexObject_Private
	{
		static inline constexpr const unsigned int p_MaxNumberVertexAttributes = 16;

		static inline int ToNativeComponentType(VertexDataPrimitiveType type)
		{
			switch (type)
			{
			case VertexDataPrimitiveType::Char:
				return GL_BYTE;
			case VertexDataPrimitiveType::UChar:
				return GL_UNSIGNED_BYTE;
			case VertexDataPrimitiveType::Short:
				return GL_SHORT;
			case VertexDataPrimitiveType::UShort:
				return GL_UNSIGNED_SHORT;
			case VertexDataPrimitiveType::Int:
				return GL_INT;
			case VertexDataPrimitiveType::UInt:
				return GL_UNSIGNED_INT;
			case VertexDataPrimitiveType::Float:
				return GL_FLOAT;
			case VertexDataPrimitiveType::Double:
				return GL_DOUBLE;
			default:
				GM_CORE_ASSERT(false, "Unsupported type.");
				return -1;
			}
		}

		static inline unsigned int NumBytesOfPrimitiveType(VertexDataPrimitiveType type)
		{
			switch (type)
			{
			case VertexDataPrimitiveType::Char:
			case VertexDataPrimitiveType::UChar:
				return sizeof(char);
			case VertexDataPrimitiveType::Short:
			case VertexDataPrimitiveType::UShort:
				return sizeof(short);
			case VertexDataPrimitiveType::Int:
			case VertexDataPrimitiveType::UInt:
				return sizeof(int);
			case VertexDataPrimitiveType::Float:
				return sizeof(float);
			case VertexDataPrimitiveType::Double:
				return sizeof(double);
			default:
				GM_CORE_ASSERT(false, "Unsupported type.");
				return 0;
			}
		}
	}

	VertexObjectIndexBuffer::VertexObjectIndexBuffer(const unsigned int* indicesData, unsigned int numIndices) :
		m_NumIndices(numIndices)
	{
		glGenBuffers(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID != 0, "Couldn't create buffer.");
		Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, (void*)indicesData, GL_STATIC_DRAW);
		UnBind();
	}

	VertexObjectIndexBuffer::~VertexObjectIndexBuffer()
	{
		if (m_RenderID != 0)
		{
			glDeleteBuffers(1, &m_RenderID);
		}
	}

	void VertexObjectIndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
	}

	void VertexObjectIndexBuffer::UnBind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void VertexObject::AddVertexAttribPointer(unsigned int numComponents, VertexDataPrimitiveType primitiveType, unsigned int stride, unsigned int byteOffset, unsigned int divisor)
	{
		using namespace VertexObject_Private;

		const bool isFloat = primitiveType == VertexDataPrimitiveType::Float || primitiveType == VertexDataPrimitiveType::Double;

		while (numComponents > 0)
		{
			// We can only have maximum 4 component vertex attributes (like vec4, vec3, vec2 and float/int/byte etc but no mat4 which contains of 16 float components)
			// Therefore we have to slice types like mat4 and bind it to multiple attribtes. This is fine, in shader we can simply use a mat4 which will pick up the correct data.
			unsigned int slicedComponentCount = glm::min(numComponents, 4U);
			numComponents -= slicedComponentCount;
			GM_CORE_ASSERT(m_CurrentVertexAttribPointer < p_MaxNumberVertexAttributes, "Maximum amount of attribute pointer exceeded.");
			glEnableVertexAttribArray(m_CurrentVertexAttribPointer);
			if (isFloat)
			{
				glVertexAttribPointer(m_CurrentVertexAttribPointer, slicedComponentCount, ToNativeComponentType(primitiveType), GL_FALSE, stride, (const void*)byteOffset);
			}
			else
			{
				glVertexAttribIPointer(m_CurrentVertexAttribPointer, slicedComponentCount, ToNativeComponentType(primitiveType), stride, (const void*)byteOffset);
			}

			if (divisor > 0)
			{
				glVertexAttribDivisor(m_CurrentVertexAttribPointer, divisor);
			}
			++m_CurrentVertexAttribPointer;
			byteOffset += (NumBytesOfPrimitiveType(primitiveType) * slicedComponentCount);
		}
	}

	VertexObject::VertexObject(const unsigned int* indicesData, unsigned int numIndices) :
		m_RenderID(0),
		m_CurrentVertexAttribPointer(0)
	{
		glGenVertexArrays(1, &m_RenderID);
		OGLContext::BindVertexArrayObject(m_RenderID);
		m_IndexBufferPtr = std::make_unique<VertexObjectIndexBuffer>(indicesData, numIndices);
		m_IndexBufferPtr->Bind();
		OGLContext::BindVertexArrayObject(0);
		m_IndexBufferPtr->UnBind();
	}

	VertexObject::~VertexObject()
	{
		OGLContext::BindVertexArrayObject(0);
		glDeleteVertexArrays(1, &m_RenderID);
	}
}