/*
 * Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_H
#define LLGL_BUFFER_H


#include "Resource.h"
#include "BufferFlags.h"//TODO: replace by "ResourceFlags.h"


namespace LLGL
{


/**
\brief Hardware buffer interface.
\see RenderSystem::CreateBuffer
*/
class LLGL_EXPORT Buffer : public Resource
{

    public:

        //! Returns the ResourceType for the respective BufferType.
        ResourceType QueryResourceType() const override final;

        /**
        \brief Returns the binding flags this buffer was created with.
        \see BufferDescriptor::bindFlags
        */
        inline long GetBindFlags() const
        {
            return bindFlags_;
        }

    protected:

        Buffer(long bindFlags);

    private:

        long bindFlags_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
