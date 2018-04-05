/*
 * ResourceViewHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_VIEW_HEAP_H
#define LLGL_RESOURCE_VIEW_HEAP_H


#include "Export.h"
#include "ResourceViewHeapFlags.h"


namespace LLGL
{


/**
\brief Resource heap interface.
\remarks An instance of this interface provides a descriptor set (as called in Vulkan)
or descriptor heap (as called in Direct3D 12) for graphics and compute pipelines.
*/
class LLGL_EXPORT ResourceViewHeap
{

    public:

        virtual ~ResourceViewHeap()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
