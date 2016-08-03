/*
 * GLRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_SYSTEM_H__
#define __LLGL_GL_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include "GLRenderContext.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"
#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


class GLRenderSystem : public RenderSystem
{

    public:

        /* ----- Render system ----- */

        GLRenderSystem();
        ~GLRenderSystem();

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        /* ----- Hardware buffers ------ */

        VertexBuffer* CreateVertexBuffer() override;
        IndexBuffer* CreateIndexBuffer() override;

        void WriteVertexBuffer(
            VertexBuffer& vertexBuffer,
            const void* data,
            std::size_t dataSize,
            const BufferUsage usage,
            const VertexFormat& vertexFormat
        ) override;

        void WriteIndexBuffer(
            VertexBuffer& vertexBuffer,
            const void* data,
            std::size_t dataSize,
            const BufferUsage usage,
            const IndexFormat& indexFormat
        ) override;

    private:

        bool OnMakeCurrent(RenderContext* renderContext) override;

        std::set<std::unique_ptr<GLRenderContext>>  renderContexts_;
        std::set<std::unique_ptr<GLVertexBuffer>>   vertexBuffers_;
        std::set<std::unique_ptr<GLIndexBuffer>>    indexBuffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
