/*
 * BufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_ARRAY_H
#define LLGL_BUFFER_ARRAY_H


#include "Export.h"
#include "BufferFlags.h"


namespace LLGL
{


/**
\brief Array of hardware buffers interface.
\remarks This array can only contain buffers which are all from the same type,
like an array of vertex buffers for instance.
*/
class LLGL_EXPORT BufferArray
{

    public:

        BufferArray(const BufferArray&) = delete;
        BufferArray& operator = (const BufferArray&) = delete;

        virtual ~BufferArray();

        //! Returns the type of buffers this array contains.
        inline BufferType GetType() const
        {
            return type_;
        }

    protected:

        BufferArray(const BufferType type);

    private:

        BufferType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
