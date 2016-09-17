/*
 * DbgVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_VERTEX_BUFFER_H__
#define __LLGL_DBG_VERTEX_BUFFER_H__


#include <LLGL/VertexBuffer.h>


namespace LLGL
{


class DbgVertexBuffer : public VertexBuffer
{

    public:

        DbgVertexBuffer(LLGL::VertexBuffer& instance) :
            instance( instance )
        {
        }

        LLGL::VertexBuffer& instance;
        std::size_t         size        = 0;
        std::size_t         elements    = 0;
        bool                initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
