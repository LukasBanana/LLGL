/*
 * CsResourceHeapFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsResourceHeapFlags.h"


namespace SharpLLGL
{


/*
 * ResourceViewDescriptor class
 */

ResourceViewDescriptor::ResourceViewDescriptor()
{
    Resource = nullptr;
}

ResourceViewDescriptor::ResourceViewDescriptor(SharpLLGL::Resource^ resource)
{
    Resource = resource;
}


/*
 * ResourceHeapDescriptor class
 */

ResourceHeapDescriptor::ResourceHeapDescriptor()
{
    PipelineLayout  = nullptr;
    ResourceViews   = gcnew List<ResourceViewDescriptor^>();
}


} // /namespace SharpLLGL



// ================================================================================
