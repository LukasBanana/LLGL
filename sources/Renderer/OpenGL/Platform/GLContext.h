/*
 * GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_CONTEXT_H
#define LLGL_GL_CONTEXT_H


#include <LLGL/Surface.h>
#include <LLGL/RenderContextFlags.h>
#include <memory>
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


// Base wrapper class for a GL context.
class GLContext
{

    public:

        // Sets the swap interval for vsync (Win32: wglSwapIntervalEXT, X11: glXSwapIntervalSGI).
        virtual bool SetSwapInterval(int interval) = 0;

        // Swaps the back buffer with the front buffer (Win32: ::SwapBuffers, X11: glXSwapBuffers).
        virtual bool SwapBuffers() = 0;

        // Resizes the GL context. This is called after the context surface has been resized.
        virtual void Resize(const Extent2D& resolution) = 0;

        // Returns the number of samples for this GL context. Must be in range [1, 64].
        virtual std::uint32_t GetSamples() const = 0;

    public:

        virtual ~GLContext();

        // Creates a platform specific GLContext instance.
        static std::unique_ptr<GLContext> Create(
            const RenderContextDescriptor&      desc,
            const RendererConfigurationOpenGL&  config,
            Surface&                            surface,
            GLContext*                          sharedContext
        );

        // Makes the specified GLContext current. If null, the current context will be deactivated.
        static bool MakeCurrent(GLContext* context);

        // Returns the active GLContext instance.
        static GLContext* Active();

        // Returns the state manager that is associated with this context.
        inline const std::shared_ptr<GLStateManager>& GetStateManager() const
        {
            return stateMngr_;
        }

    protected:

        GLContext(GLContext* sharedContext);

        // Activates or deactivates this GLContext (Win32: wglMakeCurrent, X11: glXMakeCurrent).
        virtual bool Activate(bool activate) = 0;

    private:

        std::shared_ptr<GLStateManager> stateMngr_;

};


} // /namespace LLGL


#endif



// ================================================================================
