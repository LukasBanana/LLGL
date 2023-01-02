/*
 * NullCommandOpcode.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
