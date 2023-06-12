/*
 * Buffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BUFFER_H
#define LLGL_BUFFER_H


#include <LLGL/Resource.h>
#include <LLGL/BufferFlags.h>


namespace LLGL
{


/**
\brief Hardware buffer interface.
\see RenderSystem::CreateBuffer
*/
class LLGL_EXPORT Buffer : public Resource
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Buffer );

    public:

        //! Returns ResourceType::Buffer.
        ResourceType GetResourceType() const override final;

        /**
        \brief Returns the binding flags this buffer was created with.
        \see BufferDescriptor::bindFlags
        */
        inline long GetBindFlags() const
        {
            return bindFlags_;
        }

        /**
        \brief Queries a descriptor of this buffer.
        \remarks This function only queries the following attributes:
        - \c size
        - \c bindFlags
        - \c cpuAccessFlags
        - \c miscFlags
        \remarks All other attributes (such as \c vertexAttribs etc.) cannot be queried by this function.
        Those attributes are set to the default value specified in BufferDescriptor.
        \remarks The returned flags (such as \c cpuAccessFlags etc.) are not necessarily the same that were specified when the resource was created.
        They reflect the capabilities of the actual hardware buffer.
        For example, a buffer created with CPUAccessFlags::Read might return CPUAccessFlags::ReadWrite,
        if the renderer backend does not distinguish between different CPU access flags.
        \see BufferDescriptor
        \see Texture::GetDesc
        */
        virtual BufferDescriptor GetDesc() const = 0;

    protected:

        Buffer(long bindFlags);

    private:

        long bindFlags_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
