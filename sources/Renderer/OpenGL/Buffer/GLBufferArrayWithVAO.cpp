/*
 * GLBufferArrayWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferArrayWithVAO.h"
#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferArrayWithVAO::GLBufferArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    GLBufferArray { numBuffers, bufferArray }
{
    /* Build vertex array and finalize afterwards as it references multiple buffers */
    while (GLBuffer* bufferGL = NextArrayResource<GLBuffer>(numBuffers, bufferArray))
    {
        if ((bufferGL->GetBindFlags() & BindFlags::VertexBuffer) != 0)
        {
            /* Bind VBO and build vertex layout */
            auto* vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, bufferGL);
            GLStateManager::Get().BindBuffer(GLBufferTarget::ArrayBuffer, vertexBufferGL->GetID());
            vertexArray_.BuildVertexLayout(vertexBufferGL->GetID(), vertexBufferGL->GetVertexAttribs());
        }
        else
        {
            LLGL_TRAP("cannot build vertex array with buffer that was not created with the 'LLGL::BindFlags::VertexBuffer' flag");
        }
    }
    vertexArray_.Finalize();
}

void GLBufferArrayWithVAO::SetDebugName(const char* name)
{
    vertexArray_.SetDebugName(name);
}


} // /namespace LLGL



// ================================================================================
