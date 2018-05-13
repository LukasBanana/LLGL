/*
 * Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_FENCE_H
#define LLGL_FENCE_H


#include "Export.h"


namespace LLGL
{


//! Fence interface for CPU/GPU synchronization.
class LLGL_EXPORT Fence
{

    public:

        virtual ~Fence()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
