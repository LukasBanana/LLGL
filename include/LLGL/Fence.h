/*
 * Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_FENCE_H
#define LLGL_FENCE_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Fence interface for CPU/GPU synchronization.
\see RenderSystem::CreateFence
\see CommandQueue::Submit(Fence&)
\see CommandQueue::WaitFence
*/
class LLGL_EXPORT Fence : public RenderSystemChild { };


} // /namespace LLGL


#endif



// ================================================================================
