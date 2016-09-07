/*
 * StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_STORAGE_BUFFER_H__
#define __LLGL_STORAGE_BUFFER_H__


#include "Export.h"
#include <string>


namespace LLGL
{


//! Storage buffer descriptor structure.
struct StorageBufferDescriptor
{
    std::string     name;           //!< Storage buffer name.
    unsigned int    index   = 0;    //!< Index of the storage buffer within the respective shader.
    unsigned int    size    = 0;    //!< Buffer size (in bytes).
};

//! Storage buffer (also "Shader Sotrage Object" or "Read/Write Buffer") interface.
class LLGL_EXPORT StorageBuffer
{

    public:

        virtual ~StorageBuffer()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
