/*
 * GLFence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FENCE_H
#define LLGL_GL_FENCE_H


#include <LLGL/Fence.h>
#include "../OpenGL.h"
#include "../../../Core/ImmutableString.h"


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

        GLsync          sync_ = 0;

        #ifdef LLGL_DEBUG
        // Only provide name in debug mode, to keep fence objects as lightweight as possible
        ImmutableString name_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
