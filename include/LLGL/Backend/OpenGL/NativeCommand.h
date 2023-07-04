/*
 * NativeCommand.h (OpenGL)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_NATIVE_COMMAND_H
#define LLGL_OPENGL_NATIVE_COMMAND_H


#include <cstdint>


namespace LLGL
{

namespace OpenGL
{


/**
 * \brief Enumeration of all native commands the OpenGL backend can execute.
 * \see NativeCommand::type
 */
enum class NativeCommandType
{
    /**
     * \brief Clears the internal command buffer cache.
     * \remarks This should be used when a native command buffer handle is retrieved
     * and LLGL is supposed to continue dispatching graphics and compute commands after the native command buffer has been modified.
     * \remarks This is a slow operation because all internally managed OpenGL states will be retrieved from the current GL context,
     * i.e. a large number of <code>glGet*</code> functions will be invoked by this operation.
     */
    ClearCache = 1u,
};

/**
 * \brief Native command data structure as a workaround for backend differences.
 * \see CommandBuffer::DoNativeCommand
 */
struct NativeCommand
{
    NativeCommandType type;
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
