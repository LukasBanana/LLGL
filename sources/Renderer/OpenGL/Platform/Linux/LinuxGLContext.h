/*
 * LinuxGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_LINUX_GL_CONTEXT_H__
#define __LLGL_LINUX_GL_CONTEXT_H__


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxGLContext : public GLContext
{

    public:

        LinuxGLContext(RenderContextDescriptor& desc, Window& window, LinuxGLContext* sharedContext);
        ~LinuxGLContext();

        bool SetSwapInterval(int interval) override;

        bool SwapBuffers() override;

    private:

        bool Activate(bool activate) override;

        void CreateContext(const NativeHandle& nativeHandle, LinuxGLContext* sharedContext);
        void DeleteContext();

        ::Display*      display_;
        ::Window        wnd_;
        XVisualInfo*    visual_;
        GLXContext      glc_;

};


} // /namespace LLGL


#endif



// ================================================================================
