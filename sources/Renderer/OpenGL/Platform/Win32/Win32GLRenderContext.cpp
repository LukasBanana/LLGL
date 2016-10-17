/*
 * Win32GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"


namespace LLGL
{


void GLRenderContext::GetNativeContextHandle(NativeContextHandle& windowContext)
{
    windowContext.parentWindow = 0;
}


} // /namespace LLGL



// ================================================================================
