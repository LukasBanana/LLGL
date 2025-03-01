/*
 * DebugBreakOnError.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Debug.h"


namespace LLGL
{


LLGL_EXPORT void DebugBreakOnError()
{
    /********************************************************************************************
     * Hi there, you're here because the debug layer detected a runtime error in the API usage. *
     * This may or may not be your fault. If you think it's totally not your fault,             *
     * please file a bug ticket on https://github.com/LukasBanana/LLGL                          *
     * Thank you :)                                                                             *
     ********************************************************************************************/

    LLGL_DEBUG_BREAK();
}


} // /namespace LLGL



// ================================================================================
