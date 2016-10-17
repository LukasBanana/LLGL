/*
 * MacOSGLContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSGLContext.h"
#include "../../../../Platform/MacOS/MacOSWindow.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


std::unique_ptr<GLContext> GLContext::Create(RenderContextDescriptor& desc, Window& window, GLContext* sharedContext)
{
    MacOSGLContext* sharedContextGLNS = (sharedContext != nullptr ? LLGL_CAST(MacOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<MacOSGLContext>(desc, window, sharedContextGLNS);
}

MacOSGLContext::MacOSGLContext(RenderContextDescriptor& desc, Window& window, MacOSGLContext* sharedContext) :
    LLGL::GLContext( sharedContext )
{
}

MacOSGLContext::~MacOSGLContext()
{
}

bool MacOSGLContext::SetSwapInterval(int interval)
{
    //GLint sync = (desc_.vsync.enabled ? std::max(1, std::min(static_cast<GLint>(desc_.vsync.interval), 4)) : 0);
    //CGLSetParameter(ctx, kCGLCPSwapInterval, &sync);
    return false;
}

bool MacOSGLContext::SwapBuffers()
{
    return false;
}


/*
 * ======= Private: =======
 */

bool MacOSGLContext::Activate(bool activate)
{
    return false;
}

void MacOSGLContext::CreateContext(const NativeHandle& nativeHandle, MacOSGLContext* sharedContext)
{
}

void MacOSGLContext::DeleteContext()
{
}


} // /namespace LLGL



// ================================================================================
