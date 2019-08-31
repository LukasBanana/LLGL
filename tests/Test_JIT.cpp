/*
 * Test_JIT.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>


namespace LLGL
{
LLGL_EXPORT void TestJIT1();
}


int main()
{
    try
    {
        LLGL::TestJIT1();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
