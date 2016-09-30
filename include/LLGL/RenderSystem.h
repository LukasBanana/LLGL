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
#include "RenderingDebugger.h"

#include "Buffer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Sampler.h"
#include "Query.h"

#include <string>
#include <memory>
#include <vector>

//TODO: remove
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "StorageBuffer.h"


namespace LLGL
{


/**
\brief Render system interface.
\remarks This is the main interface for the entire renderer.
It manages the ownership of all graphics objects and is used to create, modify, and delete all those objects.
The main functions for most graphics objects are "Create...", "Write...", and "Release":
\code
// Create and initialize vertex buffer
auto vertexBuffer = renderSystem->CreateVertexBuffer(*vertexBuffer, ...);

// Modify data
renderSystem->WriteVertexBuffer(*vertexBuffer, modificationData, ...);

// Release object
renderSystem->Release(*vertexBuffer);
\endcode
*/
class LLGL_EXPORT RenderSystem
{

    public:

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
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \param[in] debugger Optional pointer to a rendering debugger.
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \remarks Usually the return type is a std::unique_ptr, but LLGL needs to keep track
        of the existance of this render system because only a single instance can be loaded at a time.
        So a std::weak_ptr is stored internally to check if it has been expired
        (see http://en.cppreference.com/w/cpp/memory/weak_ptr/expired),
        and this type can only refer to a std::shared_ptr.
        \throws std::runtime_error If loading the render system from the specified module failed.
        \throws std::runtime_error If there is already a loaded instance of a render system
        (make sure there are no more shared pointer references to the previous render system!)
        */
        static std::shared_ptr<RenderSystem> Load(
            const std::string& moduleName,
            RenderingProfiler* profiler = nullptr,
            RenderingDebugger* debugger = nullptr
        );

        //! Returns the name of this render system.
        inline const std::string& GetName() const
        {
            return name_;
        }

        //! Returns all available renderer information.
        virtual std::map<RendererInfo, std::string> QueryRendererInfo() const = 0;

        //! Returns the rendering capabilities.
        virtual RenderingCaps QueryRenderingCaps() const = 0;

        //! Returns the highest version of the supported shading language.
        virtual ShadingLanguage QueryShadingLanguage() const = 0;

        /**
        \brief Sets the basic configuration.
        \remarks This can be used to change the behavior of default initializion of textures for instance.
        \see RenderSystemConfiguration
        */
        virtual void SetConfiguration(const RenderSystemConfiguration& config);

        /**
        \brief Returns the basic configuration.
        \see SetConfiguration
        */
        inline const RenderSystemConfiguration& GetConfiguration() const
        {
            return config_;
        }

        /* ----- Render Context ----- */

        /**
        \brief Creates a new render context and returns the raw pointer.
        \remarks The render system takes the ownership of this object. All render contexts are deleted in the destructor of this render system.
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) = 0;

        //! Releases the specified render context. This will all release all resources, that are associated with this render context.
        virtual void Release(RenderContext& renderContext) = 0;

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

        /* ----- Buffers ------ */

        /**
        \brief Creates a new generic hardware buffer.
        \param[in] desc Specifies the vertex buffer descriptor.
        \param[in] initialData Optional raw pointer to the data with which the buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteBuffer" function before it is used for drawing operations. By default null.
        \see WriteBuffer
        */
        virtual Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) = 0;
        
        //! Releases the specified buffer object.
        virtual void Release(Buffer& buffer) = 0;
        
        /**
        \brief Updates the data of the specified buffer.
        \param[in] buffer Specifies the buffer whose data is to be updated.
        \param[in] data Raw pointer to the data with which the buffer is to be updated. This must not be null!
        \param[in] dataSize Specifies the size (in bytes) of the data block which is to be updated.
        This must be less then or equal to the size of the buffer.
        \param[in] offset Specifies the offset (in bytes) at which the buffer is to be updated.
        This offset plus the data block size (i.e. 'offset + dataSize') must be less than or equal to the size of the buffer.
        */
        virtual void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;

        #if 1//TODO: remove
        virtual VertexBuffer* CreateVertexBuffer(const VertexBufferDescriptor& desc, const void* initialData = nullptr) = 0;
        virtual IndexBuffer* CreateIndexBuffer(const IndexBufferDescriptor& desc, const void* initialData = nullptr) = 0;
        virtual ConstantBuffer* CreateConstantBuffer(const ConstantBufferDescriptor& desc, const void* initialData = nullptr) = 0;
        virtual StorageBuffer* CreateStorageBuffer(const StorageBufferDescriptor& desc, const void* initialData = nullptr) = 0;

        virtual void Release(VertexBuffer& vertexBuffer) = 0;
        virtual void Release(IndexBuffer& indexBuffer) = 0;
        virtual void Release(ConstantBuffer& constantBuffer) = 0;
        virtual void Release(StorageBuffer& storageBuffer) = 0;

        virtual void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        virtual void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        virtual void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        virtual void WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        #endif

        /* ----- Textures ----- */

        /**
        \brief Creates a new texture.
        \param[in] textureDesc Specifies the texture descriptor.
        \param[in] imageDesc Optional pointer to the image data descriptor.
        If this is null, the texture will be initialized with the currently configured default image color.
        If this is non-null, it is used to initialize the texture data.
        \remarks If the texture type of the descriptor is not an  array texture the number of layers will be ignored.
        \see WriteTexture
        \see RenderSystemConfiguration::defaultImageColor
        */
        virtual Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) = 0;

        virtual void Release(Texture& texture) = 0;

        /**
        \brief Queries a descriptor of the specified texture.
        \remarks This can be used to query the type and dimension size of the texture.
        \see TextureDescriptor
        */
        virtual TextureDescriptor QueryTextureDescriptor(const Texture& texture) = 0;
        
        /**
        \brief Updates the image data of the specified texture.
        \param[in] texture Specifies the texture whose data is to be updated.
        \param[in] subTextureDesc Specifies the sub-texture descriptor.
        \param[in] imageDesc Specifies the image data descriptor. Its "data" member must not be null!
        */
        virtual void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) = 0;

        /**
        \brief Reads the image data from the specified texture.
        \param[in] texture Specifies the texture object to read from.
        \param[in] mipLevel Specifies the MIP-level from which to read the image data.
        \param[in] imageFormat Specifies the output image format.
        \param[in] dataType Specifies the output data type.
        \param[out] buffer Specifies the output image buffer. This must be a pointer to a memory block, which is large enough to fit all the image data.
        \remarks Depending on the image format, data type, and texture size, the output image container must be allocated with enough memory size.
        The "QueryTextureDescriptor" function can be used to determine the texture dimensions.
        \code
        std::vector<LLGL::ColorRGBAub> image(textureWidth*textureHeight);
        renderSystem->ReadTexture(texture, 0, LLGL::ImageFormat::RGBA, LLGL::DataType::UInt8, image.data());
        \endcode
        \see QueryTextureDescriptor
        */
        virtual void ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer) = 0;

        /**
        \brief Generates the MIP ("Multum in Parvo") maps for the specified texture.
        \see https://developer.valvesoftware.com/wiki/MIP_Mapping
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /* ----- Samplers ---- */

        /**
        \brief Creates a new Sampler object.
        \throws std::runtime_error If the renderer does not support Sampler objects (e.g. if OpenGL 3.1 or lower is used).
        \see RenderContext::QueryRenderingCaps
        */
        virtual Sampler* CreateSampler(const SamplerDescriptor& desc) = 0;

        //! Releases the specified Sampler object. After this call, the specified object must no longer be used.
        virtual void Release(Sampler& sampler) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Creates a new RenderTarget object with the specified number of samples.
        \throws std::runtime_error If the renderer does not support RenderTarget objects (e.g. if OpenGL 2.1 or lower is used).
        */
        virtual RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) = 0;

        //! Releases the specified RenderTarget object. After this call, the specified object must no longer be used.
        virtual void Release(RenderTarget& renderTarget) = 0;

        /* ----- Shader ----- */

        /**
        \brief Creates a new and empty shader.
        \param[in] type Specifies the type of the shader, i.e. if it is either a vertex or fragment shader or the like.
        \see Shader
        */
        virtual Shader* CreateShader(const ShaderType type) = 0;

        /**
        \brief Creates a new and empty shader program.
        \remarks At least one shader must be attached to a shader program to be used for a graphics or compute pipeline.
        \see ShaderProgram
        */
        virtual ShaderProgram* CreateShaderProgram() = 0;

        virtual void Release(Shader& shader) = 0;
        virtual void Release(ShaderProgram& shaderProgram) = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Creates a new and initialized graphics pipeline state object.
        \param[in] desc Specifies the graphics pipeline descriptor.
        This will describe the entire pipeline state, i.e. the blending-, rasterizer-, depth-, stencil- and shader states.
        The "shaderProgram" member of the descriptor must never be null!
        \see GraphicsPipelineDescriptor
        */
        virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) = 0;

        /**
        \brief Creates a new and initialized compute pipeline state object.
        \param[in] desc Specifies the compute pipeline descriptor. This will describe the shader states.
        The "shaderProgram" member of the descriptor must never be null!
        \see ComputePipelineDescriptor
        */
        virtual ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) = 0;

        virtual void Release(GraphicsPipeline& graphicsPipeline) = 0;
        virtual void Release(ComputePipeline& computePipeline) = 0;

        /* ----- Queries ----- */

        //! Creates a new query.
        virtual Query* CreateQuery(const QueryDescriptor& desc) = 0;

        virtual void Release(Query& query) = 0;

    protected:

        RenderSystem() = default;

        /**
        \brief Callback when a new render context is about to be made the current one.
        \remarks At this point, "GetCurrentContext" returns still the previous render context.
        */
        virtual bool OnMakeCurrent(RenderContext* renderContext);

        //! Creates an RGBA unsigned-byte image buffer for the specified number of pixels.
        std::vector<ColorRGBAub> GetDefaultTextureImageRGBAub(int numPixels) const;

        //! Validates the specified buffer descriptor to be used for buffer creation.
        void AssertCreateBuffer(const BufferDescriptor& desc);

    private:

        std::string                 name_;

        RenderContext*              currentContext_ = nullptr;

        RenderSystemConfiguration   config_;

};


} // /namespace LLGL


#endif



// ================================================================================
