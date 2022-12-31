/*
 * NullCommand.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_COMMAND_H
#define LLGL_NULL_COMMAND_H


#include <cstddef>
#include <cstdint>


namespace LLGL
{


class NullBuffer;


struct NullCmdBufferWrite
{
    NullBuffer* buffer;
    std::size_t offset;
    std::size_t size;
//  std::int8_t data[dataSize];
};

//TODO...

struct NullCmdPushDebugGroup
{
    std::size_t length;
//  char        name[length];
};

//struct NullCmdPopDebugGroup {};


} // /namespace LLGL


#endif



// ================================================================================
