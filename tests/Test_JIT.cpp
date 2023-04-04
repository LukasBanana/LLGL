/*
 * Test_JIT.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <iostream>


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
