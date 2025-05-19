/*
 * GLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_GL_SWAP_CHAIN_CONTEXT_H


#include <LLGL/Surface.h>
#include <memory>


namespace LLGL
{


class Surface;
class GLContext;

// Helper class to manage the link between a swap-chain and a GL context.
class GLSwapChainContext
{

    public:

        GLSwapChainContext(const GLSwapChainContext&) = delete;
        GLSwapChainContext& operator = (const GLSwapChainContext&) = delete;

        virtual ~GLSwapChainContext() = default;

        // Returns true if this swap-chain context has a drawable (e.g. EGLSurface) to render into.
        virtual bool HasDrawable() const = 0;

        // Swaps the back buffer with the front buffer (Win32: ::SwapBuffers, X11: glXSwapBuffers).
        virtual bool SwapBuffers() = 0;

        // Resizes the GL swap-chain context. This is called after the context surface has been resized.
        virtual void Resize(const Extent2D& resolution) = 0;

    public:

        inline GLContext& GetGLContext() const
        {
            return context_;
        }

    public:

        // Creates a platform specific GLSwapChainContext instance.
        static std::unique_ptr<GLSwapChainContext> Create(GLContext& context, Surface& surface);

        // Makes the specified swap-chain context link current. If null, no context is current.
        static bool MakeCurrent(GLSwapChainContext* context);

    protected:

        // Initializes the swap-chain context with the specified GL context.
        GLSwapChainContext(GLContext& context);

    private:

        // Primary function to make the specified swap-chain context link current.
        static bool MakeCurrentUnchecked(GLSwapChainContext* context);

    private:

        GLContext& context_;

};


} // /namespace LLGL


#endif



// ================================================================================
