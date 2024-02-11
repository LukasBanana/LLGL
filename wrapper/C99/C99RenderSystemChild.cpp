/*
 * C99RenderSystemChild.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderSystemChild.h>
#include <LLGL-C/RenderSystemChild.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT void llglSetDebugName(LLGLRenderSystemChild renderSystemChild, const char* name)
{
    LLGL_PTR(RenderSystemChild, renderSystemChild)->SetDebugName(name);
}

LLGL_C_EXPORT void llglSetName(LLGLRenderSystemChild renderSystemChild, const char* name)
{
    llglSetDebugName(renderSystemChild, name);
}


// } /namespace LLGL



// ================================================================================
