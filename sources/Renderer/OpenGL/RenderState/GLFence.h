/*
 * GLFence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FENCE_H
#define LLGL_GL_FENCE_H


#include <LLGL/Fence.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLFence : public Fence
{

    public:

        ~GLFence();

        void Submit();
        bool Wait(GLuint64 timeout);

    private:

        void Release();

        GLsync sync_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
