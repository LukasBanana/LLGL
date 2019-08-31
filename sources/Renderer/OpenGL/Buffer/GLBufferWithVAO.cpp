/*
 * GLBufferWithVAO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(long bindFlags) :
    GLBuffer { bindFlags }
{
}

void GLBufferWithVAO::BuildVertexArray(const VertexFormat& vertexFormat)
{
    /* Store vertex format (required if this buffer is used in a buffer array) */
    vertexFormat_ = vertexFormat;

    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasExtension(GLExt::ARB_vertex_array_object))
    {
        /* Build vertex array with emulator (for GL 2.x compatibility) */
        BuildVertexArrayWithEmulator();
    }
    else
    #endif // /LLGL_GL_ENABLE_OPENGL2X
    {
        /* Build vertex array with native VAO */
        BuildVertexArrayWithVAO();
    }
}


/*
 * ======= Private: =======
 */

void GLBufferWithVAO::BuildVertexArrayWithVAO()
{
    /* Bind VAO */
    GLStateManager::Get().BindVertexArray(GetVaoID());
    {
        /* Bind VBO */
        GLStateManager::Get().BindBuffer(GLBufferTarget::ARRAY_BUFFER, GetID());

        /* Build each vertex attribute */
        for (GLuint i = 0, n = static_cast<GLuint>(vertexFormat_.attributes.size()); i < n; ++i)
        {
            vao_.BuildVertexAttribute(
                vertexFormat_.attributes[i],
                static_cast<GLsizei>(vertexFormat_.stride),
                i
            );
        }
    }
    GLStateManager::Get().BindVertexArray(0);
}

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLBufferWithVAO::BuildVertexArrayWithEmulator()
{
    /* Build each vertex attribute */
    for (GLuint i = 0, n = static_cast<GLuint>(vertexFormat_.attributes.size()); i < n; ++i)
    {
        vertexArrayGL2X_.BuildVertexAttribute(
            GetID(),
            vertexFormat_.attributes[i],
            static_cast<GLsizei>(vertexFormat_.stride),
            i
        );
    }
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
