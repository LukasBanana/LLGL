/*
 * GLRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_SYSTEM_H__
#define __LLGL_GL_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include "GLExtensionLoader.h"
#include "GLRenderContext.h"

#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLConstantBuffer.h"

#include "Shader/GLVertexShader.h"
#include "Shader/GLFragmentShader.h"
#include "Shader/GLGeometryShader.h"
#include "Shader/GLTessControlShader.h"
#include "Shader/GLTessEvaluationShader.h"
#include "Shader/GLComputeShader.h"
#include "Shader/GLShaderProgram.h"

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
        ConstantBuffer* CreateConstantBuffer() override;

        void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) override;
        void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) override;
        void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;

        void WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        /* ----- Shader ----- */

        VertexShader* CreateVertexShader() override;
        FragmentShader* CreateFragmentShader() override;
        GeometryShader* CreateGeometryShader() override;
        TessControlShader* CreateTessControlShader() override;
        TessEvaluationShader* CreateTessEvaluationShader() override;
        ComputeShader* CreateComputeShader() override;

        ShaderProgram* CreateShaderProgram() override;

    private:

        template <typename T>
        using HWObjectContainer = std::set<std::unique_ptr<T>>;

        bool OnMakeCurrent(RenderContext* renderContext) override;

        void LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc);

        /* ----- Common GL render system objects ----- */

        OpenGLExtensionMap                          extensionMap_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<GLRenderContext>          renderContexts_;
        
        HWObjectContainer<GLVertexBuffer>           vertexBuffers_;
        HWObjectContainer<GLIndexBuffer>            indexBuffers_;
        HWObjectContainer<GLConstantBuffer>         constantBuffers_;

        HWObjectContainer<GLVertexShader>           vertexShaders_;
        HWObjectContainer<GLFragmentShader>         fragmentShaders_;
        HWObjectContainer<GLGeometryShader>         geometryShaders_;
        HWObjectContainer<GLTessControlShader>      tessControlShaders_;
        HWObjectContainer<GLTessEvaluationShader>   tessEvaluationShaders_;
        HWObjectContainer<GLComputeShader>          computeShaders_;

        HWObjectContainer<GLShaderProgram>          shaderPrograms_;

};


} // /namespace LLGL


#endif



// ================================================================================
