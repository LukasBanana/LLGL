/*
 * CommandQueueFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_QUEUE_FLAGS_H
#define LLGL_COMMAND_QUEUE_FLAGS_H


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Command buffer recording flags.
\see Begin(CommandBuffer&, long)
*/
struct RecordingFlags
{
    enum
    {
        //! Specifies that each recording of a command buffer is only submitted once.
        OneTimeSubmit = (1 << 0),
    };
};


} // /namespace LLGL


#endif



// ================================================================================
