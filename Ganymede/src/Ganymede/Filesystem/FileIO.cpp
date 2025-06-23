#include "FileIO.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Ganymede
{
    bool FileIO::WriteBytesToFile(const std::string& filename, const std::vector<uint8_t>& data)
    {
        std::ofstream out(filename, std::ios::binary);
        if (!out)
        {
            return false;
        }

        out.write(reinterpret_cast<const char*>(data.data()), data.size());

        return out.good();
    }

    std::optional<std::vector<uint8_t>> FileIO::ReadBytesFromFile(const std::string& filename)
    {
        std::ifstream in(filename, std::ios::binary | std::ios::ate);
        if (!in)
        {
            return std::nullopt;
        };

        std::streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!in.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            return std::nullopt;
        }

        return buffer;
    }
}