/*
 * PipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_LAYOUT_H
#define LLGL_PIPELINE_LAYOUT_H


#include "Export.h"
#include "PipelineLayoutFlags.h"


namespace LLGL
{


/**
\brief Pipeline layout interface.
\remarks An instance of this interface provides all descriptor sets (as called in Vulkan)
or descriptor heaps (as called in Direct3D 12) for graphics and compute pipelines.
\todo Maybe rename "PipelineLayout" in "ResourceViewLayout"?
*/
class LLGL_EXPORT PipelineLayout
{

    public:

        virtual ~PipelineLayout()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
