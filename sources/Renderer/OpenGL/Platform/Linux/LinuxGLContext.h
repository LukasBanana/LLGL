/*
 * LinuxGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_GL_CONTEXT_H
#define LLGL_LINUX_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxGLContext : public GLContext
{

    public:

        LinuxGLContext(RenderContextDescriptor& desc, Surface& surface, LinuxGLContext* sharedContext);
        ~LinuxGLContext();

        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        void Resize(const Size& resolution) override;

    private:

        bool Activate(bool activate) override;

        void CreateContext(const RenderContextDescriptor& contextDesc, const NativeHandle& nativeHandle, LinuxGLContext* sharedContext);
        void DeleteContext();
        
        GLXContext CreateContextCoreProfile(GLXContext glcShared, int major, int minor);
        GLXContext CreateContextCompatibilityProfile(GLXContext glcShared);

        ::Display*      display_    = nullptr;
        ::Window        wnd_        = 0;
        XVisualInfo*    visual_     = nullptr;
        GLXContext      glc_        = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
