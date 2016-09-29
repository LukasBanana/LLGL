/*
 * DbgIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_INDEX_BUFFER_H__
#define __LLGL_DBG_INDEX_BUFFER_H__


#include <LLGL/IndexBuffer.h>


namespace LLGL
{


class DbgIndexBuffer : public IndexBuffer
{

    public:

        DbgIndexBuffer(LLGL::IndexBuffer& instance) :
            instance( instance )
        {
        }

        LLGL::IndexBuffer&      instance;
        IndexBufferDescriptor   desc;
        unsigned int            elements    = 0;
        bool                    initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
