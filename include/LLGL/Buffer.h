/*
 * Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_H
#define LLGL_BUFFER_H


#include "Resource.h"
#include "BufferFlags.h"


namespace LLGL
{


//! Hardware buffer interface.
class LLGL_EXPORT Buffer : public Resource
{

    public:

        //! Returns the ResourceType for the respective BufferType.
        ResourceType QueryResourceType() const override;

        //! Returns the type of this buffer.
        inline BufferType GetType() const
        {
            return type_;
        }

    protected:

        Buffer(const BufferType type);

    private:

        BufferType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
