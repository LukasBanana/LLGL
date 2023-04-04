/*
 * Fence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FENCE_H
#define LLGL_FENCE_H


#include <LLGL/RenderSystemChild.h>


namespace LLGL
{


/**
\brief Fence interface for CPU/GPU synchronization.
\see RenderSystem::CreateFence
\see CommandQueue::Submit(Fence&)
\see CommandQueue::WaitFence
*/
class LLGL_EXPORT Fence : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::Fence );
};


} // /namespace LLGL


#endif



// ================================================================================
