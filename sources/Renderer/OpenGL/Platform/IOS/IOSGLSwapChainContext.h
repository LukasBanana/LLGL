/*
 * IOSGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_IOS_GL_SWAP_CHAIN_CONTEXT_H


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"

#include <OpenGLES/EAGL.h>


@interface IOSGLSwapChainViewController : GLKViewController
@end


namespace LLGL
{


class IOSGLContext;

class IOSGLSwapChainContext final : public GLSwapChainContext
{

    public:

        IOSGLSwapChainContext(IOSGLContext& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentEGLContext(IOSGLSwapChainContext* context);

    private:

        EAGLContext*                    context_        = nullptr;
        GLKView*                        view_           = nullptr;
        IOSGLSwapChainViewController*   viewController_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
