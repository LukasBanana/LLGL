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

#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"

#include "GLVertexShader.h"
#include "GLFragmentShader.h"
#include "GLGeometryShader.h"
#include "GLTessControlShader.h"
#include "GLTessEvaluationShader.h"
#include "GLComputeShader.h"
#include "GLShaderProgram.h"

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
            IndexBuffer& indexBuffer,
            const void* data,
            std::size_t dataSize,
            const BufferUsage usage,
            const IndexFormat& indexFormat
        ) override;

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
