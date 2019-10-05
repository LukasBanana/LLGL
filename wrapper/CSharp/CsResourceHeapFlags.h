/*
 * CsResourceHeapFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsRenderSystemChild.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Structures ----- */

public ref class ResourceViewDescriptor
{

    public:

        ResourceViewDescriptor();
        ResourceViewDescriptor(Resource^ resource);

        property Resource^ Resource;

};

public ref class ResourceHeapDescriptor
{

    public:

        ResourceHeapDescriptor();

        property PipelineLayout^                PipelineLayout;
        property List<ResourceViewDescriptor^>^ ResourceViews;

};


} // /namespace SharpLLGL



// ================================================================================
