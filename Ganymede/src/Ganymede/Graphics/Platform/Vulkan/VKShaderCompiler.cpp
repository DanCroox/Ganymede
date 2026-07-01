#include "VKShaderCompiler.h"

#include "VKContext.h"
#include <shaderc/shaderc.hpp>
#include <vector>
#include <string>
#include <stdexcept>

namespace Ganymede
{
	namespace
	{
		bool CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& debugName, std::vector<uint8_t>& dataOut)
		{
			VKContext& vkContext = VKContext::GetInstance();
			uint32_t apiVersion = shaderc_env_version_vulkan_1_0;
			switch (vkContext.GetApiVersion())
			{
			case VK_API_VERSION_1_0:
				apiVersion = shaderc_env_version_vulkan_1_0;
				break;
			case VK_API_VERSION_1_1:
				apiVersion = shaderc_env_version_vulkan_1_1;
				break;
			case VK_API_VERSION_1_2:
				apiVersion = shaderc_env_version_vulkan_1_2;
				break;
			case VK_API_VERSION_1_3:
				apiVersion = shaderc_env_version_vulkan_1_3;
				break;
			case VK_API_VERSION_1_4:
				apiVersion = shaderc_env_version_vulkan_1_4;
				break;
			default:
				GM_CORE_ASSERT(false, "Unsupported Vulkan API version.");
				return false;
			}

			vkContext.GetApiVersion();
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			options.SetOptimizationLevel(shaderc_optimization_level_performance);
			options.SetTargetEnvironment(shaderc_target_env_vulkan, apiVersion);
			options.SetSourceLanguage(shaderc_source_language_glsl);

			shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, debugName.c_str(), options);

			if (result.GetCompilationStatus() == shaderc_compilation_status_success)
			{
				dataOut.insert(
					dataOut.end(),
					reinterpret_cast<const uint8_t*>(result.cbegin()),
					reinterpret_cast<const uint8_t*>(result.cend())
				);
				return true;
			}

			GM_CORE_ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success, result.GetErrorMessage());
			return false;
		}
	}

	bool VKShaderCompiler::CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut)
	{
		const bool hasCS = programSource.ComputeSource.size() > 0;
		const bool hasVS = programSource.VertexSource.size() > 0;
		const bool hasFS = programSource.FragmentSource.size() > 0;
		const bool hasGS = programSource.GeometrySource.size() > 0;

		if (!hasCS && !hasVS)
		{
			GM_CORE_ASSERT(false, "No vertex shader provided. All shaders except compute require a vertex shader.");
			return false;
		}

		if (!hasCS && !hasVS && !hasFS && !hasGS)
		{
			GM_CORE_ASSERT(false, "Nothing to compile.");
			return false;
		}

		std::string shaderName = "";
		
		if (hasCS)
		{
#ifndef GM_RETAIL
			shaderName = programSource.m_Name + ".compute";
#endif //GM_RETAIL
			ShaderBinary::Binary& binary = binaryDataOut.emplace_back();
			binary.m_DataFormat = 0;
			binary.m_ShaderTypeBits = ShaderBinaryTypeBits::COMPUTE;
			if (!CompileGLSL(programSource.ComputeSource, shaderc_shader_kind::shaderc_glsl_compute_shader, shaderName, binary.m_Data))
			{
				return false;
			}
		}

		if (hasVS)
		{
#ifndef GM_RETAIL
			shaderName = programSource.m_Name + ".vertex";
#endif //GM_RETAIL
			ShaderBinary::Binary& binary = binaryDataOut.emplace_back();
			binary.m_DataFormat = 0;
			binary.m_ShaderTypeBits = ShaderBinaryTypeBits::VERTEX;
			if (!CompileGLSL(programSource.VertexSource, shaderc_shader_kind::shaderc_glsl_vertex_shader, shaderName, binary.m_Data))
			{
				return false;
			}
		}

		if (hasFS)
		{
#ifndef GM_RETAIL
			shaderName = programSource.m_Name + ".fragment";
#endif //GM_RETAIL
			ShaderBinary::Binary& binary = binaryDataOut.emplace_back();
			binary.m_DataFormat = 0;
			binary.m_ShaderTypeBits = ShaderBinaryTypeBits::FRAGMENT;
			if (!CompileGLSL(programSource.FragmentSource, shaderc_shader_kind::shaderc_glsl_fragment_shader, shaderName, binary.m_Data))
			{
				return false;
			}
		}

		if (hasGS)
		{
#ifndef GM_RETAIL
			shaderName = programSource.m_Name + ".geometry";
#endif //GM_RETAIL
			ShaderBinary::Binary& binary = binaryDataOut.emplace_back();
			binary.m_DataFormat = 0;
			binary.m_ShaderTypeBits = ShaderBinaryTypeBits::GEOMETRY;
			if (CompileGLSL(programSource.GeometrySource, shaderc_shader_kind::shaderc_glsl_geometry_shader, shaderName, binary.m_Data))
			{
				return false;
			}
		}

		return true;
	}
}