#pragma once

#include <fstream>
#include <glad/glad.h>

namespace zfw2
{

using Byte = unsigned char;
using GLID = GLuint;

template<typename T>
T read_from_ifs(std::ifstream &ifs)
{
    T val;
    ifs.read(reinterpret_cast<char *>(&val), sizeof(T));
    return val;
}

}
