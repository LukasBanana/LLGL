/*
 * MacOSGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_MACOS_GL_SWAP_CHAIN_CONTEXT_H


#import <Cocoa/Cocoa.h>

#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"


namespace LLGL
{


class Surface;
class MacOSGLContext;

class MacOSGLSwapChainContext final : public GLSwapChainContext
{

    public:

        MacOSGLSwapChainContext(MacOSGLContext& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;
        
    public:

        static bool MakeCurrentNSGLContext(MacOSGLSwapChainContext* context);

    private:

        NSOpenGLContext*    ctx_    = nullptr;
        NSView*             view_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
