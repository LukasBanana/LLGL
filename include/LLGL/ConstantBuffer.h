/*
 * ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_CONSTANT_BUFFER_H__
#define __LLGL_CONSTANT_BUFFER_H__


#include "Export.h"
#include <string>


namespace LLGL
{


//! Constant buffer descriptor structure.
struct ConstantBufferDescriptor
{
    std::string     name;           //!< Constant buffer name.
    unsigned int    index   = 0;    //!< Index of the constant buffer within the respective shader.
    unsigned int    size    = 0;    //!< Buffer size (in bytes).
};

//! Constant buffer (also "Uniform Buffer Object") interface.
class LLGL_EXPORT ConstantBuffer
{

    public:

        virtual ~ConstantBuffer()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
