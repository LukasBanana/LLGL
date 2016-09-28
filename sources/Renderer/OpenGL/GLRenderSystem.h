/*
 * GLRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_SYSTEM_H__
#define __LLGL_GL_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include "Ext/GLExtensionLoader.h"
#include "GLRenderContext.h"
#include "../ContainerTypes.h"

#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLConstantBuffer.h"
#include "Buffer/GLStorageBuffer.h"

#include "Shader/GLShader.h"
#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLRenderTarget.h"
#include "Texture/GLSampler.h"

#include "RenderState/GLQuery.h"
#include "RenderState/GLGraphicsPipeline.h"
#include "RenderState/GLComputePipeline.h"

#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


#define LLGL_ASSERT_CAP(FEATURE) \
    AssertCap(renderingCaps_.FEATURE, #FEATURE)


class GLRenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        GLRenderSystem();
        ~GLRenderSystem();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        RenderingCaps QueryRenderingCaps() const override;

        ShadingLanguage QueryShadingLanguage() const override;

        /* ----- Render Context ----- */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Hardware Buffers ------ */

        VertexBuffer* CreateVertexBuffer() override;
        IndexBuffer* CreateIndexBuffer() override;
        ConstantBuffer* CreateConstantBuffer() override;
        StorageBuffer* CreateStorageBuffer() override;

        void Release(VertexBuffer& vertexBuffer) override;
        void Release(IndexBuffer& indexBuffer) override;
        void Release(ConstantBuffer& constantBuffer) override;
        void Release(StorageBuffer& storageBuffer) override;

        void SetupVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) override;
        void SetupIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) override;
        void SetupConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;
        void SetupStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;

        void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& desc, const ImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;

        void SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc = nullptr) override;
        void SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc = nullptr) override;
        
        void WriteTexture1D(Texture& texture, int mipLevel, int position, int size, const ImageDescriptor& imageDesc) override;
        void WriteTexture2D(Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDescriptor& imageDesc) override;
        void WriteTexture3D(Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDescriptor& imageDesc) override;

        void WriteTextureCube(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace,
            const Gs::Vector2i& size, const ImageDescriptor& imageDesc
        ) override;
        
        void WriteTexture1DArray(
            Texture& texture, int mipLevel, int position, unsigned int layerOffset,
            int size, unsigned int layers, const ImageDescriptor& imageDesc
        ) override;
        
        void WriteTexture2DArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
            const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor& imageDesc
        ) override;

        void WriteTextureCubeArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
            const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDescriptor& imageDesc
        ) override;

        void ReadTexture(const Texture& texture, int mipLevel, ImageFormat dataFormat, DataType dataType, void* data) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;

        void Release(Sampler& sampler) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderType type) override;
        ShaderProgram* CreateShaderProgram() override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;

        /* ----- Pipeline States ----- */

        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) override;
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;
        
        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryDescriptor& desc) override;

        void Release(Query& query) override;

        /* ----- Extended Internal Functions ----- */

        bool HasExtension(const std::string& name) const;

        inline const RenderingCaps& GetRenderingCaps() const
        {
            return renderingCaps_;
        }

    protected:

        RenderContext* AddRenderContext(
            std::unique_ptr<GLRenderContext>&& renderContext,
            const RenderContextDescriptor& desc,
            const std::shared_ptr<Window>& window
        );

    private:

        bool OnMakeCurrent(RenderContext* renderContext) override;

        void LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc);
        void SetDebugCallback(const DebugCallback& debugCallback);

        void BindTextureAndSetType(GLTexture& textureGL, const TextureType type);

        void StoreRenderingCaps();
        void AssertCap(bool supported, const std::string& memberName);

        /* ----- Common objects ----- */

        OpenGLExtensionMap                      extensionMap_;

        RenderingCaps                           renderingCaps_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<GLRenderContext>      renderContexts_;
        
        HWObjectContainer<GLVertexBuffer>       vertexBuffers_;
        HWObjectContainer<GLIndexBuffer>        indexBuffers_;
        HWObjectContainer<GLConstantBuffer>     constantBuffers_;
        HWObjectContainer<GLStorageBuffer>      storageBuffers_;

        HWObjectContainer<GLTexture>            textures_;
        HWObjectContainer<GLRenderTarget>       renderTargets_;

        HWObjectContainer<GLShader>             shaders_;
        HWObjectContainer<GLShaderProgram>      shaderPrograms_;

        HWObjectContainer<GLGraphicsPipeline>   graphicsPipelines_;
        HWObjectContainer<GLComputePipeline>    computePipelines_;
        HWObjectContainer<GLSampler>            samplers_;
        HWObjectContainer<GLQuery>              queries_;

};


} // /namespace LLGL


#endif



// ================================================================================
