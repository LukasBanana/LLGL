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
#include "Shader/GLShader.h"
#include "Shader/GLShaderProgram.h"
#include "Texture/GLTexture.h"

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

        /* ----- Textures ----- */

        Texture* CreateTexture() override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;

        void WriteTexture1D(Texture& texture, const TextureFormat format, int width, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTexture2D(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTexture3D(Texture& texture, const TextureFormat format, int width, int height, int depth, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTextureCube(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTexture1DArray(Texture& texture, const TextureFormat format, int width, unsigned int layers, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTexture2DArray(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor* textureData = nullptr) override;
        void WriteTextureCubeArray(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor* textureData = nullptr) override;
        
        void WriteTexture1DSub(Texture& texture, const TextureFormat format, int width, const TextureDataDescriptor& textureData) override;
        void WriteTexture2DSub(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor& textureData) override;
        void WriteTexture3DSub(Texture& texture, const TextureFormat format, int width, int height, int depth, const TextureDataDescriptor& textureData) override;
        void WriteTextureCubeSub(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor& textureData) override;
        void WriteTexture1DArraySub(Texture& texture, const TextureFormat format, int width, unsigned int layers, const TextureDataDescriptor& textureData) override;
        void WriteTexture2DArraySub(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor& textureData) override;
        void WriteTextureCubeArraySub(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor& textureData) override;

        void ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderType type) override;
        ShaderProgram* CreateShaderProgram() override;

    protected:

        RenderContext* AddRenderContext(
            std::unique_ptr<GLRenderContext>&& renderContext,
            const RenderContextDescriptor& desc,
            const std::shared_ptr<Window>& window
        );

    private:

        template <typename T>
        using HWObjectContainer = std::set<std::unique_ptr<T>>;

        bool OnMakeCurrent(RenderContext* renderContext) override;

        void LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc);
        void SetDebugCallback(const DebugCallback& debugCallback);

        void BindTextureAndSetType(GLTexture& textureGL, const TextureType type);

        /* ----- Common GL render system objects ----- */

        OpenGLExtensionMap                          extensionMap_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<GLRenderContext>          renderContexts_;
        
        HWObjectContainer<GLVertexBuffer>           vertexBuffers_;
        HWObjectContainer<GLIndexBuffer>            indexBuffers_;
        HWObjectContainer<GLConstantBuffer>         constantBuffers_;

        HWObjectContainer<GLTexture>                textures_;

        HWObjectContainer<GLShader>                 shaders_;
        HWObjectContainer<GLShaderProgram>          shaderPrograms_;

};


} // /namespace LLGL


#endif



// ================================================================================
