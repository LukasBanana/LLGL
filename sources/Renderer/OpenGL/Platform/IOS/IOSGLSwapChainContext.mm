/*
 * IOSGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "IOSGLSwapChainContext.h"
#include "IOSGLContext.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& /*surface*/)
{
    return MakeUnique<IOSGLSwapChainContext>(static_cast<IOSGLContext&>(context));
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return IOSGLSwapChainContext::MakeCurrentEGLContext(static_cast<IOSGLSwapChainContext*>(context));
}


/*
 * IOSGLSwapChainContext class
 */

IOSGLSwapChainContext::IOSGLSwapChainContext(IOSGLContext& context) :
    GLSwapChainContext { context                  },
    context_           { context.GetEAGLContext() }
{
}

bool IOSGLSwapChainContext::SwapBuffers()
{
    return true; // dummy
}

bool IOSGLSwapChainContext::MakeCurrentEGLContext(IOSGLSwapChainContext* context)
{
    EAGLContext* contextEAGL = (context != nullptr ? context->context_ : nil);
    return ([EAGLContext setCurrentContext:contextEAGL] != NO);
}


} // /namespace LLGL



// ================================================================================
