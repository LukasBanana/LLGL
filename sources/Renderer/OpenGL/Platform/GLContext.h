/*
 * GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_CONTEXT_H__
#define __LLGL_GL_CONTEXT_H__


namespace LLGL
{


class GLContext
{

    public:

        virtual ~GLContext()
        {
        }

        virtual bool Activate(bool activate) = 0;

        virtual bool SetSwapInterval(int interval) = 0;

        virtual bool SwapBuffers() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
