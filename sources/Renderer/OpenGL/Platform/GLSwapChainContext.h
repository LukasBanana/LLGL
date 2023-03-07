/*
 * GLSwapChainContext.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        // Swaps the back buffer with the front buffer (Win32: ::SwapBuffers, X11: glXSwapBuffers).
        virtual bool SwapBuffers() = 0;

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
