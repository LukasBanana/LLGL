/*
 * NativeCommand.h (Metal)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_METAL_NATIVE_COMMAND_H
#define LLGL_METAL_NATIVE_COMMAND_H


#include <cstdint>


namespace LLGL
{

namespace Metal
{


/**
 * \brief Enumeration of all native commands the Metal backend can execute.
 * \see NativeCommand::type
 */
enum class NativeCommandType
{
    /**
     * \brief Clears the internal command buffer cache.
     * \remarks This should be used when a native command buffer handle is retrieved
     * and LLGL is supposed to continue dispatching graphics and compute commands after the native command buffer has been modified.
     */
    ClearCache = 1u,

    /**
     * \brief Sets the binding slot for internal tessellation factor buffer.
     * \see NativeCommand::tessFactorBuffer
     */
    TessFactorBuffer,
};

/**
 * \brief Native command data structure as a workaround for backend differences.
 * \see CommandBuffer::DoNativeCommand
 */
struct NativeCommand
{
    NativeCommandType type;

    struct TessFactorBuffer
    {
        /**
        \brief Specifies the buffer slot for the internal tessellation factor buffer. By default 30, which is the maximum buffer slot.
        \remarks In the respective Metal tessellation kernel,
        this must refer to a buffer of type \c MTLTriangleTessellationFactorsHalf or \c MTLQuadTessellationFactorsHalf.
        */
        std::uint32_t slot;
    };

    union
    {
        TessFactorBuffer tessFactorBuffer;
    };
};


} // /namespace Metal

} // /namespace LLGL


#endif



// ================================================================================
