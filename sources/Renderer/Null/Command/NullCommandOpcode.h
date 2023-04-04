/*
 * NullCommandOpcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_COMMAND_OPCODE_H
#define LLGL_NULL_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum NullOpcode : std::uint8_t
{
    NullOpcodeBufferWrite = 1,
    NullOpcodeCopySubresource,
    NullOpcodeGenerateMips,
    //TODO
    NullOpcodeDraw,
    NullOpcodeDrawIndexed,
    NullOpcodePushDebugGroup,
    NullOpcodePopDebugGroup,
};


} // /namespace LLGL


#endif



// ================================================================================
