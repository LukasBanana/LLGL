/*
 * D3D9CommandExecutor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_COMMAND_EXECUTOR_H
#define LLGL_D3D9_COMMAND_EXECUTOR_H


#include "D3D9CommandBuffer.h"
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9StateManager;

// Executes all virtual commands from the specified command buffer.
void ExecuteD3D9VirtualCommandBuffer(const D3D9VirtualCommandBuffer& virtualCmdBuffer, D3D9StateManager* stateMngr);


} // /namespace LLGL


#endif



// ================================================================================
