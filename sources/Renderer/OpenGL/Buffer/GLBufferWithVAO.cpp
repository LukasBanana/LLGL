/*
 * GLBufferWithVAO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(long bindFlags) :
    GLBuffer { bindFlags }
{
}

void GLBufferWithVAO::BuildVertexArray(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    /* Store vertex format (required if this buffer is used in a buffer array) */
    if (numVertexAttribs > 0)
        vertexAttribs_ = std::vector<VertexAttribute>(vertexAttribs, vertexAttribs + numVertexAttribs);
    else
        vertexAttribs_.clear();

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
        for (const auto& attrib : vertexAttribs_)
            vao_.BuildVertexAttribute(attrib);
    }
    GLStateManager::Get().BindVertexArray(0);
}

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLBufferWithVAO::BuildVertexArrayWithEmulator()
{
    /* Build each vertex attribute */
    for (const auto& attrib : vertexFormat_.attributes)
        vertexArrayGL2X_.BuildVertexAttribute(GetID(), attrib);
    vertexArrayGL2X_.Finalize();
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
