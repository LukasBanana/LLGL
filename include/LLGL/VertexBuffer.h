/*
 * VertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_VERTEX_BUFFER_H__
#define __LLGL_VERTEX_BUFFER_H__


#include "Export.h"
#include "VertexFormat.h"


namespace LLGL
{


//! Vertex buffer descriptor structure.
struct VertexBufferDescriptor
{
    VertexBufferDescriptor() = default;

    VertexBufferDescriptor(unsigned int size, BufferUsage usage, const VertexFormat& vertexFormat) :
        size        ( size         ),
        usage       ( usage        ),
        vertexFormat( vertexFormat )
    {
    }

    //! Buffer size (in bytes).
    unsigned int    size            = 0;

    //! Buffer usage (typically "BufferUsage::Static", since a vertex buffer is rarely changed).
    BufferUsage     usage           = BufferUsage::Static;

    /**
    \brief Specifies the vertex format layout.
    \remarks This is required to tell the renderer how the vertex attributes are stored inside the vertex buffer and
    it must be the same vertex format which is used for the respective graphics pipeline shader program.
    */
    VertexFormat    vertexFormat;
};


//! Vertex buffer interface.
class LLGL_EXPORT VertexBuffer
{

    public:

        virtual ~VertexBuffer()
        {
        }

        inline const VertexFormat& GetVertexFormat() const
        {
            return vertexFormat_;
        }

    protected:

        inline void SetVertexFormat(const VertexFormat& vertexFormat)
        {
            vertexFormat_ = vertexFormat;
        }

    private:

        VertexFormat vertexFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
