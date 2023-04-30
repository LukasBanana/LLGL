/*
 * RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Platform/Module.h"
#include "../Core/CoreUtils.h"
#include "../Core/StringUtils.h"
#include "../Core/Assertion.h"
#include "../Core/Exception.h"
#include "RenderTargetUtils.h"
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Format.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/StaticLimits.h>
#include <LLGL/Log.h>
#include "BuildID.h"

#include <LLGL/RenderSystem.h>
#include <inttypes.h>
#include <string>
#include <unordered_map>

#ifdef LLGL_ENABLE_DEBUG_LAYER
#   include "DebugLayer/DbgRenderSystem.h"
#endif

#include <LLGL/Platform/Platform.h>
#ifdef LLGL_OS_ANDROID
#   include "../Platform/Android/AndroidApp.h"
#endif

#include "ModuleInterface.h"


namespace LLGL
{


/* ----- Render system ----- */

struct RenderSystem::Pimpl
{
    int                     rendererID = 0;
    std::string             name;
    RendererInfo            info;
    RenderingCapabilities   caps;
    Report                  report;
};

static std::unordered_map<RenderSystem*, std::unique_ptr<Module>> g_renderSystemModules;

RenderSystem::RenderSystem() :
    pimpl_ { new Pimpl{} }
{
}

RenderSystem::~RenderSystem()
{
    delete pimpl_;
}

#ifdef LLGL_BUILD_STATIC_LIB

std::vector<std::string> RenderSystem::FindModules()
{
    return StaticModule::GetStaticModules();
}

#else // LLGL_BUILD_STATIC_LIB

std::vector<std::string> RenderSystem::FindModules()
{
    /* Iterate over all known modules and return those that are available on the current platform */
    constexpr const char* knownModules[] =
    {
        "Null",

        #if defined(LLGL_OS_IOS) || defined(LLGL_OS_ANDROID)
        "OpenGLES3",
        #else
        "OpenGL",
        #endif

        #if defined(LLGL_OS_MACOS) || defined(LLGL_OS_IOS)
        "Metal",
        #else
        "Vulkan",
        #endif

        #ifdef LLGL_OS_WIN32
        "Direct3D11",
        "Direct3D12",
        #endif
    };

    std::vector<std::string> modules;

    for (auto m : knownModules)
    {
        auto moduleName = Module::GetModuleFilename(m);
        if (Module::IsAvailable(moduleName.c_str()))
            modules.push_back(m);
    }

    return modules;
}

static bool LoadRenderSystemBuildID(
    Module&             module,
    const std::string&  moduleFilename,
    Report*             report)
{
    /* Load "LLGL_RenderSystem_BuildID" procedure */
    LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_BUILDID, (void));

    auto RenderSystem_BuildID = reinterpret_cast<PFN_RENDERSYSTEM_BUILDID>(module.LoadProcedure("LLGL_RenderSystem_BuildID"));
    if (!RenderSystem_BuildID)
    {
        ReportException(report, "failed to load <LLGL_RenderSystem_BuildID> procedure from module: %s", moduleFilename.c_str());
        return false;
    }

    return (RenderSystem_BuildID() == LLGL_BUILD_ID);
}

static int LoadRenderSystemRendererID(Module& module, const RenderSystemDescriptor& renderSystemDesc)
{
    /* Load "LLGL_RenderSystem_RendererID" procedure */
    LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_RENDERERID, (const void*));

    auto RenderSystem_RendererID = reinterpret_cast<PFN_RENDERSYSTEM_RENDERERID>(module.LoadProcedure("LLGL_RenderSystem_RendererID"));
    if (RenderSystem_RendererID)
        return RenderSystem_RendererID(&renderSystemDesc);

    return RendererID::Undefined;
}

static const char* LoadRenderSystemName(Module& module, const RenderSystemDescriptor& renderSystemDesc)
{
    /* Load "LLGL_RenderSystem_Name" procedure */
    LLGL_PROC_INTERFACE(const char*, PFN_RENDERSYSTEM_NAME, (const void*));

    if (auto RenderSystem_Name = reinterpret_cast<PFN_RENDERSYSTEM_NAME>(module.LoadProcedure("LLGL_RenderSystem_Name")))
        return RenderSystem_Name(&renderSystemDesc);
    else
        return "";
}

static RenderSystem* LoadRenderSystem(
    Module&                         module,
    const char*                     moduleFilename,
    const RenderSystemDescriptor&   renderSystemDesc,
    Report*                         outReport)
{
    /* Load "LLGL_RenderSystem_Alloc" procedure */
    LLGL_PROC_INTERFACE(void*, PFN_RENDERSYSTEM_ALLOC, (const void*, int));

    auto RenderSystem_Alloc = reinterpret_cast<PFN_RENDERSYSTEM_ALLOC>(module.LoadProcedure("LLGL_RenderSystem_Alloc"));
    if (!RenderSystem_Alloc)
        return ReportException(outReport, "failed to load 'LLGL_RenderSystem_Alloc' procedure from module: %s", moduleFilename);

    /* Allocate render system */
    auto renderSystem = reinterpret_cast<RenderSystem*>(RenderSystem_Alloc(&renderSystemDesc, static_cast<int>(sizeof(RenderSystemDescriptor))));
    if (!renderSystem)
        return ReportException(outReport, "failed to allocate render system from module: %s", moduleFilename);

    /* Check if errors where reported and the render system is unusable */
    if (auto report = renderSystem->GetReport())
    {
        if (outReport != nullptr)
            *outReport = *report;
        if (report->HasErrors())
            return nullptr;
    }

    return renderSystem;
}

static RenderSystemDeleter::RenderSystemDeleterFuncPtr LoadRenderSystemDeleter(Module& module)
{
    /* Load "LLGL_RenderSystem_Free" procedure */
    return reinterpret_cast<RenderSystemDeleter::RenderSystemDeleterFuncPtr>(module.LoadProcedure("LLGL_RenderSystem_Free"));
}

#endif // /LLGL_BUILD_STATIC_LIB

RenderSystemPtr RenderSystem::Load(const RenderSystemDescriptor& renderSystemDesc, Report* report)
{
    /* Initialize mobile specific states */
    #if defined LLGL_OS_ANDROID

    AndroidApp::Get().Initialize(renderSystemDesc.androidApp);

    #endif

    #ifdef LLGL_BUILD_STATIC_LIB

    /* Allocate render system */
    auto renderSystem = RenderSystemPtr
    {
        reinterpret_cast<RenderSystem*>(StaticModule::AllocRenderSystem(renderSystemDesc))
    };

    if (renderSystemDesc.profiler != nullptr || renderSystemDesc.debugger != nullptr)
    {
        #ifdef LLGL_ENABLE_DEBUG_LAYER

        /* Create debug layer render system */
        renderSystem = RenderSystemPtr{ new DbgRenderSystem{ std::move(renderSystem), renderSystemDesc.profiler, renderSystemDesc.debugger } };

        #else

        if (report != nullptr)
            report->Errorf("LLGL was not compiled with debug layer support");

        #endif // /LLGL_ENABLE_DEBUG_LAYER
    }

    renderSystem->pimpl_->name          = StaticModule::GetRendererName(renderSystemDesc.moduleName);
    renderSystem->pimpl_->rendererID    = StaticModule::GetRendererID(renderSystemDesc.moduleName);

    /* Return new render system and unique pointer */
    return renderSystem;

    #else // LLGL_BUILD_STATIC_LIB

    /* Load render system module */
    auto moduleFilename = Module::GetModuleFilename(renderSystemDesc.moduleName.c_str());
    auto module         = Module::Load(moduleFilename.c_str());

    /*
    Verify build ID from render system module to detect a module,
    that has compiled with a different compiler (type, version, debug/release mode etc.)
    */
    if (!LoadRenderSystemBuildID(*module, moduleFilename, report))
        return ReportException(report, "build ID mismatch in render system module");

    #ifdef LLGL_ENABLE_EXCEPTIONS
    try
    #endif
    {
        /* Allocate render system */
        auto renderSystem = RenderSystemPtr
        {
            LoadRenderSystem(*module, moduleFilename.c_str(), renderSystemDesc, report),
            RenderSystemDeleter{ LoadRenderSystemDeleter(*module) }
        };

        if (renderSystem)
        {
            if (renderSystemDesc.profiler != nullptr || renderSystemDesc.debugger != nullptr)
            {
                #ifdef LLGL_ENABLE_DEBUG_LAYER

                /* Create debug layer render system */
                renderSystem = RenderSystemPtr{ new DbgRenderSystem{ std::move(renderSystem), renderSystemDesc.profiler, renderSystemDesc.debugger } };

                #else

                if (report != nullptr)
                    report->Errorf("LLGL was not compiled with debug layer support");

                #endif // /LLGL_ENABLE_DEBUG_LAYER
            }

            renderSystem->pimpl_->name          = LoadRenderSystemName(*module,renderSystemDesc);
            renderSystem->pimpl_->rendererID    = LoadRenderSystemRendererID(*module,renderSystemDesc);

            /* Store new module inside internal map */
            g_renderSystemModules[renderSystem.get()] = std::move(module);
        }

        return renderSystem;
    }
    #ifdef LLGL_ENABLE_EXCEPTIONS
    catch (const std::exception& e)
    {
        /* Throw with new exception, otherwise the exception's v-table will be corrupted since it's part of the module */
        if (report != nullptr)
            report->Errorf("%s", e.what());
        return nullptr;
    }
    #endif

    #endif // /LLGL_BUILD_STATIC_LIB
}

void RenderSystem::Unload(RenderSystemPtr&& renderSystem)
{
    auto it = g_renderSystemModules.find(renderSystem.get());
    if (it != g_renderSystemModules.end())
    {
        /* Delete render system first, then release module */
        renderSystem.reset();
        g_renderSystemModules.erase(it);
    }
}

int RenderSystem::GetRendererID() const
{
    return pimpl_->rendererID;
}

const char* RenderSystem::GetName() const
{
    return pimpl_->name.c_str();
}

const RendererInfo& RenderSystem::GetRendererInfo() const
{
    return pimpl_->info;
}

const RenderingCapabilities& RenderSystem::GetRenderingCaps() const
{
    return pimpl_->caps;
}

const Report* RenderSystem::GetReport() const
{
    return (pimpl_->report ? &(pimpl_->report) : nullptr);
}


/*
 * ======= Protected: =======
 */

Report& RenderSystem::GetMutableReport()
{
    return pimpl_->report;
}

void RenderSystem::SetRendererInfo(const RendererInfo& info)
{
    pimpl_->info = info;
}

void RenderSystem::SetRenderingCaps(const RenderingCapabilities& caps)
{
    pimpl_->caps = caps;
}

void RenderSystem::AssertCreateBuffer(const BufferDescriptor& bufferDesc, std::uint64_t maxSize)
{
    LLGL_ASSERT(
        (bufferDesc.size <= maxSize),
        "buffer descriptor with size of 0x%016" PRIX64 " exceeded limit of 0x%016" PRIX64, bufferDesc.size, maxSize
    );

    /* Validate binding flags */
    const long validBindFlags =
    (
        BindFlags::VertexBuffer         |
        BindFlags::IndexBuffer          |
        BindFlags::ConstantBuffer       |
        BindFlags::Sampled              |
        BindFlags::Storage              |
        BindFlags::StreamOutputBuffer   |
        BindFlags::IndirectBuffer       |
        BindFlags::CopySrc              |
        BindFlags::CopyDst
    );

    LLGL_ASSERT(
        ((bufferDesc.bindFlags & (~validBindFlags)) == 0),
        "buffer descriptor with invalid binding flags 0x%08X", bufferDesc.bindFlags
    );
}

static void AssertCreateResourceArrayCommon(std::uint32_t numResources, void* const * resourceArray, const char* resourceName)
{
    /* Validate number of buffers */
    LLGL_ASSERT(!(numResources == 0), "cannot create %s array with zero elements", resourceName);

    /* Validate array pointer */
    LLGL_ASSERT(!(resourceArray == nullptr), "cannot create %s array with null pointer for array", resourceName);

    /* Validate pointers in array */
    for_range(i, numResources)
        LLGL_ASSERT(!(resourceArray[i] == nullptr), "cannot create %s array with null pointer for array element [%u]", resourceName, i);
}

void RenderSystem::AssertCreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    /* Validate common resource array parameters */
    AssertCreateResourceArrayCommon(numBuffers, reinterpret_cast<void* const*>(bufferArray), "buffer");
}

void RenderSystem::AssertCreateShader(const ShaderDescriptor& shaderDesc)
{
    LLGL_ASSERT(
        !(shaderDesc.source == nullptr),
        "cannot create shader with <source> being a null pointer"
    );
    LLGL_ASSERT(
        !(shaderDesc.sourceType == ShaderSourceType::BinaryBuffer && shaderDesc.sourceSize == 0),
        "cannot create shader from binary buffer with <sourceSize> being zero"
    );
}

// Returns the number of color attachments in the specified render target descriptor
static std::size_t CountColorAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::size_t n = 0;
    for (const auto& attachment : renderTargetDesc.attachments)
    {
        if (IsColorFormat(GetAttachmentFormat(attachment)))
            ++n;
    }
    return n;
}

// Returns the number of depth-stencil attachments in the specified render target descriptor
static std::size_t CountDepthStencilAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::size_t n = 0;
    for (const auto& attachment : renderTargetDesc.attachments)
    {
        if (IsDepthOrStencilFormat(GetAttachmentFormat(attachment)))
            ++n;
    }
    return n;
}

void RenderSystem::AssertCreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    if (renderTargetDesc.attachments.size() > LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        /* Check if there is one depth-stencil attachment */
        const auto numColorAttachments = CountColorAttachments(renderTargetDesc);
        LLGL_ASSERT(
            !(numColorAttachments > LLGL_MAX_NUM_COLOR_ATTACHMENTS),
            "render target descriptor with %zu color attachments exceeded limit of %u", numColorAttachments, LLGL_MAX_NUM_COLOR_ATTACHMENTS
        );
    }
    else if (renderTargetDesc.attachments.size() > 1)
    {
        /* Check there are not more than one depth-stencil attachment */
        const auto numDepthStencilAttachments = CountDepthStencilAttachments(renderTargetDesc);
        LLGL_ASSERT(
            !(numDepthStencilAttachments > 1),
            "render target descriptor with %zu depth-stencil attachments exceeded limit of 1", numDepthStencilAttachments
        );
    }
}

void RenderSystem::AssertImageDataSize(std::size_t dataSize, std::size_t requiredDataSize, const char* useCase)
{
    LLGL_ASSERT(
        !(dataSize < requiredDataSize),
        "image data size is too small%s%s; %zu byte(s) are required, but only %zu is specified",
        (useCase != nullptr && *useCase != '\0' ? " for" : ""),
        (useCase != nullptr && *useCase != '\0' ? useCase : ""),
        requiredDataSize,
        dataSize
    );
}

static void CopyRowAlignedData(void* dstData, const void* srcData, std::size_t dstSize, std::size_t dstStride, std::size_t srcStride)
{
    auto dst = reinterpret_cast<std::int8_t*>(dstData);
    auto src = reinterpret_cast<const std::int8_t*>(srcData);

    for (auto dstEnd = dst + dstSize; dst < dstEnd; dst += dstStride, src += srcStride)
        ::memcpy(dst, src, dstStride);
}

void RenderSystem::CopyTextureImageData(
    const DstImageDescriptor&   dstImageDesc,
    const Extent3D&             extent,
    const Format                format,
    const void*                 data,
    std::size_t                 rowStride)
{
    /* Check if image buffer must be converted */
    const auto  numTexels       = (extent.width * extent.height * extent.depth);
    const auto& srcTexFormat    = GetFormatAttribs(format);
    auto        srcFormatSize   = DataTypeSize(srcTexFormat.dataType) * ImageFormatSize(srcTexFormat.format);
    auto        srcImageSize    = (numTexels * srcFormatSize);
    const auto  dstPitch        = (extent.width * srcFormatSize);

    if (srcTexFormat.format != dstImageDesc.format || srcTexFormat.dataType != dstImageDesc.dataType)
    {
        /* Check if padding must be removed */
        ByteBuffer unpaddedData;
        if (rowStride != 0 && dstPitch != rowStride)
        {
            unpaddedData = AllocateByteBuffer(srcImageSize, UninitializeTag{});
            CopyRowAlignedData(unpaddedData.get(), data, srcImageSize, dstPitch, rowStride);
            data = unpaddedData.get();
        }

        /* Determine destination image size */
        auto dstFormatSize  = DataTypeSize(dstImageDesc.dataType) * ImageFormatSize(dstImageDesc.format);
        auto dstImageSize   = (numTexels * dstFormatSize);

        /* Validate input size */
        AssertImageDataSize(dstImageDesc.dataSize, dstImageSize);

        /* Convert mapped data into requested format */
        auto tempData = ConvertImageBuffer(
            SrcImageDescriptor
            {
                srcTexFormat.format,
                srcTexFormat.dataType,
                data,
                srcImageSize
            },
            dstImageDesc.format,
            dstImageDesc.dataType,
            Constants::maxThreadCount
        );

        /* Copy temporary data into output buffer */
        ::memcpy(dstImageDesc.data, tempData.get(), dstImageSize);
    }
    else
    {
        /* Validate input size */
        AssertImageDataSize(dstImageDesc.dataSize, srcImageSize);

        /* Copy mapped data directly into the output buffer */
        if (rowStride != 0 && dstPitch != rowStride)
            CopyRowAlignedData(dstImageDesc.data, data, srcImageSize, dstPitch, rowStride);
        else
            ::memcpy(dstImageDesc.data, data, srcImageSize);
    }
}


} // /namespace LLGL



// ================================================================================
