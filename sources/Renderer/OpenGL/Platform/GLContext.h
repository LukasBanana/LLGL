/*
 * GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_CONTEXT_H__
#define __LLGL_GL_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContextDescriptor.h>
#include <memory>


namespace LLGL
{


class GLContext
{

    public:

        virtual ~GLContext()
        {
        }

        static std::unique_ptr<GLContext> Create(RenderContextDescriptor& desc, Window& window, GLContext* sharedContext);

        static bool MakeCurrent(GLContext* context);

        static GLContext* Active();

        virtual bool SetSwapInterval(int interval) = 0;

        virtual bool SwapBuffers() = 0;

    protected:

        virtual bool Activate(bool activate) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
