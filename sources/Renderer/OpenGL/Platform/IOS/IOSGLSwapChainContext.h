/*
 * IOSGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_IOS_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"

#include <OpenGLES/EAGL.h>


namespace LLGL
{


class IOSGLContext;

class IOSGLSwapChainContext final : public GLSwapChainContext
{

    public:

        IOSGLSwapChainContext(IOSGLContext& context);

        bool SwapBuffers() override;

    public:

        static bool MakeCurrentEGLContext(IOSGLSwapChainContext* context);

    private:

        EAGLContext* context_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
