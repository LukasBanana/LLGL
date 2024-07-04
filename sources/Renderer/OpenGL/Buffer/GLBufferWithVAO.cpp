/*
 * GLBufferWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(long bindFlags, const char* debugName) :
    GLBuffer { bindFlags, debugName }
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
    if (!HasNativeVAO())
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
    /* Bind VAO and build vertex layout */
    GLStateManager::Get().BindVertexArray(GetVaoID());
    {
        GLStateManager::Get().BindBuffer(GLBufferTarget::ArrayBuffer, GetID());
        vao_.BuildVertexLayout(vertexAttribs_);
    }
    GLStateManager::Get().BindVertexArray(0);
}

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLBufferWithVAO::BuildVertexArrayWithEmulator()
{
    vertexArrayGL2X_.BuildVertexLayout(GetID(), vertexAttribs_);
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
