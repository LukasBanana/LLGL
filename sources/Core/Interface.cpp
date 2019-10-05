/*
 * Interface.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Interface.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <LLGL/Input.h>
#include <LLGL/Timer.h>
#include <LLGL/Display.h>


namespace LLGL
{


bool Interface::IsInstanceOf(int id) const
{
    return (id == InterfaceID::Interface);
}

void RenderSystemChild::SetName(const char* name)
{
    // dummy
}

// Implement bases functions of all sub classes of <Interface> here:

LLGL_IMPLEMENT_INTERFACE( RenderSystem,             Interface               )
LLGL_IMPLEMENT_INTERFACE( RenderSystemChild,        Interface               )
LLGL_IMPLEMENT_INTERFACE( Surface,                  Interface               )
LLGL_IMPLEMENT_INTERFACE( Window,                   Surface                 )
LLGL_IMPLEMENT_INTERFACE( Window::EventListener,    Interface               )
LLGL_IMPLEMENT_INTERFACE( Input,                    Window::EventListener   )
LLGL_IMPLEMENT_INTERFACE( Canvas,                   Surface                 )
LLGL_IMPLEMENT_INTERFACE( Canvas::EventListener,    Interface               )
LLGL_IMPLEMENT_INTERFACE( Timer,                    Interface               )
LLGL_IMPLEMENT_INTERFACE( Display,                  Interface               )
LLGL_IMPLEMENT_INTERFACE( ResourceHeap,             RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( Resource,                 RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( Texture,                  Resource                )
LLGL_IMPLEMENT_INTERFACE( Buffer,                   Resource                )
LLGL_IMPLEMENT_INTERFACE( Sampler,                  Resource                )
LLGL_IMPLEMENT_INTERFACE( CommandBuffer,            RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( CommandQueue,             RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( Fence,                    RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( PipelineLayout,           RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( PipelineState,            RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( QueryHeap,                RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( RenderTarget,             RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( RenderContext,            RenderTarget            )
LLGL_IMPLEMENT_INTERFACE( RenderPass,               RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( Shader,                   RenderSystemChild       )
LLGL_IMPLEMENT_INTERFACE( ShaderProgram,            RenderSystemChild       )


} // /namespace LLGL



// ================================================================================
