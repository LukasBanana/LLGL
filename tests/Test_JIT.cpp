/*
 * Test_JIT.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <stdio.h>


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
        ::fprintf(stderr, "%s\n", e.what());
    }

    return 0;
}

#else // LLGL_ENABLE_JIT_COMPILER

int main()
{
    ::fprintf(stderr, "LLGL was not compiled with LLGL_ENABLE_JIT_COMPILER\n");
    return 0;
}

#endif // /LLGL_ENABLE_JIT_COMPILER
