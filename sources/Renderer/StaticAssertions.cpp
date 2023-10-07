/*
 * StaticAssertions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "StaticAssertions.h"
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


} // /namespace LLGL



// ================================================================================
