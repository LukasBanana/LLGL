/*
 * Helper.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEST_HELPER_H
#define LLGL_TEST_HELPER_H


#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <fstream>


inline std::string ReadFileContent(const std::string& filename)
{
    std::ifstream file { filename };

    if (!file.good())
        throw std::runtime_error("failed to open file: \"" + filename + "\"");

    return std::string
    {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
}


#endif

