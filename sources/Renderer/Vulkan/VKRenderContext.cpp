/*
 * VKRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderContext.h"


namespace LLGL
{


/* ----- Common ----- */

VKRenderContext::VKRenderContext(const VKPtr<VkInstance>& instance, RenderContextDescriptor desc, const std::shared_ptr<Surface>& surface) :
    instance_ { instance                      },
    surface_  { instance, vkDestroySurfaceKHR }
{
    SetOrCreateSurface(surface, desc.videoMode, nullptr);
    CreateVkSurface();
}

void VKRenderContext::Present()
{
    //todo
}

/* ----- Configuration ----- */

void VKRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    //todo
}

void VKRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    //todo
}


/*
 * ======= Private: =======
 */

void VKRenderContext::CreateVkSurface()
{


}


} // /namespace LLGL



// ================================================================================
