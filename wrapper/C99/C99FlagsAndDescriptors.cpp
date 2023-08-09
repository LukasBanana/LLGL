/*
 * C99FlagsAndDescriptors.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Format.h>
#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT const LLGLFormatAttributes* llglGetFormatAttribs(LLGLFormat format)
{
    return (const LLGLFormatAttributes*)(&GetFormatAttribs((Format)format));
}


// } /namespace LLGL



// ================================================================================
