/*
 * GLBufferArrayWithVAO.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

void GLBufferArrayWithVAO::SetName(const char* name)
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
        while (auto bufferGL = NextArrayResource<GLBuffer>(numBuffers, bufferArray))
        {
            if ((bufferGL->GetBindFlags() & BindFlags::VertexBuffer) != 0)
            {
                /* Bind VBO */
                auto vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, bufferGL);
                GLStateManager::Get().BindBuffer(GLBufferTarget::ARRAY_BUFFER, vertexBufferGL->GetID());

                /* Build each vertex attribute */
                const auto& vertexAttribs = vertexBufferGL->GetVertexAttribs();
                for (const auto& attrib : vertexAttribs)
                    vao_.BuildVertexAttribute(attrib);
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
            auto vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, (*bufferArray++));

            /* Build each vertex attribute */
            const auto& vertexAttribs = vertexBufferGL->GetVertexAttribs();
            for (const auto& attrib : vertexAttribs)
                vertexArrayGL2X_.BuildVertexAttribute(vertexBufferGL->GetID(), attrib);
        }
        else
            ThrowNoVertexBufferErr();
    }
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
