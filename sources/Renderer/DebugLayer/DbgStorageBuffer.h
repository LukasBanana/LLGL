/*
 * DbgStorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_STORAGE_BUFFER_H__
#define __LLGL_DBG_STORAGE_BUFFER_H__


#include <LLGL/StorageBuffer.h>


namespace LLGL
{


class DbgStorageBuffer : public StorageBuffer
{

    public:

        DbgStorageBuffer(LLGL::StorageBuffer& instance) :
            instance( instance )
        {
        }

        LLGL::StorageBuffer&    instance;
        StorageBufferDescriptor desc;
        bool                    initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
