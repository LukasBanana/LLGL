/*
 * D3D11CommandExecutor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_EXECUTOR_H
#define LLGL_D3D11_COMMAND_EXECUTOR_H


namespace LLGL
{


class D3D11CommandBuffer;
class D3D11CommandContext;
class D3D11SecondaryCommandBuffer;

/*
Executes all D3D11 commands that have been recorded in the specified command buffer.
D3D render states are tracked with the specified state manager.
*/
void ExecuteD3D11SecondaryCommandBuffer(const D3D11SecondaryCommandBuffer& cmdbuffer, D3D11CommandContext& context);
void ExecuteD3D11CommandBuffer(const D3D11CommandBuffer& cmdbuffer, D3D11CommandContext& context);


} // /namespace LLGL


#endif



// ================================================================================
