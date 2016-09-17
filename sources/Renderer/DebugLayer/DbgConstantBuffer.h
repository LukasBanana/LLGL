/*
 * DbgConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_CONSTANT_BUFFER_H__
#define __LLGL_DBG_CONSTANT_BUFFER_H__


#include <LLGL/ConstantBuffer.h>


namespace LLGL
{


class DbgConstantBuffer : public ConstantBuffer
{

    public:

        DbgConstantBuffer(LLGL::ConstantBuffer& instance) :
            instance( instance )
        {
        }

        LLGL::ConstantBuffer&   instance;
        std::size_t             size        = 0;
        bool                    initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
