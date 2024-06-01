/*
 * D3D11StaticAssertions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../StaticAssertions.h"
#include "Command/D3D11Command.h"


namespace LLGL
{


// D3D11Command.h
LLGL_ASSERT_POD_TYPE( D3D11CmdSetVertexBuffer );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetVertexBufferArray );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetIndexBuffer );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetResourceHeap );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetResource );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetPipelineState );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetBlendFactor );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetStencilRef );
LLGL_ASSERT_POD_TYPE( D3D11CmdSetUniforms );
LLGL_ASSERT_POD_TYPE( D3D11CmdDraw );
LLGL_ASSERT_POD_TYPE( D3D11CmdDrawIndexed );
LLGL_ASSERT_POD_TYPE( D3D11CmdDrawInstanced );
LLGL_ASSERT_POD_TYPE( D3D11CmdDrawIndexedInstanced );
LLGL_ASSERT_POD_TYPE( D3D11CmdDrawInstancedIndirect );
LLGL_ASSERT_POD_TYPE( D3D11CmdDispatch );
LLGL_ASSERT_POD_TYPE( D3D11CmdDispatchIndirect );


} // /namespace LLGL



// ================================================================================
