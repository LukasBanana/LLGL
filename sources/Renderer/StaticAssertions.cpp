/*
 * StaticAssertions.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "StaticAssertions.h"
#include <LLGL/Format.h>
#include <LLGL/IndirectArguments.h>


namespace LLGL
{


LLGL_ASSERT_POD_STRUCT( FormatDescriptor );
LLGL_ASSERT_POD_STRUCT( DrawIndirectArguments );
LLGL_ASSERT_POD_STRUCT( DrawIndexedIndirectArguments );
LLGL_ASSERT_POD_STRUCT( DrawPatchIndirectArgument );
LLGL_ASSERT_POD_STRUCT( DispatchIndirectArguments );


} // /namespace LLGL



// ================================================================================
