/*
 * GLFence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_FENCE_H
#define LLGL_GL_FENCE_H


#include <LLGL/Fence.h>
#include "../OpenGL.h"
#include <string>


namespace LLGL
{


class GLFence final : public Fence
{

    public:

        void SetName(const char* name) override;

    public:

        ~GLFence();

        void Submit();
        bool Wait(GLuint64 timeout);

    private:

        GLsync      sync_ = 0;

        #ifdef LLGL_DEBUG
        // Only provide name in debug mode, to keep fence objects as lightweight as possible
        std::string name_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
