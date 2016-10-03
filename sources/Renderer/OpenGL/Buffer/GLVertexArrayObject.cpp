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

void GLVertexArrayObject::BuildVertexAttribute(const VertexAttribute& attribute, unsigned int stride, unsigned int index)
{
    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(index);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (!attribute.conversion && attribute.dataType != DataType::Float && attribute.dataType != DataType::Double)
    {
        //if (!glVertexAttribIPointer)
        //    throw std::runtime_error("integral vertex attributes not supported by renderer");

        glVertexAttribIPointer(
            index,
            attribute.components,
            GLTypes::Map(attribute.dataType),
            stride,
            reinterpret_cast<const char*>(0) + attribute.offset
        );
    }
    else
    {
        glVertexAttribPointer(
            index,
            attribute.components,
            GLTypes::Map(attribute.dataType),
            GL_FALSE,
            stride,
            reinterpret_cast<const char*>(0) + attribute.offset
        );
    }
}


} // /namespace LLGL



// ================================================================================
