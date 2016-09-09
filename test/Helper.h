/*
 * Helper.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEST_HELPER_H__
#define __LLGL_TEST_HELPER_H__


#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <fstream>

inline std::string ReadFileContent(const std::string& filename)
{
    std::ifstream file(filename);

    std::string content(
        ( std::istreambuf_iterator<char>(file) ),
        ( std::istreambuf_iterator<char>() )
    );

    return content;
}


#endif

