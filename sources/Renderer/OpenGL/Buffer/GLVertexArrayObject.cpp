/*
 * GLVertexArrayObject.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexArrayObject.h"
#include "GLSharedContextVertexArray.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/TypeNames.h>


namespace LLGL
{


void GLVertexArrayObject::Release()
{
    #if GL_ARB_vertex_array_object

    //TODO: this must use some form of deferred deletion as this d'tor is not guaranteed to be invoked with the correct GL context in place
    if (id_ != 0)
    {
        glDeleteVertexArrays(1, &id_);
        GLStateManager::Get().NotifyVertexArrayRelease(id_);
        id_ = 0;
    }

    #endif // /GL_ARB_vertex_array_object
}

void GLVertexArrayObject::BuildVertexLayout(const ArrayView<GLVertexAttribute>& attributes)
{
    #if GL_ARB_vertex_array_object

    LLGL_ASSERT_GL_EXT(ARB_vertex_array_object);

    /* Generate a VAO if not already done */
    if (id_ == 0)
        glGenVertexArrays(1, &id_);

    /* Build vertex attributes for this VAO */
    GLStateManager::Get().BindVertexArray(id_);
    {
        for (const GLVertexAttribute& attrib : attributes)
            BuildVertexAttribute(attrib);
    }
    GLStateManager::Get().BindVertexArray(0);

    #else // GL_ARB_vertex_array_object

    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_vertex_array_object");

    #endif // /GL_ARB_vertex_array_object
}


/*
 * ======= Private: =======
 */

void GLVertexArrayObject::BuildVertexAttribute(const GLVertexAttribute& attribute)
{
    #if GL_ARB_vertex_array_object

    GLStateManager::Get().BindBuffer(GLBufferTarget::ArrayBuffer, attribute.buffer);

    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(attribute.index);

    /* Set instance divisor */
    if (attribute.divisor > 0)
        glVertexAttribDivisor(attribute.index, attribute.divisor);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (attribute.isInteger)
    {
        LLGL_ASSERT_GL_EXT(EXT_gpu_shader4, "integral vertex attributes");
        glVertexAttribIPointer(
            attribute.index,
            attribute.size,
            attribute.type,
            attribute.stride,
            reinterpret_cast<const void*>(attribute.offsetPtrSized)
        );
    }
    else
    {
        glVertexAttribPointer(
            attribute.index,
            attribute.size,
            attribute.type,
            attribute.normalized,
            attribute.stride,
            reinterpret_cast<const void*>(attribute.offsetPtrSized)
        );
    }

    #endif // /GL_ARB_vertex_array_object
}


} // /namespace LLGL



// ================================================================================
