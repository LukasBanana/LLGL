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
