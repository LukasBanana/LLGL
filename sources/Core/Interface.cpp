/*
 * Interface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Interface.h>
#include <LLGL/RenderSystem.h>
#if LLGL_WINDOWING_ENABLED
#   include <LLGL/Window.h>
#endif
#include <LLGL/Canvas.h>
#include <LLGL/Utils/Input.h>
#include <LLGL/Display.h>


namespace LLGL
{


bool Interface::IsInstanceOf(int id) const
{
    return (id == InterfaceID::Interface);
}

void RenderSystemChild::SetDebugName(const char* /*name*/)
{
    // dummy
}

// Implement bases functions of all sub classes of <Interface> here:

LLGL_IMPLEMENT_INTERFACE( RenderSystem,             Interface         )
LLGL_IMPLEMENT_INTERFACE( RenderSystemChild,        Interface         )
LLGL_IMPLEMENT_INTERFACE( Surface,                  Interface         )

#if LLGL_WINDOWING_ENABLED
LLGL_IMPLEMENT_INTERFACE( Window,                   Surface           )
LLGL_IMPLEMENT_INTERFACE( Window::EventListener,    Interface         )
#endif

LLGL_IMPLEMENT_INTERFACE( Input,                    Interface         )
LLGL_IMPLEMENT_INTERFACE( Canvas,                   Surface           )
LLGL_IMPLEMENT_INTERFACE( Canvas::EventListener,    Interface         )
LLGL_IMPLEMENT_INTERFACE( Display,                  Interface         )
LLGL_IMPLEMENT_INTERFACE( ResourceHeap,             RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( Resource,                 RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( Texture,                  Resource          )
LLGL_IMPLEMENT_INTERFACE( Buffer,                   Resource          )
LLGL_IMPLEMENT_INTERFACE( Sampler,                  Resource          )
LLGL_IMPLEMENT_INTERFACE( CommandBuffer,            RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( CommandBufferTier1,       CommandBuffer     )
LLGL_IMPLEMENT_INTERFACE( CommandQueue,             RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( Fence,                    RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( PipelineLayout,           RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( PipelineCache,            RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( PipelineState,            RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( QueryHeap,                RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( RenderTarget,             RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( RenderPass,               RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( Shader,                   RenderSystemChild )
LLGL_IMPLEMENT_INTERFACE( SwapChain,                RenderTarget      )


} // /namespace LLGL



// ================================================================================
