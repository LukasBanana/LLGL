/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_H
#define LLGL_RENDER_SYSTEM_H


#include "NonCopyable.h"
#include "RenderContext.h"
#include "CommandQueue.h"
#include "CommandBufferExt.h"
#include "RenderSystemFlags.h"
#include "RenderingProfiler.h"
#include "RenderingDebugger.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "Texture.h"
#include "Sampler.h"
#include "ResourceHeap.h"

#include "RenderTarget.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "PipelineLayout.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Query.h"
#include "Fence.h"

#include <string>
#include <memory>
#include <vector>
#include <cstdint>


namespace LLGL
{


/**
\brief Render system interface.
\remarks This is the main interface for the entire renderer.
It manages the ownership of all graphics objects and is used to create, modify, and delete all those objects.
The main functions for most graphics objects are "Create...", "Write...", "Read...", "Map...", "Unmap...", and "Release":
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
class LLGL_EXPORT RenderSystem : public NonCopyable
{

    public:

        /* ----- Common ----- */

        /**
        \brief Returns the list of all available render system modules for the current platform.
        \remarks For example, on Win32 this might be { "OpenGL", "Direct3D11", "Direct3D12" }, but on MacOS it might be only { "OpenGL" }.
        */
        static std::vector<std::string> FindModules();

        /**
        \brief Loads a new render system from the specified module.
        \param[in] renderSystemDesc Specifies the render system descriptor structure. The 'moduleName' member of this strucutre must not be empty.
        \param[in] profiler Optional pointer to a rendering profiler. If this is used, the counters of the profiler must be reset manually.
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \param[in] debugger Optional pointer to a rendering debugger.
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \remarks The descriptor structure can be initialized by only the module name like shown in the following example:
        \code
        // Load the "OpenGL" render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL");
        \endcode
        \throws std::runtime_error If loading the render system from the specified module failed.
        \see RenderSystemDescriptor::moduleName
        */
        static std::unique_ptr<RenderSystem> Load(
            const RenderSystemDescriptor&   renderSystemDesc,
            RenderingProfiler*              profiler            = nullptr,
            RenderingDebugger*              debugger            = nullptr
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
        inline const RenderingCapabilities& GetRenderingCaps() const
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
        If this is null, the render context will create its own platform specific surface, which can be accessed by RenderContext::GetSurface.
        The default surface is not shown automatically.
        \see RenderContext::GetSurface
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = {}) = 0;

        //! Releases the specified render context. This will all release all resources, that are associated with this render context.
        virtual void Release(RenderContext& renderContext) = 0;

        /* ----- Command queues ----- */

        //! Returns the single instance of the command queue.
        virtual CommandQueue* GetCommandQueue() = 0;

        /* ----- Command buffers ----- */

        /**
        \brief Creates a new command buffer.
        \remarks All render systems can create multiple command buffers,
        but especially for the legacy graphics APIs such as OpenGL and Direct3D 11, this doesn't provide any benefit,
        since all graphics and compute commands are submitted sequentially to the GPU.
        */
        virtual CommandBuffer* CreateCommandBuffer() = 0;

        /**
        \brief Creates a new extended command buffer (if supported) with dynamic state access for shader resources (i.e. Constant Buffers, Storage Buffers, Textures, and Samplers).
        \return Pointer to the new CommandBufferExt object, or null if the render system does not support extended command buffers.
        \remarks For those render systems that do not support dynamic state access for shader resources, use the ResourceHeap interface.
        \note Only supported with: OpenGL, Direct3D 11.
        \see RenderingCapabilities::hasCommandBufferExt
        \see CreateResourceHeap
        */
        virtual CommandBufferExt* CreateCommandBufferExt() = 0;

        /**
        \brief Releases the specified command buffer. After this call, the specified object must no longer be used.
        \remarks This can be used for both CommandBuffer and CommandBufferExt objects as the latter one inherits from the former one.
        \see CreateCommandBuffer
        \see CreateCommandBufferExt
        */
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
        \todo Rename to "CreateVertexArray".
        */
        virtual BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) = 0;

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
        \todo Maybe replace std::size_t with std::uint64_t here.
        */
        virtual void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;

        /**
        \brief Maps the specified buffer from GPU to CPU memory space.
        \param[in] buffer Specifies the buffer which is to be mapped.
        \param[in] access Specifies the CPU buffer access requirement, i.e. if the CPU can read and/or write the mapped memory.
        \return Raw pointer to the mapped memory block. You should be aware of the storage buffer size, to not cause memory violations.
        \see UnmapBuffer
        */
        virtual void* MapBuffer(Buffer& buffer, const CPUAccess access) = 0;

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
        \see RenderSystemConfiguration::imageInitialization
        */
        virtual Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) = 0;

        //! Releases the specified texture object. After this call, the specified object must no longer be used.
        virtual void Release(Texture& texture) = 0;

        /**
        \brief Updates the image data of the specified texture.
        \param[in] texture Specifies the texture whose data is to be updated.
        \param[in] subTextureDesc Specifies the sub-texture descriptor.
        \param[in] imageDesc Specifies the image data descriptor. Its "data" member must not be null!
        \remarks This function can only be used for non-multi-sample textures, i.e. from types other than TextureType::Texture2DMS and TextureType::Texture2DMSArray.
        */
        virtual void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc) = 0;

        /**
        \brief Reads the image data from the specified texture.
        \param[in] texture Specifies the texture object to read from.
        \param[in] mipLevel Specifies the MIP-level from which to read the texture data.
        \param[out] imageDesc Specifies the destination image descriptor to write the texture data to.
        \remarks The required size for a successful texture read operation depends on the image format, data type, and texture size.
        The Texture::QueryDesc or Texture::QueryMipExtent functions can be used to determine the texture dimensions.
        \code
        // Query texture size attribute
        auto myTextureExtent = myTexture->QueryMipExtent(0);

        // Allocate image buffer with elements in all dimensions
        std::vector<LLGL::ColorRGBAub> myImage(myTextureExtent.width * myTextureExtent.height * myTextureExtent.depth);

        // Initialize destination image descriptor
        const DstImageDescriptor myImageDesc {
            LLGL::ImageFormat::RGBA,                    // RGBA image format, since we used LLGL::ColorRGBAub
            LLGL::DataType::UInt8,                      // 8-bit unsigned integral data type: <std::uint8_t> or <unsigned char>
            myImage.data(),                             // Output image buffer
            myImage.size() * sizeof(LLGL::ColorRGBAub)  // Image buffer size: number of color elements and size of each color element
        };

        // Read texture data from first MIP-map level (index 0)
        myRenderSystem->ReadTexture(*myTexture, 0, myImageDesc);
        \endcode
        \note The behavior is undefined if 'imageDesc.data' points to an invalid buffer,
        or 'imageDesc.data' points to a buffer that is smaller than specified by 'imageDesc.dataSize',
        or 'imageDesc.dataSize' is less than the required size.
        \throws std::invalid_argument If 'imageDesc.data' is null.
        \see Texture::QueryDesc
        \see Texture::QueryMipExtent
        */
        virtual void ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc) = 0;

        /**
        \brief Generates all MIP-maps for the specified texture.
        \param[in,out] texture Specifies the texture whose MIP-maps are to be generated.
        \remarks To generate only a small amout of MIP levels, use the secondary 'GenerateMips' function.
        \see GenerateMips(Texture&, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
        \todo Maybe move this to "CommandBuffer" and no longer use the staging command buffer for this in Vulkan renderer.
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /**
        \brief Generates at least the specified range of MIP-maps for the specified texture.
        \param[in,out] texture Specifies the texture whose MIP-maps are to be generated.
        \param[in] baseMipLevel Specifies the zero-based index of the first MIP-map level.
        \param[in] numMipLevels Specifies the number of MIP-maps to generate. This also includes the base MIP-map level, so a number of less than 2 has no effect.
        \param[in] baseArrayLayer Specifies the zero-based index of the first array layer (if an array texture is used). By default 0.
        \param[in] numArrayLayers Specifies the number of array layers. For both array textures and non-array textures this must be at least 1. By default 1.
        \remarks This function only guarantees to generate at least the specified amount of MIP-maps.
        It may also update all other MIP-maps if the respective rendering API does not support hardware accelerated generation of a sub-range of MIP-maps.
        \note Only use this function if the range of MIP-maps is significantly smaller than the entire MIP chain,
        e.g. only a single slice of a large 2D array texture, and use the primary 'GenerateMips' function otherwise.
        \see GenerateMips(Texture&)
        \see NumMipLevels
        \todo Maybe move this to "CommandBuffer" and no longer use the staging command buffer for this in Vulkan renderer.
        */
        virtual void GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer = 0, std::uint32_t numArrayLayers = 1) = 0;

        /* ----- Samplers ---- */

        /**
        \brief Creates a new Sampler object.
        \throws std::runtime_error If the renderer does not support Sampler objects (e.g. if OpenGL 3.1 or lower is used).
        \see GetRenderingCaps
        */
        virtual Sampler* CreateSampler(const SamplerDescriptor& desc) = 0;

        //! Releases the specified Sampler object. After this call, the specified object must no longer be used.
        virtual void Release(Sampler& sampler) = 0;

        /* ----- Resource Heaps ----- */

        /**
        \brief Creates a new resource heap.
        \param[in] desc Specifies the descriptor which determines all shader resource.
        \remarks Resources heaps are used in combination with a pipeline layout.
        The pipeline layout determines to which binding points the resources are bound.
        \see CreatePipelineLayout
        \see CommandBuffer::SetGraphicsResourceHeap
        \see CommandBuffer::SetComputeResourceHeap
        */
        virtual ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& desc) = 0;

        //! Releases the specified ResourceHeap object. After this call, the specified object must no longer be used.
        virtual void Release(ResourceHeap& resourceHeap) = 0;

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
        \brief Creates a new and Shader object and compiles the specified source.
        \remarks To check whether the compilation was successful or not, use the 'HasErrors' and 'QueryInfoLog' functions of the Shader interface.
        \see Shader::HasErrors
        \see Shader::QueryInfoLog
        \see ShaderDescriptor
        \see ShaderDescFromFile
        */
        virtual Shader* CreateShader(const ShaderDescriptor& desc) = 0;

        /**
        \brief Creates a new shader program and links all specified shaders.
        \remarks To check whether the linking was successful or not, use the 'HasErrors' and 'QueryInfoLog' functions of the ShaderProgram interface.
        \see ShaderProgram::HasErrors
        \see ShaderProgram::QueryInfoLog
        \see ShaderProgramDescriptor
        \see ShaderProgramDesc
        */
        virtual ShaderProgram* CreateShaderProgram(const ShaderProgramDescriptor& desc) = 0;

        //! Releases the specified Shader object. After this call, the specified object must no longer be used.
        virtual void Release(Shader& shader) = 0;

        //! Releases the specified ShaderProgram object. After this call, the specified object must no longer be used.
        virtual void Release(ShaderProgram& shaderProgram) = 0;

        /* ----- Pipeline Layouts ----- */

        /**
        \brief Creates a new and initialized pipeline layout object, if and only if the renderer supports pipeline layouts.
        \param[in] desc Specifies the pipeline layout descriptor with all layout bindings.
        \remarks A pipeline layout is required in combination with a ResourceHeap to bind multiple resources at once.
        For modern graphics APIs (i.e. Direct3D 12 and Vulkan), this is only way to bind shader resources.
        For legacy graphics APIs (i.e. Direct3D 11 and OpenGL), shader resources can also be bound individually with the extended command buffer.
        \return Pointer to the new PipelineLayout object or null if the renderer does not support pipeline layouts.
        \see CommandBufferExt
        \see CreateResourceHeap
        */
        virtual PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& desc) = 0;

        //! Releases the specified PipelineLayout object. After this call, the specified object must no longer be used.
        virtual void Release(PipelineLayout& pipelineLayout) = 0;

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

        /* ----- Fences ----- */

        /**
        \brief Creates a new fence (used for CPU/GPU synchronization).
        \see CommandBuffer::SubmitFence
        \see CommandBuffer::WaitFence
        */
        virtual Fence* CreateFence() = 0;

        //! Releases the specified Fence object. After this call, the specified object must no longer be used.
        virtual void Release(Fence& fence) = 0;

    protected:

        RenderSystem() = default;

        //! Sets the renderer information.
        void SetRendererInfo(const RendererInfo& info);

        //! Sets the rendering capabilities.
        void SetRenderingCaps(const RenderingCapabilities& caps);

        //! Validates the specified buffer descriptor to be used for buffer creation.
        void AssertCreateBuffer(const BufferDescriptor& desc, std::uint64_t maxSize);

        //! Validates the specified arguments to be used for buffer array creation.
        void AssertCreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        //! Validates the specified shader descriptor.
        void AssertCreateShader(const ShaderDescriptor& desc);

        //! Validates the specified shader program descriptor.
        void AssertCreateShaderProgram(const ShaderProgramDescriptor& desc);

        //! Validates the specified image data size against the required size (in bytes).
        void AssertImageDataSize(std::size_t dataSize, std::size_t requiredDataSize, const char* info = nullptr);

    private:

        int                         rendererID_ = 0;
        std::string                 name_;

        RendererInfo                info_;
        RenderingCapabilities       caps_;
        RenderSystemConfiguration   config_;

};


} // /namespace LLGL


#endif



// ================================================================================
