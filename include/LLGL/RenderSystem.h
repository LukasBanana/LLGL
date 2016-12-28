/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_H
#define LLGL_RENDER_SYSTEM_H


#include "Export.h"
#include "RenderContext.h"
#include "CommandBuffer.h"
#include "RenderSystemFlags.h"
#include "RenderingProfiler.h"
#include "RenderingDebugger.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "Texture.h"
#include "TextureArray.h"
#include "Sampler.h"
#include "SamplerArray.h"

#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Query.h"

#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


/**
\brief Render system interface.
\remarks This is the main interface for the entire renderer.
It manages the ownership of all graphics objects and is used to create, modify, and delete all those objects.
The main functions for most graphics objects are "Create...", "Write...", and "Release":
\code
// Create and initialize vertex buffer
LLGL::BufferDescriptor bufferDesc;
//fill descriptor ...
auto vertexBuffer = renderSystem->CreateBuffer(*buffer, bufferDesc, initialData);

// Modify data
renderSystem->WriteBuffer(*buffer, modificationData, ...);

// Release object
renderSystem->Release(*buffer);
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
        (e.g. on Win32 this might be { "OpenGL", "Direct3D11", "Direct3D12" }, but on MacOS it might be only { "OpenGL" }).
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
        \throws std::runtime_error If loading the render system from the specified module failed.
        \throws std::runtime_error If there is already a loaded instance of a render system
        (make sure there are no more shared pointer references to the previous render system!)
        */
        static std::unique_ptr<RenderSystem> Load(
            const std::string& moduleName,
            RenderingProfiler* profiler = nullptr,
            RenderingDebugger* debugger = nullptr
        );

        /**
        \brief Unloads the specified render system and the internal module.
        \remarks After this call, the specified render system and all the objects associated to it must no longer be used!
        */
        static void Unload(std::unique_ptr<RenderSystem>&& renderSystem);

        /**
        \brief Rendering API identification number.
        \remarks This can be a value of the RendererID entries.
        Since the render system is modular, a new render system can have its own ID number.
        \see RendererID
        */
        inline int GetRendererID() const
        {
            return rendererID_;
        }

        //! Returns the name of this render system.
        inline const std::string& GetName() const
        {
            return name_;
        }

        /**
        \brief Returns basic renderer information.
        \remarks The validity of these information is only guaranteed if this function is called
        after a valid render context has been created. Otherwise the behavior is undefined!
        */
        inline const RendererInfo& GetRendererInfo() const
        {
            return info_;
        }

        /**
        \brief Returns the rendering capabilities.
        \remarks The validity of these information is only guaranteed if this function is called
        after a valid render context has been created. Otherwise the behavior is undefined!
        */
        inline const RenderingCaps& GetRenderingCaps() const
        {
            return caps_;
        }

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
        \param[in] desc Specifies the render context descriptor, which contains the video mode, vsync, multi-sampling settings etc.
        \param[in] surface Optional shared pointer to a surface for the render context.
        If this is null, the render context will create its own platform specific surface, which can be accessed by "RenderContext::GetSurface".
        \remarks The render system takes the ownership of this object. All render contexts are deleted in the destructor of this render system.
        \see RenderContext::GetSurface
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) = 0;

        //! Releases the specified render context. This will all release all resources, that are associated with this render context.
        virtual void Release(RenderContext& renderContext) = 0;

        /* ----- Command buffers ----- */

        /**
        \brief Creates a new command buffer.
        \remarks Some render systems only support a single command buffer, such as OpenGL and Direct3D 11.
        */
        virtual CommandBuffer* CreateCommandBuffer() = 0;

        //! Releases the specified command buffer. After this call, the specified object must no longer be used.
        virtual void Release(CommandBuffer& commandBuffer) = 0;

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
        
        /**
        \brief Creates a new buffer array.
        \param[in] numBuffers Specifies the number of buffers in the array. This must be greater than 0.
        \param[in] bufferArray Pointer to an array of Buffer object pointers. This must not be null.
        \remarks This array can only contain buffers which are all from the same type, like an array of vertex buffers for instance.
        The buffers inside this array must persist as long as this buffer array is used,
        and the individual buffers are still required to read and write its data from and to the GPU.
        \throws std::invalid_argument If 'numBuffers' is 0, if 'bufferArray' is null,
        if any of the pointers in the array are null, if not all buffers have the same type, or if the buffer array type is
        not one of these: BufferType::Vertex, BufferType::Constant, BufferType::Storage, or BufferType::StreamOutput.
        */
        virtual BufferArray* CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) = 0;

        //! Releases the specified buffer object. After this call, the specified object must no longer be used.
        virtual void Release(Buffer& buffer) = 0;

        //! Releases the specified buffer array object. After this call, the specified object must no longer be used.
        virtual void Release(BufferArray& bufferArray) = 0;
        
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

        /**
        \brief Maps the specified buffer from GPU to CPU memory space.
        \param[in] buffer Specifies the buffer which is to be mapped.
        \param[in] access Specifies the CPU buffer access requirement, i.e. if the CPU can read and/or write the mapped memory.
        \return Raw pointer to the mapped memory block. You should be aware of the storage buffer size, to not cause memory violations.
        \see UnmapBuffer
        */
        virtual void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) = 0;

        /**
        \brief Unmaps the specified buffer.
        \see MapBuffer
        */
        virtual void UnmapBuffer(Buffer& buffer) = 0;

        /* ----- Textures ----- */

        /**
        \brief Creates a new texture.
        \param[in] textureDesc Specifies the texture descriptor.
        \param[in] imageDesc Optional pointer to the image data descriptor.
        If this is null, the texture will be initialized with the currently configured default image color.
        If this is non-null, it is used to initialize the texture data.
        This parameter will be ignored if the texture type is a multi-sampled texture (i.e. TextureType::Texture2DMS or TextureType::Texture2DMSArray).
        \see WriteTexture
        \see RenderSystemConfiguration::defaultImageColor
        */
        virtual Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) = 0;

        /**
        \brief Creates a new texture array.
        \param[in] numTextures Specifies the number of textures in the array. This must be greater than 0.
        \param[in] textureArray Pointer to an array of Texture object pointers. This must not be null.
        \remarks This texture array is not an "array texture" (like TextureType::Texture2DArray for instance).
        It is just a container of multiple texture objects, which can be used to bind several hardware textures at once, to improve performance.
        \throws std::invalid_argument If 'numTextures' is 0, if 'textureArray' is null,
        or if any of the pointers in the array are null.
        */
        virtual TextureArray* CreateTextureArray(unsigned int numTextures, Texture* const * textureArray) = 0;

        //! Releases the specified texture object. After this call, the specified object must no longer be used.
        virtual void Release(Texture& texture) = 0;

        //! Releases the specified texture array object. After this call, the specified object must no longer be used.
        virtual void Release(TextureArray& textureArray) = 0;

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
        \remarks This function can only be used for non-multi-sample textures
        (i.e. from types other than TextureType::Texture2DMS and TextureType::Texture2DMSArray),
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

        /**
        \brief Creates a new sampler array.
        \param[in] numSamplers Specifies the number of samplers in the array. This must be greater than 0.
        \param[in] samplerArray Pointer to an array of Sampler object pointers. This must not be null.
        \throws std::invalid_argument If 'numSamplers' is 0, if 'samplerArray' is null,
        or if any of the pointers in the array are null.
        */
        virtual SamplerArray* CreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray) = 0;

        //! Releases the specified Sampler object. After this call, the specified object must no longer be used.
        virtual void Release(Sampler& sampler) = 0;

        //! Releases the specified sampler array object. After this call, the specified object must no longer be used.
        virtual void Release(SamplerArray& samplerArray) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Creates a new RenderTarget object.
        \throws std::runtime_error If the renderer does not support RenderTarget objects (e.g. if OpenGL 2.1 or lower is used).
        */
        virtual RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) = 0;

        //! Releases the specified RenderTarget object. After this call, the specified object must no longer be used.
        virtual void Release(RenderTarget& renderTarget) = 0;

        /* ----- Shader ----- */

        /**
        \brief Creates a new and empty Shader object of the specified type.
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

        //! Releases the specified Shader object. After this call, the specified object must no longer be used.
        virtual void Release(Shader& shader) = 0;

        //! Releases the specified ShaderProgram object. After this call, the specified object must no longer be used.
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

        //! Releases the specified GraphicsPipeline object. After this call, the specified object must no longer be used.
        virtual void Release(GraphicsPipeline& graphicsPipeline) = 0;

        //! Releases the specified ComputePipeline object. After this call, the specified object must no longer be used.
        virtual void Release(ComputePipeline& computePipeline) = 0;

        /* ----- Queries ----- */

        //! Creates a new query.
        virtual Query* CreateQuery(const QueryDescriptor& desc) = 0;

        //! Releases the specified Query object. After this call, the specified object must no longer be used.
        virtual void Release(Query& query) = 0;

    protected:

        RenderSystem() = default;

        //! Sets the renderer information.
        void SetRendererInfo(const RendererInfo& info);

        //! Sets the rendering capabilities.
        void SetRenderingCaps(const RenderingCaps& caps);

        //! Creates an RGBA unsigned-byte image buffer for the specified number of pixels.
        std::vector<ColorRGBAub> GetDefaultTextureImageRGBAub(int numPixels) const;

        //! Validates the specified buffer descriptor to be used for buffer creation.
        void AssertCreateBuffer(const BufferDescriptor& desc);

        //! Validates the specified arguments to be used for buffer array creation.
        void AssertCreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray);

        //! Validates the specified arguments to be used for texture array creation.
        void AssertCreateTextureArray(unsigned int numTextures, Texture* const * textureArray);

        //! Validates the specified arguments to be used for sampler array creation.
        void AssertCreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray);

    private:

        int                         rendererID_ = 0;
        std::string                 name_;

        RendererInfo                info_;
        RenderingCaps               caps_;
        RenderSystemConfiguration   config_;

};


} // /namespace LLGL


#endif



// ================================================================================
