/*
 * NullCommandExecutor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_COMMAND_EXECUTOR_H
#define LLGL_NULL_COMMAND_EXECUTOR_H


#include "NullCommandBuffer.h"


namespace LLGL
{


// Executes all virtual commands from the specified command buffer.
void ExecuteNullVirtualCommandBuffer(const NullVirtualCommandBuffer& virtualCmdBuffer);


} // /namespace LLGL


#endif



// ================================================================================
