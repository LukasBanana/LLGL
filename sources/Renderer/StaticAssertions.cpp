/*
 * StaticAssertions.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "StaticAssertions.h"
#include "Serialization.h"
#include <LLGL/Format.h>
#include <LLGL/IndirectArguments.h>
#include <LLGL/QueryHeapFlags.h>


namespace LLGL
{


LLGL_ASSERT_STDLAYOUT_STRUCT( FormatAttributes );
LLGL_ASSERT_STDLAYOUT_STRUCT( DrawIndirectArguments );
LLGL_ASSERT_STDLAYOUT_STRUCT( DrawIndexedIndirectArguments );
LLGL_ASSERT_STDLAYOUT_STRUCT( DrawPatchIndirectArguments );
LLGL_ASSERT_STDLAYOUT_STRUCT( DispatchIndirectArguments );
LLGL_ASSERT_STDLAYOUT_STRUCT( QueryPipelineStatistics );
LLGL_ASSERT_STDLAYOUT_STRUCT( Serialization::Segment );


} // /namespace LLGL



// ================================================================================
