/*
 * RenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_PASS_H
#define LLGL_RENDER_PASS_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Render pass interface.
\remarks A render pass is a high level construct adopted from Vulkan and Metal.
It is used to tell the driver the various segments of a frame and which render target attachments are used and which depend on each other.
\see RenderSystem::CreateRenderPass
\see CommandBuffer::BeginRenderPass
\see CommandBuffer::EndRenderPass
*/
class LLGL_EXPORT RenderPass : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::RenderPass );
};


} // /namespace LLGL


#endif



// ================================================================================
