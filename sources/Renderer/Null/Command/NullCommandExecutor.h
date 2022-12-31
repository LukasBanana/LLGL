/*
 * NullCommandExecutor.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
