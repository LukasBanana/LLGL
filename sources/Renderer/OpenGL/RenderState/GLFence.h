/*
 * GLFence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FENCE_H
#define LLGL_GL_FENCE_H


#include <LLGL/Fence.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLFence final : public Fence
{

    public:

        ~GLFence();

        void Submit();
        bool Wait(GLuint64 timeout);

    private:

        GLsync sync_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
