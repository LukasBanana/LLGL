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

    /* Set instance divisor */
    if (attribute.instanceDivisor > 0)
        glVertexAttribDivisor(index, attribute.instanceDivisor);

    /* Get data type and components of vector type */
    DataType        dataType    = DataType::Float;
    unsigned int    components  = 0;
    VectorTypeFormat(attribute.vectorType, dataType, components);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (!attribute.conversion && dataType != DataType::Float && dataType != DataType::Double)
    {
        //if (!glVertexAttribIPointer)
        //    throw std::runtime_error("integral vertex attributes not supported by renderer");

        glVertexAttribIPointer(
            index,
            components,
            GLTypes::Map(dataType),
            stride,
            reinterpret_cast<const void*>(attribute.offset)
        );
    }
    else
    {
        glVertexAttribPointer(
            index,
            components,
            GLTypes::Map(dataType),
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(attribute.offset)
        );
    }
}


} // /namespace LLGL



// ================================================================================
