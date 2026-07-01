#include "ShaderCompiler.h"

#include <fstream>
#include <sstream>

namespace Ganymede
{
	bool ShaderCompiler::Compile(const std::string& filePath, std::vector<ShaderBinary::Binary>& binaryDataOut)
	{
		ProgramSource source = ParseShader(filePath);
		const bool compilationSuccess = CompileProgram(source, binaryDataOut);

		GM_CORE_ASSERT(compilationSuccess, "Shader compilation failed");

		return compilationSuccess;
	}

	ShaderCompiler::ProgramSource ShaderCompiler::ParseShader(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		GM_CORE_ASSERT(stream.good(), "Couldn't load shader source from path.");

		std::string line;
		std::stringstream ss[4];
		uint8_t typeBit = 0;
		while (getline(stream, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("compute") != std::string::npos)
				{
					typeBit = ShaderBinaryTypeBits::COMPUTE;
				}
				else if (line.find("vertex") != std::string::npos)
				{
					typeBit = ShaderBinaryTypeBits::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos)
				{
					typeBit = ShaderBinaryTypeBits::FRAGMENT;
				}
				else if (line.find("geometry") != std::string::npos)
				{
					typeBit = ShaderBinaryTypeBits::GEOMETRY;
				}
			}
			else
			{
				uint8_t mask = typeBit;
				int index = 0;
				while (mask >>= 1) ++index;

				std::stringstream& outBuffer = ss[index];
				if (line.find("#include") != std::string::npos)
				{
					ParseIncludeHierarchy(filepath, line, outBuffer);
				}
				else
				{
					outBuffer << line << "\n";
				}
			}
		}

		return {
#ifndef GM_RETAIL
			filepath,
#endif //GM_RETAIL
			ss[0].str(),
			ss[1].str(),
			ss[2].str(),
			ss[3].str()
		};
	}

	void ShaderCompiler::ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut)
	{
		// Parse #include line to find given file and create relative file path
		int selectionStart = line.find("\"");
		int selectionEnd = line.rfind("\"");
		if (selectionStart != std::string::npos && selectionEnd != std::string::npos)
		{
			++selectionStart; //remove the search terms itself
			selectionEnd;
			const std::string includeFile = line.substr(selectionStart, selectionEnd - selectionStart);
			selectionEnd = filepath.rfind("/");
			std::string fileDirectory = filepath.substr(0, selectionEnd + 1);
			const std::string finalIncludeFilePath = fileDirectory + includeFile;

			//read include file
			std::ifstream stream(finalIncludeFilePath);
			if (!stream)
			{
				GM_CORE_ASSERT(false, "Shader include file does not exist!");
			}

			std::string includeLine = line;
			while (std::getline(stream, includeLine))
			{
				if (includeLine.find("#include") != std::string::npos)
				{
					// Recursively parse include files
					// TODO: Detect circular dependencies and ignore multipe includes of same file
					ParseIncludeHierarchy(filepath, includeLine, stringOut);
				}
				else
				{
					stringOut << includeLine << "\n";
				}
			}
		}
	}
}