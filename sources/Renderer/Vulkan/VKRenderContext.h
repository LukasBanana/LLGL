/*
 * VKRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_CONTEXT_H
#define LLGL_VK_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <memory>
#include "Vulkan.h"
#include "VKPtr.h"


namespace LLGL
{


class VKRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        VKRenderContext(
            const VKPtr<VkInstance>& instance,
            RenderContextDescriptor desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

    private:

        void CreateVkSurface();
        
        /* ----- Common objects ----- */

        VkInstance          instance_;
        VKPtr<VkSurfaceKHR> surface_;

};


} // /namespace LLGL


#endif



// ================================================================================
