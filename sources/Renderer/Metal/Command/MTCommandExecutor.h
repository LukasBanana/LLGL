/*
 * MTCommandExecutor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_EXECUTOR_H
#define LLGL_MT_COMMAND_EXECUTOR_H


namespace LLGL
{


namespace Metal { struct NativeCommand; }

class MTCommandContext;
class MTCommandBuffer;
class MTMultiSubmitCommandBuffer;

/*
Executes all Metal commands that have been recorded in the specified command buffer.
Metal render states are tracked with the specified command context.
*/
void ExecuteMTMultiSubmitCommandBuffer(const MTMultiSubmitCommandBuffer& cmdbuffer, MTCommandContext& context);
void ExecuteMTCommandBuffer(const MTCommandBuffer& cmdbuffer, MTCommandContext& context);

// Executes the specified native Metal command.
void ExecuteNativeMTCommand(const Metal::NativeCommand& cmd, MTCommandContext& context);


} // /namespace LLGL


#endif



// ================================================================================
