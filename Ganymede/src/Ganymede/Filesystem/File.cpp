#include "File.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace Ganymede
{
    bool File::WriteBytesToFile(const std::string& filename, const std::vector<uint8_t>& data)
    {
        std::ofstream out(filename, std::ios::binary);
        if (!out)
        {
            return false;
        }

        out.write(reinterpret_cast<const char*>(data.data()), data.size());

        return out.good();
    }

    std::optional<std::vector<uint8_t>> File::ReadBytesFromFile(const std::string& filename)
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

    bool File::EndsWith(std::string_view str, std::string_view suffix)
    {
        return str.size() >= suffix.size()
            && str.substr(str.size() - suffix.size()) == suffix;
    }
}