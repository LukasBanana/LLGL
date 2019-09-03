/*
 * Test_JIT.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>


#if defined LLGL_ENABLE_JIT_COMPILER && defined LLGL_DEBUG

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

#else // LLGL_ENABLE_JIT_COMPILER

int main()
{
    std::cerr << "LLGL was not compiled with LLGL_ENABLE_JIT_COMPILER" << std::endl;
    return 0;
}

#endif // /LLGL_ENABLE_JIT_COMPILER
