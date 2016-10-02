/*
 * GLVertexArrayObject.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexArrayObject.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"


namespace LLGL
{


GLVertexArrayObject::GLVertexArrayObject()
{
    glGenVertexArrays(1, &id_);
}

GLVertexArrayObject::~GLVertexArrayObject()
{
    glDeleteVertexArrays(1, &id_);
}

void GLVertexArrayObject::BuildVertexAttribute(const VertexFormat& vertexFormat, unsigned int index)
{
    /* Get vertex attribute from list */
    const auto& attrib = vertexFormat.GetAttributes()[index];

    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(index);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (!attrib.conversion && attrib.dataType != DataType::Float && attrib.dataType != DataType::Double)
    {
        //if (!glVertexAttribIPointer)
        //    throw std::runtime_error("integral vertex attributes not supported by renderer");

        glVertexAttribIPointer(
            index,
            attrib.components,
            GLTypes::Map(attrib.dataType),
            vertexFormat.GetFormatSize(),
            reinterpret_cast<const char*>(0) + attrib.offset
        );
    }
    else
    {
        glVertexAttribPointer(
            index,
            attrib.components,
            GLTypes::Map(attrib.dataType),
            GL_FALSE,
            vertexFormat.GetFormatSize(),
            reinterpret_cast<const char*>(0) + attrib.offset
        );
    }
}


} // /namespace LLGL



// ================================================================================
