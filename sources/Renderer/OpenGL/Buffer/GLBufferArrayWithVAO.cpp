/*
 * GLBufferArrayWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferArrayWithVAO.h"
#include "GLBufferWithVAO.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferArrayWithVAO::GLBufferArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    GLBufferArray { numBuffers, bufferArray }
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeVAO())
    {
        /* Build vertex array with emulator (for GL 2.x compatibility) */
        BuildVertexArrayWithEmulator(numBuffers, bufferArray);
    }
    else
    #endif // /LLGL_GL_ENABLE_OPENGL2X
    {
        /* Build vertex array with native VAO */
        BuildVertexArrayWithVAO(numBuffers, bufferArray);
    }
}

void GLBufferArrayWithVAO::SetDebugName(const char* name)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (HasNativeVAO())
    #endif
    {
        /* Set label for VAO */
        GLSetObjectLabel(GL_VERTEX_ARRAY, vao_.GetID(), name);
    }
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void ThrowNoVertexBufferErr()
{
    throw std::invalid_argument(
        "cannot build vertex array with buffer that was not created with the 'LLGL::BindFlags::VertexBuffer' flag"
    );
}

void GLBufferArrayWithVAO::BuildVertexArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    /* Bind VAO */
    GLStateManager::Get().BindVertexArray(GetVaoID());
    {
        while (GLBuffer* bufferGL = NextArrayResource<GLBuffer>(numBuffers, bufferArray))
        {
            if ((bufferGL->GetBindFlags() & BindFlags::VertexBuffer) != 0)
            {
                /* Bind VBO and build vertex layout */
                auto* vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, bufferGL);
                GLStateManager::Get().BindBuffer(GLBufferTarget::ArrayBuffer, vertexBufferGL->GetID());
                vao_.BuildVertexLayout(vertexBufferGL->GetVertexAttribs());
            }
            else
                ThrowNoVertexBufferErr();
        }
    }
    GLStateManager::Get().BindVertexArray(0);
}

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLBufferArrayWithVAO::BuildVertexArrayWithEmulator(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    while (numBuffers-- > 0)
    {
        if (((*bufferArray)->GetBindFlags() & BindFlags::VertexBuffer) != 0)
        {
            auto* vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, (*bufferArray++));
            vertexArrayGL2X_.BuildVertexLayout(vertexBufferGL->GetID(), vertexBufferGL->GetVertexAttribs());
        }
        else
            ThrowNoVertexBufferErr();
    }
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
