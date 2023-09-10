/*
 * RenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_H
#define LLGL_RENDER_SYSTEM_H


#include <LLGL/Interface.h>
#include <LLGL/SwapChain.h>
#include <LLGL/CommandQueue.h>
#include <LLGL/CommandBuffer.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>
#include <LLGL/Blob.h>
#include <LLGL/Container/ArrayView.h>

#include <LLGL/Buffer.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/BufferArray.h>
#include <LLGL/Texture.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Sampler.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/Shader.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/QueryHeap.h>
#include <LLGL/QueryHeapFlags.h>
#include <LLGL/Fence.h>

#include <string>
#include <memory>
#include <vector>
#include <cstdint>


namespace LLGL
{


class RenderSystem;


/**
\brief Delegate to delete an instance of the RenderSystem interface.
\remarks This deleter keeps a function pointer to the actual deleter from the renderer module.
If no function pointer is provided, the deleter uses the C++ \c delete operator by default.
\see RenderSystem::Load
\see RenderSystem::Unload
*/
class RenderSystemDeleter
{

    public:

        #ifdef _WIN32
        typedef void (__cdecl *RenderSystemDeleterFuncPtr)(void*);
        #else
        typedef void (*RenderSystemDeleterFuncPtr)(void*);
        #endif

        RenderSystemDeleter() noexcept = default;

        RenderSystemDeleter(const RenderSystemDeleter&) noexcept = default;
        RenderSystemDeleter& operator = (const RenderSystemDeleter&) noexcept = default;

        //! Constructs the deleter with the actual deleter function pointer.
        inline RenderSystemDeleter(RenderSystemDeleterFuncPtr deleterFuncPtr);

        /**
        \brief Deletes the specified render system using the function pointer this deleter was initialized with.
        \remarks If no function pointer was provided, the deleter uses the C++ \c delete operator by default.
        */
        inline void operator()(RenderSystem* ptr) const;

    private:

        RenderSystemDeleterFuncPtr deleterFuncPtr_ = nullptr;

};

/**
\brief Unique pointer type for the RenderSystem interface with a custom deleter.
\see RenderSystem
\see RenderSystem::Load
\see RenderSystem::Unload
\see RenderSystemDeleter
*/
using RenderSystemPtr = std::unique_ptr<RenderSystem, RenderSystemDeleter>;

/**
\brief Render system interface.
\remarks This is the main interface for the entire renderer.
It manages the ownership of all graphics objects and is used to create, modify, and delete all those objects.
The main functions for most graphics objects are \c Create*, \c Write*, \c Read*, \c Map*, \c Unmap*, and \c Release:
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
class LLGL_EXPORT RenderSystem : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::RenderSystem );

    public:

        //! Releases the internal data.
        ~RenderSystem();

        /* ----- Common ----- */

        /**
        \brief Returns the list of all available render system modules for the current platform.
        \remarks For example, on Win32 this might be <code>{ "Direct3D12", "Direct3D11", "OpenGL" }</code>, but on MacOS it might be <code>{ "Metal", "OpenGL" }</code>.
        */
        static std::vector<std::string> FindModules();

        /**
        \brief Loads a new render system from the specified module.
        \param[in] renderSystemDesc Specifies the render system descriptor structure. The 'moduleName' member of this strucutre must not be empty.
        \param[out] report Optional pointer to a report on potential failure of loading the specified module.
        \remarks If loading the specified moduel failed, the return value is null and the reason for failure is reported in \c report if it's a valid pointer.
        \remarks The descriptor structure can be initialized by only the module name like shown in the following example:
        \code
        // Load the "OpenGL" render system module
        LLGL::RenderSystemPtr myRenderSystem = LLGL::RenderSystem::Load("OpenGL");
        \endcode
        \remarks The debugger and profiler can be used like this:
        \code
        // Forward all log reports to the standard output stream for errors
        LLGL::Log::RegisterCallbackStd();

        // Declare profiler and debugger (these classes can also be extended)
        LLGL::RenderingProfiler myProfiler;
        LLGL::RenderingDebugger myDebugger;

        // Load the "Direct3D11" render system module
        LLGL::RenderSystemDescriptor myRendererDesc;
        {
            myRendererDesc.moduleName   = "Direct3D11";
            myRendererDesc.profiler     = &myProfiler;
            myRendererDesc.debugger     = &myDebugger;
        }
        LLGL::RenderSystemPtr myRenderSystem = LLGL::RenderSystem::Load(myRendererDesc);
        \endcode
        \throws std::runtime_error If loading the render system from the specified module failed.
        \see RenderSystemDescriptor::moduleName
        */
        static RenderSystemPtr Load(const RenderSystemDescriptor& renderSystemDesc, Report* report = nullptr);

        /**
        \brief Unloads the specified render system and the internal module.
        \remarks After this call, the specified render system and all the objects associated to it must no longer be used!
        */
        static void Unload(RenderSystemPtr&& renderSystem);

    public:

        /**
        \brief Rendering API identification number.
        \remarks This can be a value of the RendererID entries.
        Since the render system is modular, a new render system can have its own ID number.
        \see RendererID
        */
        int GetRendererID() const;

        //! Returns the name of this render system.
        const char* GetName() const;

        /**
        \brief Returns basic renderer information.
        \remarks The validity of these information is only guaranteed if this function is called
        after a valid swap-chain has been created. Otherwise the behavior is undefined!
        */
        const RendererInfo& GetRendererInfo() const;

        /**
        \brief Returns the rendering capabilities.
        \remarks The validity of these information is only guaranteed if this function is called
        after a valid swap-chain has been created. Otherwise the behavior is undefined!
        */
        const RenderingCapabilities& GetRenderingCaps() const;

        /**
        \brief Returns a pointer to the report or null if there is none.
        \remarks If there is a report, it indicates errors from a previous operation, similar to \c ::GetLastError() from the Windows API.
        \see Report
        */
        const Report* GetReport() const;

    public:

        /* ----- Swap-chain ----- */

        /**
        \brief Creates a new swap-chain. At least one swap-chain is required to render into an output surface.
        \param[in] swapChainDesc Specifies the swap-chain descriptor, which contains resolution, bit depth, multi-sampling settings etc.
        \param[in] surface Optional shared pointer to a surface for the swap-chain.
        If this is null, the swap-chain will create its own platform specific surface, which can be accessed by SwapChain::GetSurface.
        The default surface on desktop platforms (i.e. Window interface) is not shown automatically, i.e. the Window::Show function has to be invoked to show the surface.
        \see SwapChain::GetSurface
        \see Window::Show
        */
        virtual SwapChain* CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface = {}) = 0;

        /**
        \brief Releases the specified swap-chain. After this call, the specified object must no longer be used.
        \see CreateSwapChain
        */
        virtual void Release(SwapChain& swapChain) = 0;

        /* ----- Command queues ----- */

        //! Returns the single instance of the command queue.
        virtual CommandQueue* GetCommandQueue() = 0;

        /* ----- Command buffers ----- */

        /**
        \brief Creates a new command buffer.
        \param[in] commandBufferDesc Specifies an optional command buffer descriptor.
        \remarks Each render system can create multiple command buffers,
        but especially the legacy graphics APIs such as OpenGL and Direct3D 11 don't provide a performance benefit with that feature.
        */
        virtual CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc = {}) = 0;

        /**
        \brief Releases the specified command buffer. After this call, the specified object must no longer be used.
        \see CreateCommandBuffer
        */
        virtual void Release(CommandBuffer& commandBuffer) = 0;

        /* ----- Buffers ------ */

        /**
        \brief Creates a new generic hardware buffer.
        \param[in] bufferDesc Specifies the buffer descriptor.
        \param[in] initialData Optional raw pointer to the data with which the buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteBuffer" function before it is used for drawing operations. By default null.
        \see WriteBuffer
        */
        virtual Buffer* CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData = nullptr) = 0;

        /**
        \brief Creates a new buffer array.
        \param[in] numBuffers Specifies the number of buffers in the array. This must be greater than 0.
        \param[in] bufferArray Pointer to an array of Buffer object pointers. This must not be null.
        \remarks All buffers within this array must have the same binding flags.
        The buffers inside this array must persist as long as this buffer array is used,
        and the individual buffers are still required to read and write its data from and to the GPU.
        \throws std::invalid_argument If \c numBuffers is 0.
        \throws std::invalid_argument If \c bufferArray is null.
        \throws std::invalid_argument If any of the pointers in the array are null.
        \throws std::invalid_argument If not all buffers have the same binding flags.
        \see BufferDescriptor::bindFlags
        */
        virtual BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) = 0;

        //! Releases the specified buffer object. After this call, the specified object must no longer be used.
        virtual void Release(Buffer& buffer) = 0;

        //! Releases the specified buffer array object. After this call, the specified object must no longer be used.
        virtual void Release(BufferArray& bufferArray) = 0;

        /**
        \brief Updates the data of the specified buffer.
        \param[in] buffer Specifies the destination buffer whose data is to be updated.
        \param[in] offset Specifies the offset (in bytes) at which the buffer is to be updated.
        This offset plus the data block size (i.e. <code>offset + dataSize</code>) must be less than or equal to the size of the buffer.
        \param[in] data Raw pointer to the data with which the buffer is to be updated. This must not be null!
        \param[in] dataSize Specifies the size (in bytes) of the data block which is to be updated.
        This must be less then or equal to the size of the buffer.
        \remarks To update a small buffer (maximum of 65536 bytes) during encoding a command buffer, use CommandBuffer::UpdateBuffer.
        \see ReadBuffer
        */
        virtual void WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize) = 0;

        /**
        \brief Reads the data from the specified buffer.
        \param[in] buffer Specifies the buffer which is to be read.
        \param[in] offset Specifies the offset (in bytes) at which the buffer is to be read.
        \param[out] data Raw pointer to a memory block in CPU memory space where the data will be written to.
        \param[in] dataSize Specifies the size (in bytes) of the data block given by the \c data parameter.
        \see WriteBuffer
        */
        virtual void ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize) = 0;

        /**
        \brief Maps the specified buffer from GPU to CPU memory space.
        \param[in] buffer Specifies the buffer which is to be mapped. Depending on the CPU access type (see \c access parameter),
        this buffer must have been created with the corresponding CPU access flag, i.e. CPUAccessFlags::Read and/or CPUAccessFlags::Write.
        \param[in] access Specifies the CPU buffer access requirement, i.e. if the CPU can read and/or write the mapped memory.
        \return Raw pointer to the mapped memory block in CPU memory space or null if the operation failed.
        \remarks Memory that is written back from CPU to GPU becomes visible in the GPU after a corresponding UnmapBuffer operation.
        \see UnmapBuffer
        */
        virtual void* MapBuffer(Buffer& buffer, const CPUAccess access) = 0;

        /**
        \brief Maps the specified buffer range from GPU to CPU memory space.
        \param[in] buffer Specifies the buffer which is to be mapped. Depending on the CPU access type (see \c access parameter),
        this buffer must have been created with the corresponding CPU access flag, i.e. CPUAccessFlags::Read and/or CPUAccessFlags::Write.
        \param[in] access Specifies the CPU buffer access requirement, i.e. if the CPU can read and/or write the mapped memory.
        \param[in] offset Specifies the memory offset (in bytes) from the GPU buffer.
        \param[in] length Specifies the length of the memory block (in bytes) that is to be mapped.
        \return Raw pointer to the mapped memory block in CPU memory space or null if the operation failed.
        \remarks Memory that is written back from CPU to GPU becomes visible in the GPU after a corresponding UnmapBuffer operation.
        \see UnmapBuffer
        */
        virtual void* MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length) = 0;

        /**
        \brief Unmaps the specified buffer.
        \remarks This must be called on a buffer that was previously mapped into CPU memory space.
        The following example illustrates how to map and unmap a buffer from GPU into CPU memory sapce:
        \code
        if (void* data = myRenderer->MapBuffer(*myBuffer, LLGL::CPUAccess::Write))
        {
            // Write to 'data' ...
            myRenderer->UnmapBuffer(*myBuffer);
        }
        \endcode
        \see MapBuffer
        */
        virtual void UnmapBuffer(Buffer& buffer) = 0;

        /* ----- Textures ----- */

        /**
        \brief Creates a new texture.
        \param[in] textureDesc Specifies the texture descriptor.
        \param[in] imageDesc Optional pointer to the image data descriptor.
        If this is null, the texture will be initialized with the currently configured default image color (if this feature is enabled).
        If this is non-null, it is used to initialize the texture data.
        This parameter will be ignored if the texture type is a multi-sampled texture (i.e. TextureType::Texture2DMS or TextureType::Texture2DMSArray).
        \see WriteTexture
        */
        virtual Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) = 0;

        //! Releases the specified texture object. After this call, the specified object must no longer be used.
        virtual void Release(Texture& texture) = 0;

        /**
        \brief Updates the image data of the specified texture.
        \param[in] texture Specifies the texture whose data is to be updated.
        \param[in] textureRegion Specifies the region where the texture is to be updated. The field TextureRegion::numMipLevels \b must be 1.
        \param[in] imageDesc Specifies the image data descriptor. Its \c data member must not be null!
        \remarks This function can only be used for non-multi-sample textures, i.e. from types other than TextureType::Texture2DMS and TextureType::Texture2DMSArray.
        */
        virtual void WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc) = 0;

        /**
        \brief Reads the image data from the specified texture.
        \param[in] texture Specifies the texture object to read from.
        \param[in] textureRegion Specifies the region where the texture data is to be read.
        \param[out] imageDesc Specifies the destination image descriptor to write the texture data to.
        \remarks The required size for a successful texture read operation depends on the image format, data type, and texture size.
        The Texture::GetDesc or Texture::GetMipExtent functions can be used to determine the texture dimensions.
        \code
        // Query texture size attribute
        auto myTextureExtent = myTexture->GetMipExtent(0);

        // Allocate image buffer with elements in all dimensions
        std::vector<std::uint8_t> myImage(myTextureExtent.width * myTextureExtent.height * myTextureExtent.depth * 4);

        // Initialize destination image descriptor
        const DstImageDescriptor myImageDesc {
            LLGL::ImageFormat::RGBA,                // RGBA image format, since the size of 'myImage' is a multiple of 4
            LLGL::DataType::UInt8,                  // 8-bit unsigned integral data type: <std::uint8_t> or <unsigned char>
            myImage.data(),                         // Output image buffer
            myImage.size() * sizeof(std::uint8_t)   // Image buffer size: number of color elements and size of each color element
        };

        // Read texture data from first MIP-map level (index 0)
        myRenderSystem->ReadTexture(*myTexture, 0, myImageDesc);
        \endcode
        \note The behavior is undefined if <code>imageDesc.data</code> points to an invalid buffer,
        or <code>imageDesc.data</code> points to a buffer that is smaller than specified by <code>imageDesc.dataSize</code>,
        or <code>imageDesc.dataSize</code> is less than the required size.
        \throws std::invalid_argument If <code>imageDesc.data</code> is null.
        \see Texture::GetDesc
        \see Texture::GetMipExtent
        */
        virtual void ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc) = 0;

        /* ----- Samplers ---- */

        /**
        \brief Creates a new Sampler object.
        \throws std::runtime_error If the renderer does not support Sampler objects (e.g. if OpenGL 3.1 or lower is used).
        \see GetRenderingCaps
        */
        virtual Sampler* CreateSampler(const SamplerDescriptor& samplerDesc) = 0;

        //! Releases the specified Sampler object. After this call, the specified object must no longer be used.
        virtual void Release(Sampler& sampler) = 0;

        /* ----- Resource Heaps ----- */

        /**
        \brief Creates a new resource heap.
        \param[in] resourceHeapDesc Specifies the descriptor for the resource heap.
        If the \c numResourceViews field is zero, the \c initialResourceViews parameter will determine the number of resources,
        it must \e not be empty and it \b must be a multiple of the number of bindings in the piepline layout.
        \param[in] initialResourceViews Specifies an optional array of initial resource views.
        If this is non-null, the array pointed to must have enough elements to initialze the entire resource heap.
        Uninitialized resource views must be written with a call to WriteResourceHeap before the resource heap can be used in a command buffer.
        \remarks Resource heaps are used in combination with a pipeline layout.
        The pipeline layout determines to which binding points the resources are bound.
        \see CreatePipelineLayout
        \see CommandBuffer::SetResourceHeap
        \see WriteResourceHeap
        \see ResourceHeapDescriptor::numResourceViews
        */
        virtual ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews = {}) = 0;

        //! Releases the specified ResourceHeap object. After this call, the specified object must no longer be used.
        virtual void Release(ResourceHeap& resourceHeap) = 0;

        /**
        \brief Writes new resource view descriptors into the specified resource heap.
        \param[in] resourceHeap Specifies the resource heap that is to be updated.
        \param[in] firstDescriptor Zero-based index to the first descriptor that is to be updated.
        This must be less than the number of bindings in the resource heap's pipeline layout (PipelineLayout::GetNumHeapBindings)
        multiplied by the number of descriptor sets in the resource heap (ResourceHeap::GetNumDescriptorSets).
        \param[in] resourceViews Array of resource view descriptors.
        \remarks The type of a resource view, i.e. whether it's a buffer, texture, or sampler, must not be changed with this function.
        \return Number of resource views that have been updated by this call. Any resource view descriptor with a \c resource field that is null will be ignored silently.
        \see ResourceHeap::GetNumDescriptorSets
        \see PipelineLayout::GetNumHeapBindings
        */
        virtual std::uint32_t WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews) = 0;

        /* ----- Render Passes ----- */

        /**
        \brief Creates a new RenderPass object.
        \return Pointer to the new RenderPass object or null if the render system does not use render passes.
        In the case of the latter, null pointers are allowed for render passes.
        \see RenderTargetDescriptor::renderPass
        \see GraphicsPipelineDescriptor::renderPass
        \see CommandBuffer::BeginRenderPass
        \see CommandBuffer::EndRenderPass
        */
        virtual RenderPass* CreateRenderPass(const RenderPassDescriptor& renderPassDesc) = 0;

        //! Releases the specified RenderPass object. After this call, the specified object must no longer be used.
        virtual void Release(RenderPass& renderPass) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Creates a new RenderTarget object.
        \throws std::runtime_error If the renderer does not support RenderTarget objects (e.g. if OpenGL 2.1 or lower is used).
        */
        virtual RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc) = 0;

        //! Releases the specified RenderTarget object. After this call, the specified object must no longer be used.
        virtual void Release(RenderTarget& renderTarget) = 0;

        /* ----- Shader ----- */

        /**
        \brief Creates a new and Shader object and compiles the specified source.
        \remarks To check whether the compilation was successful or not, use the \c HasErrors and \c GetReport functions of the Shader interface.
        \see Shader::HasErrors
        \see Shader::GetReport
        \see ShaderDescriptor
        \see ShaderDescFromFile
        */
        virtual Shader* CreateShader(const ShaderDescriptor& shaderDesc) = 0;

        //! Releases the specified Shader object. After this call, the specified object must no longer be used.
        virtual void Release(Shader& shader) = 0;

        /* ----- Pipeline Layouts ----- */

        /**
        \brief Creates a new and initialized pipeline layout object, if and only if the renderer supports pipeline layouts.
        \param[in] pipelineLayoutDesc Specifies the pipeline layout descriptor with all layout bindings.
        \remarks A pipeline layout is required in combination with a ResourceHeap to bind multiple resources at once.
        For modern graphics APIs (i.e. Direct3D 12 and Vulkan), this is only way to bind shader resources.
        For legacy graphics APIs (i.e. Direct3D 11 and OpenGL), shader resources can also be bound individually with the extended command buffer.
        \return Pointer to the new PipelineLayout object or null if the renderer does not support pipeline layouts.
        \see CreateResourceHeap
        \see Parse
        */
        virtual PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc) = 0;

        //! Releases the specified PipelineLayout object. After this call, the specified object must no longer be used.
        virtual void Release(PipelineLayout& pipelineLayout) = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Creates a new graphics or compute pipeline state object (PSO) from the specified cache.
        \param[in] serializedCache Specifies the serialized cache that was created from the same pipeline state.
        This is usually created during a previous application run, stored to file, and restored on a later run.
        \remarks Here is an example how to use pipeline state caches:
        \code
        // Try to read PSO cache file
        if (auto myCache = LLGL::Blob::CreateFromFile("MyPSOCacheFile.bin"))
        {
            // Create PSO from cache
            myPipelineState = myRenderer->CreatePipelineState(*myCache);
        }
        else
        {
            // Setup initial pipeline state
            LLGL::ComputePipelineDescritpor myPipelineDesc;
            myPipelineDesc.pipelineLayout = myPipelineLayout;
            myPipelineDesc.computeShader  = myComputeShader;

            // Create new PSO
            LLGL::Blob myCache;
            myRenderer->CreatePipelineState(myPipelineDesc, &myCache);

            // Store PSO to file
            std::ofstream myCacheFile{ "MyPSOCacheFile.bin", std::ios::out | std::ios::binary };
            myCacheFile.write(
                reinterpret_cast<const char*>(myCache.GetData()),
                static_cast<std::streamsize>(myCache.GetSize())
            );
        }
        \endcode
        \see CreatePipelineState(const GraphicsPipelineDescriptor&, Blob*)
        \see CreatePipelineState(const ComputePipelineDescriptor&, Blob*)
        */
        virtual PipelineState* CreatePipelineState(const Blob& serializedCache) = 0;

        /**
        \brief Creates a new graphics pipeline state object (PSO).
        \param[in] pipelineStateDesc Specifies the graphics PSO descriptor.
        This will describe the entire pipeline state, i.e. the blending-, rasterizer-, depth-, stencil- and shader states.
        The \c vertexShader member of the descriptor must never be null!
        \param[out] serializedCache Optional pointer to a Blob instance. If this is not null, the renderer returns the pipeline state as serialized cache.
        This cache may be unique to the respective hardware and driver the application is running on. The behavior is undefined if this cache is used in a different software environment.
        It can be used to faster restore a pipeline state on next application run.
        \see GraphicsPipelineDescriptor
        \see CreatePipelineState(const Blob&)
        */
        virtual PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, Blob* serializedCache = nullptr) = 0;

        /**
        \brief Creates a new compute pipeline state object (PSO).
        \param[in] pipelineStateDesc Specifies the compute PSO descriptor. This will describe the entire pipeline state.
        The \c computeShader member of the descriptor must never be null!
        \param[out] serializedCache Optional pointer to a Blob instance. If this is not null, the renderer returns the pipeline state as serialized cache.
        This cache may be unique to the respective hardware and driver the application is running on. The behavior is undefined if this cache is used in a different software environment.
        It can be used to faster restore a pipeline state on next application run.
        \see ComputePipelineDescriptor
        \see CreatePipelineState(const Blob&)
        */
        virtual PipelineState* CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, Blob* serializedCache = nullptr) = 0;

        //! Releases the specified PipelineState object. After this call, the specified object must no longer be used.
        virtual void Release(PipelineState& pipelineState) = 0;

        /* ----- Queries ----- */

        //! Creates a new query heap.
        virtual QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc) = 0;

        //! Releases the specified QueryHeap object. After this call, the specified object must no longer be used.
        virtual void Release(QueryHeap& queryHeap) = 0;

        /* ----- Fences ----- */

        /**
        \brief Creates a new fence (used for CPU/GPU synchronization).
        \see CommandBuffer::SubmitFence
        \see CommandBuffer::WaitFence
        */
        virtual Fence* CreateFence() = 0;

        //! Releases the specified Fence object. After this call, the specified object must no longer be used.
        virtual void Release(Fence& fence) = 0;

        /* ----- Extensions ----- */

        /**
        \brief Returns the native device handle.

        \param[out] nativeHandle Raw pointer to the backend specific structure to store the native handle.
        Optain the respective structure from <code>#include <LLGL/Backend/BACKEND/NativeHandle.h></code>
        where \c BACKEND must be either \c Direct3D12, \c Direct3D11, \c Metal, or \c Vulkan.
        OpenGL does not have a native handle as it uses the current platform specific GL context.

        \param[in] nativeHandleSize Specifies the size (in bytes) of the native handle structure for robustness.
        This must be <code>sizeof(STRUCT)</code> where \c STRUCT is the respective backend specific structure such as \c LLGL::Direct3D12::RenderSystemNativeHandle.

        \return True if the native handle was successfully retrieved. Otherwise, \c nativeHandleSize specifies an incompatible structure size.

        \remarks For the Direct3D backends, all retrieved COM pointers will be incremented and the user is responsible for releasing those pointers,
        i.e. a call to \c IUnknown::Release is required to each of the objects returned by this function.
        \remarks For the Metal backend, all retrieved \c NSObject instances will have their retain counter incremented and the user is responsible for releasing those objects,
        i.e. a call to <code>-(oneway void)release</code> is required to each of the objects returned by this function.
        \remarks For backends that do not support this function, the return value is false unless \c nativeHandle is null or \c nativeHandleSize is 0.
        \remarks Example for obtaining the native handle of a Direct3D12 render system:
        \code
        #include <LLGL/Backend/Direct3D12/NativeHandle.h>
        //...
        LLGL::Direct3D12::RenderSystemNativeHandle d3dNativeHandle;
        myRenderer->GetNativeHandle(&d3dNativeHandle, sizeof(d3dNativeHandle));
        ID3D12Device* d3dDevice = d3dNativeHandle.device;
        ...
        d3dDevice->Release();
        \endcode

        \note Only supported with: Direct3D 12, Direct3D 11, Vulkan, Metal.

        \see Direct3D12::RenderSystemNativeHandle
        \see Direct3D11::RenderSystemNativeHandle
        \see Vulkan::RenderSystemNativeHandle
        \see Metal::RenderSystemNativeHandle
        */
        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) = 0;

    protected:

        //! Allocates the internal data.
        RenderSystem();

        //! Returns the internal report of this render system to be modified by the renderer implementation.
        Report& GetMutableReport();

        /**
        \brief Prints a formatted string and \e replaces the current render system report with it as error.
        \remarks This can be used to log non-fatal errors so the client programmer can query the last error message.
        \see Report::Errorf
        */
        void Errorf(const char* format, ...);

        //! Sets the renderer information.
        void SetRendererInfo(const RendererInfo& info);

        //! Sets the rendering capabilities.
        void SetRenderingCaps(const RenderingCapabilities& caps);

    protected:

        //! Validates the specified buffer descriptor to be used for buffer creation.
        static void AssertCreateBuffer(const BufferDescriptor& bufferDesc, std::uint64_t maxSize);

        //! Validates the specified arguments to be used for buffer array creation.
        static void AssertCreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        //! Validates the specified shader descriptor.
        static void AssertCreateShader(const ShaderDescriptor& shaderDesc);

        //! Validates the specified image data size against the required size (in bytes).
        static void AssertImageDataSize(std::size_t dataSize, std::size_t requiredDataSize, const char* useCase = nullptr);

        /**
        \brief Copies the specified source image to the destination image.
        \remarks This function also performs image conversion if there is a mismatch between source and destination format.
        \returns The number of bytes that have been written into the destination image buffer.
        \see ConvertImageBuffer
        */
        static std::size_t CopyTextureImageData(
            const DstImageDescriptor&   dstImageDesc,
            const SrcImageDescriptor&   srcImageDesc,
            std::uint32_t               numTexels,
            std::uint32_t               numTexelsInRow,
            std::uint32_t               rowStride       = 0
        );

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


/*
 * RenderSystemDeleter implementation
 */

inline RenderSystemDeleter::RenderSystemDeleter(RenderSystemDeleterFuncPtr deleterFuncPtr) :
    deleterFuncPtr_ { deleterFuncPtr }
{
}

inline void RenderSystemDeleter::operator()(RenderSystem* ptr) const
{
    if (ptr != nullptr)
    {
        if (deleterFuncPtr_ != nullptr)
            deleterFuncPtr_(ptr);
        else
            delete ptr;
    }
}


} // /namespace LLGL


#endif



// ================================================================================
