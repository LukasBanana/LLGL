/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_H__
#define __LLGL_RENDER_SYSTEM_H__


#include "Export.h"
#include "RenderContext.h"
#include "RenderSystemFlags.h"
#include "RenderingProfiler.h"

#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "IndexBuffer.h"
#include "IndexFormat.h"
#include "ConstantBuffer.h"
#include "GraphicsPipeline.h"

#include "Texture.h"
#include "RenderTarget.h"
#include "ShaderProgram.h"

#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


//! Render system interface.
class LLGL_EXPORT RenderSystem
{

    public:

        //! Render system configuration structure.
        struct Configuration
        {
            /**
            \brief Default color for an uninitialized texture. The default value is white (255, 255, 255, 255).
            \remarks This will be used for each "WriteTexture..." function, when no initial image data is specified.
            */
            ColorRGBAub defaultTextureImageColor;
        };

        /* ----- Common ----- */

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator = (const RenderSystem&) = delete;

        virtual ~RenderSystem();

        /**
        \brief Returns the list of all available render system modules for the current platform
        (e.g. on Windows this might be { "OpenGL", "Direct3D12" }, but on MacOS it might be only { "OpenGL" }).
        */
        static std::vector<std::string> FindModules();

        /**
        \brief Loads a new render system from the specified module.
        \param[in] moduleName Specifies the name from which the new render system is to be loaded.
        This denotes a dynamic library (*.dll-files on Windows, *.so-files on Unix systems).
        If compiled in debug mode, the postfix "D" is appended to the module name.
        Moreover, the platform dependent file extension is always added automatically
        as well as the prefix "LLGL_", i.e. a module name "OpenGL" will be
        translated to "LLGL_OpenGLD.dll", if compiled on Windows in Debug mode.
        \param[in] profiler Optional pointer to a rendering profiler. If this is used, the counters of the profiler must be reset manually.
        \remarks Usually the return type is a std::unique_ptr, but LLGL needs to keep track
        of the existance of this render system because only a single instance can be loaded at a time.
        So a std::weak_ptr is stored internally to check if it has been expired
        (see http://en.cppreference.com/w/cpp/memory/weak_ptr/expired),
        and this type can only refer to a std::shared_ptr.
        \throws std::runtime_exception If loading the render system from the specified module failed.
        \throws std::runtime_exception If there is already a loaded instance of a render system
        (make sure there are no more shared pointer references to the previous render system!)
        */
        static std::shared_ptr<RenderSystem> Load(const std::string& moduleName, RenderingProfiler* profiler = nullptr);

        //! Returns the name of this render system.
        inline const std::string& GetName() const
        {
            return name_;
        }

        /* ----- Render Context ----- */

        /**
        \brief Creates a new render context and returns the raw pointer.
        \remarks The render system takes the ownership of this object. All render contexts are deleted in the destructor of this render system.
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) = 0;

        /**
        \brief Makes the specified render context to the current one.
        \param[in] renderContext Specifies the new current render context. If this is null, no render context is active.
        \return True on success, otherwise false.
        \remarks Never draw anything, while no render context is active!
        */
        bool MakeCurrent(RenderContext* renderContext);

        //! Returns the current render context. This may also be null.
        inline RenderContext* GetCurrentContext() const
        {
            return currentContext_;
        }

        /* ----- Hardware Buffers ------ */

        virtual VertexBuffer* CreateVertexBuffer() = 0;
        virtual IndexBuffer* CreateIndexBuffer() = 0;
        virtual ConstantBuffer* CreateConstantBuffer() = 0;
        //virtual StorageBuffer* CreateStorageBuffer() = 0;

        //virtual void Release(VertexBuffer& vertexBuffer) = 0;
        //virtual void Release(IndexBuffer& indexBuffer) = 0;

        virtual void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) = 0;
        virtual void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) = 0;
        virtual void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) = 0;

        virtual void WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        virtual void WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        virtual void WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;

        /* ----- Textures ----- */

        virtual Texture* CreateTexture() = 0;
        //virtual void Release(Texture& texture) = 0;

        virtual TextureDescriptor QueryTextureDescriptor(const Texture& texture) = 0;

        virtual void WriteTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        virtual void WriteTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        
        virtual void WriteTexture1DSub(Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTexture2DSub(Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTexture3DSub(Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTextureCubeSub(Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTexture1DArraySub(Texture& texture, int mipLevel, int position, unsigned int layers, int size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTexture2DArraySub(Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) = 0;
        virtual void WriteTextureCubeArraySub(Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) = 0;

        /**
        \brief Reads the image data from the specified texture.
        \param[in] texture Specifies the texture object to read from.
        \param[in] mipLevel Specifies the MIP-level from which to read the image data.
        \param[in] dataFormat Specifies the output data format.
        \param[in] dataType Specifies the output data type.
        \param[out] data Specifies the output image data. This must be a pointer to a memory block, which is large enough to fit all the image data.
        \remarks Depending on the data format, data type, and texture size, the output image container must be allocated with enough memory size.
        The "QueryTextureDescriptor" function can be used to determine the texture dimensions.
        \code
        std::vector<LLGL::ColorRGBAub> image(textureWidth*textureHeight);
        renderSystem->ReadTexture(texture, 0, LLGL::ColorFormat::RGBA, LLGL::DataType::UByte, image.data());
        \endcode
        \see QueryTextureDescriptor
        */
        virtual void ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data) = 0;

        /* ----- Render Targets ----- */

        virtual RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) = 0;

        /* ----- Shader ----- */

        virtual Shader* CreateShader(const ShaderType type) = 0;
        virtual ShaderProgram* CreateShaderProgram() = 0;

        /* ----- Pipeline States ----- */

        virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) = 0;
        //virtual ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) = 0;

        /* === Members === */

        /**
        \brief Render system basic configuration.
        \remarks This can be used to change the behavior of default initializion of textures for instance.
        */
        Configuration config;

    protected:

        RenderSystem() = default;

        /**
        \brief Callback when a new render context is about to be made the current one.
        \remarks At this point, "GetCurrentContext" returns still the previous render context.
        */
        virtual bool OnMakeCurrent(RenderContext* renderContext);

        //! Creates an RGBA unsigned-byte image buffer for the specified number of pixels.
        std::vector<ColorRGBAub> GetDefaultTextureImageRGBAub(int numPixels) const;

    private:

        std::string     name_;

        RenderContext*  currentContext_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
