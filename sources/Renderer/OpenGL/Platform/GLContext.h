/*
 * GLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_CONTEXT_H
#define LLGL_GL_CONTEXT_H


#include <LLGL/Surface.h>
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Container/ArrayView.h>
#include <memory>
#include "../RenderState/GLStateManager.h"


namespace LLGL
{

namespace OpenGL
{

struct RenderSystemNativeHandle;

} // /namespace OpenGL


// GL pixel format structure: samples and pixel bit size.
struct GLPixelFormat
{
    int colorBits   = 32;
    int depthBits   = 0;
    int stencilBits = 0;
    int samples     = 0;
};

// Compare operators for GLPixelFormat structure:
bool operator == (const GLPixelFormat& lhs, const GLPixelFormat& rhs);
bool operator != (const GLPixelFormat& lhs, const GLPixelFormat& rhs);

// Base wrapper class for a GL context.
class GLContext
{

    public:

        virtual ~GLContext() = default;

        // Returns the number of samples for this GL context. Must be in range [1, 64].
        virtual int GetSamples() const = 0;

        // Returns the native handle of the GL context.
        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const = 0;

    public:

        // Returns the color format for this GL context.
        inline Format GetColorFormat() const
        {
            return colorFormat_;
        }

        // Returns the depth-stencil format for this GL context.
        inline Format GetDepthStencilFormat() const
        {
            return depthStencilFormat_;
        }

        // Returns the state manager that is associated with this context.
        inline GLStateManager& GetStateManager()
        {
            return stateMngr_;
        }

        // Returns the global index of this GL context. This is assigned when the context is created. The first index starts with 1. The invalid index is 0.
        inline unsigned GetGlobalIndex() const
        {
            return globalIndex_;
        }

    public:

        // Creates a platform specific GLContext instance.
        static std::unique_ptr<GLContext> Create(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            GLContext*                          sharedContext       = nullptr,
            const ArrayView<char>&              customNativeHandle  = {}
        );

        // Sets the current GL context. This only stores a reference to this context (GetCurrent) and its global index (GetGlobalIndex).
        static void SetCurrent(GLContext* context);

        // Returns a pointer to the current GL context.
        static GLContext* GetCurrent();

        // Returns the global index of the current GL context. 0 denotes an invalid index.
        static unsigned GetCurrentGlobalIndex();

        // Sets the swap interval for the current GL context.
        static bool SetCurrentSwapInterval(int interval);

    protected:

        // Sets the swap interval of the platform dependent GL context.
        virtual bool SetSwapInterval(int interval) = 0;

    protected:

        // Initializes the GL context with an assigned global index (GetGlobalIndex).
        GLContext();

        // Deduces the color format by the specified component bits and shifting.
        void DeduceColorFormat(int rBits, int rShift, int gBits, int gShift, int bBits, int bShift, int aBits, int aShift);

        // Deduces the depth-stencil format by the specified bit sizes.
        void DeduceDepthStencilFormat(int depthBits, int stencilBits);

        // Sets the color format to RGBA8UNorm.
        void SetDefaultColorFormat();

        // Sets the depth-stencil format to D24UNormS8UInt;
        void SetDefaultDepthStencilFormat();

    private:

        GLStateManager  stateMngr_;
        Format          colorFormat_        = Format::Undefined;
        Format          depthStencilFormat_ = Format::Undefined;
        unsigned        globalIndex_        = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
