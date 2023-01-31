/*
 * BufferArray.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_ARRAY_H
#define LLGL_BUFFER_ARRAY_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/BufferFlags.h>


namespace LLGL
{


/**
\brief Hardware buffer container interface.
\remarks This array can only contain buffers which are all from the same type,
like an array of vertex buffers for instance.
\see RenderSystem::CreateBufferArray
*/
class LLGL_EXPORT BufferArray : public RenderSystemChild
{

    public:

        /**
        \brief Returns a bitwise-OR combination of the binding flags of all sub-buffers.
        \see BufferDescriptor::bindFlags
        */
        inline long GetBindFlags() const
        {
            return bindFlags_;
        }

    protected:

        BufferArray(long bindFlags);

    private:

        long bindFlags_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
