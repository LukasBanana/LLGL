/*
 * TestNativeHandle.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#define TESTBED_DISABLE_USING_NAMESPACE_LLGL 1
#include "Testbed.h"
#undef TESTBED_DISABLE_USING_NAMESPACE_LLGL

#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/TypeNames.h>

#if LLGL_BUILD_STATIC_LIB

// Test D3D11 and D3D12 on Windows only
#if defined(LLGL_OS_WIN32)
#   include <LLGL/Backend/Direct3D11/NativeHandle.h>
#   include <LLGL/Backend/Direct3D12/NativeHandle.h>
#   define LLGL_TEST_NATIVEHANDLE_D3D 1
#else
#   define LLGL_TEST_NATIVEHANDLE_D3D 0
#endif

// Test Metal on macOS and iOS only
#if defined(LLGL_OS_MACOS) || defined(LLGL_OS_IOS)
#   include <LLGL/Backend/Metal/NativeHandle.h>
#   define LLGL_TEST_NATIVEHANDLE_MT 1
#else
#   define LLGL_TEST_NATIVEHANDLE_MT 0
#endif

// Test Vulkan on Windows, Linux, and Android only
#if LLGL_TESTBED_INCLUDE_VULKAN && (defined(LLGL_OS_WIN32) || defined(LLGL_OS_LINUX) || defined(LLGL_OS_ANDROID))
#   include <LLGL/Backend/Vulkan/NativeHandle.h>
#   define LLGL_TEST_NATIVEHANDLE_VK 1
#else
#   define LLGL_TEST_NATIVEHANDLE_VK 0
#endif

// All platforms support OpenGL
#if LLGL_TESTBED_INCLUDE_OPENGL
#   include <LLGL/Backend/OpenGL/NativeHandle.h>
#   define LLGL_TEST_NATIVEHANDLE_GL 1
#else
#   define LLGL_TEST_NATIVEHANDLE_GL 0
#endif


/*
Print information and match descriptors of native resource handles.
Not all attributes of the native objects are predictable, but their dimensions,
e.g. D3D11_RESOURCE_DIMENSION_BUFFER is expected for a buffer resource with Direct3D 11.
*/
DEF_TEST( NativeHandle )
{
    using namespace LLGL;

    TestResult result = TestResult::Passed;

    // Create buffer resources
    BufferDescriptor buf1Desc;
    {
        buf1Desc.debugName  = "buf1{size=4096,dst}";
        buf1Desc.size       = 4096;
        buf1Desc.bindFlags  = BindFlags::CopyDst;
    }
    CREATE_BUFFER(buf1, buf1Desc, buf1Desc.debugName, nullptr);

    BufferDescriptor buf2Desc;
    {
        buf2Desc.debugName  = "buf2{size=600,rw}";
        buf2Desc.size       = 600;
        buf2Desc.bindFlags  = BindFlags::Storage | BindFlags::Sampled;
        buf2Desc.stride     = 60;
    }
    CREATE_BUFFER(buf2, buf2Desc, buf2Desc.debugName, nullptr);

    BufferDescriptor buf3Desc;
    {
        buf3Desc.debugName  = "buf3{size=512,cbuffer}";
        buf3Desc.size       = 512;
        buf3Desc.bindFlags  = BindFlags::ConstantBuffer;
    }
    CREATE_BUFFER(buf3, buf3Desc, buf3Desc.debugName, nullptr);

    // Create texture resources
    TextureDescriptor tex1Desc;
    {
        tex1Desc.debugName      = "tex1{1D}";
        tex1Desc.type           = TextureType::Texture1D;
        tex1Desc.bindFlags      = BindFlags::Sampled;
        tex1Desc.format         = Format::RGBA8UNorm;
        tex1Desc.extent         = { 8, 1, 1 };
        tex1Desc.mipLevels      = 2;
    }
    CREATE_TEXTURE(tex1, tex1Desc, tex1Desc.debugName, nullptr);

    TextureDescriptor tex2Desc;
    {
        tex2Desc.debugName      = "tex2{2D[8]}";
        tex2Desc.type           = TextureType::Texture2DArray;
        tex2Desc.bindFlags      = BindFlags::Sampled | BindFlags::ColorAttachment;
        tex2Desc.format         = Format::RG16Float;
        tex2Desc.extent         = { 1024, 128, 1 };
        tex2Desc.mipLevels      = 5;
        tex2Desc.arrayLayers    = 8;
    }
    CREATE_TEXTURE(tex2, tex2Desc, tex2Desc.debugName, nullptr);

    TextureDescriptor tex3Desc;
    {
        tex3Desc.debugName      = "tex3{3D,rw}";
        tex3Desc.type           = TextureType::Texture3D;
        tex3Desc.bindFlags      = BindFlags::Sampled | BindFlags::Storage;
        tex3Desc.format         = Format::RGBA8UNorm;
        tex3Desc.extent         = { 4, 4, 4 };
        tex3Desc.mipLevels      = 1;
    }
    CREATE_TEXTURE(tex3, tex3Desc, tex3Desc.debugName, nullptr);

    // Create sampler resources
    SamplerDescriptor smpl1Desc = {};
    smpl1Desc.debugName = "smpl1{default}";
    Sampler* smpl1 = renderer->CreateSampler(smpl1Desc);

    SamplerDescriptor smpl2Desc = Parse("filter.mag=nearest,anisotropy=4");
    smpl2Desc.debugName = "smpl2{aniso4}";
    Sampler* smpl2 = renderer->CreateSampler(smpl2Desc);

    // Test resource with native handles
    const int rendererID = renderer->GetRendererID();

    #define GET_NATIVE_HANDLE(BACKEND, RES)                                             \
        BACKEND::ResourceNativeHandle RES ## _handle;                                   \
        if (!(RES)->GetNativeHandle(&(RES ## _handle), sizeof(RES ## _handle)))         \
        {                                                                               \
            Log::Errorf("LLGL::Resource::GetNativeHandle() failed for \"%s\"\n", #RES); \
            result = TestResult::FailedMismatch;                                        \
        }                                                                               \
        else

    #if LLGL_TEST_NATIVEHANDLE_D3D

    if (rendererID == RendererID::Direct3D11)
    {
        auto FailedToQueryInterface = [&result](const char* interfaceName, const char* objName, HRESULT hr) -> void
        {
            Log::Errorf(
                "LLGL::Resource::GetNativeHandle() did not provide the COM interface '%s' for \"%s\" (Error=0x%08X)\n",
                interfaceName, objName, static_cast<int>(hr)
            );
            result = TestResult::FailedMismatch;
        };

        #define GET_D3D_INTERFACE(OBJ, INTERFACE)                                                                           \
            INTERFACE* OBJ ## _d3d = nullptr;                                                                               \
            HRESULT hr_ ## OBJ = S_OK;                                                                                      \
            if (!(OBJ ## _handle).deviceChild)                                                                              \
            {                                                                                                               \
                Log::Errorf("LLGL::Resource::GetNativeHandle() returned null pointer for \"%s\"\n", #OBJ);                  \
                result = TestResult::FailedMismatch;                                                                        \
            }                                                                                                               \
            else if ( ( hr_ ## OBJ = (OBJ ## _handle).deviceChild->QueryInterface(IID_PPV_ARGS(&(OBJ ## _d3d))) ) != S_OK ) \
            {                                                                                                               \
                FailedToQueryInterface(#INTERFACE, #OBJ, static_cast<int>(hr_ ## OBJ));                                     \
            }                                                                                                               \
            else

        auto TestBufferDescD3D11 = [this, &result](ID3D11Buffer* d3dBuffer, const BufferDescriptor& inDesc) -> void
        {
            D3D11_BUFFER_DESC d3dBufferDesc;
            d3dBuffer->GetDesc(&d3dBufferDesc);
            {
                if (opt.sanityCheck)
                {
                    Log::Printf(
                        Log::ColorFlags::StdAnnotation,
                        "D3D11_BUFFER_DESC \"%s\": ByteWidth=%d, StructureByteStride=%u\n",
                        inDesc.debugName, d3dBufferDesc.ByteWidth, d3dBufferDesc.StructureByteStride
                    );
                }

                // Internal buffer is allowed to be larger than the requested size, but it must have at least that amount.
                if (d3dBufferDesc.ByteWidth < inDesc.size)
                {
                    Log::Errorf(
                        "Mismatch between internal size (D3D11_BUFFER_DESC.ByteWidth = %u) of native resource \"%s\" and requested size (%u bytes)\n",
                        d3dBufferDesc.ByteWidth, inDesc.debugName, static_cast<std::uint32_t>(inDesc.size)
                    );
                    result = TestResult::FailedMismatch;
                }
                else
                // Structured stride must be the same.
                if (d3dBufferDesc.StructureByteStride != inDesc.stride)
                {
                    Log::Errorf(
                        "Mismatch between internal stride (D3D11_BUFFER_DESC.StructureByteStride = %u) of native resource \"%s\" and requested stride (%u bytes)\n",
                        d3dBufferDesc.StructureByteStride, inDesc.debugName, inDesc.stride
                    );
                    result = TestResult::FailedMismatch;
                }
            }
            d3dBuffer->Release();
        };

        GET_NATIVE_HANDLE(Direct3D11, buf1)
        {
            GET_D3D_INTERFACE(buf1, ID3D11Buffer)
            {
                TestBufferDescD3D11(buf1_d3d, buf1Desc);
            }
            buf1_handle.deviceChild->Release();
        }

        GET_NATIVE_HANDLE(Direct3D11, buf2)
        {
            GET_D3D_INTERFACE(buf2, ID3D11Buffer)
            {
                TestBufferDescD3D11(buf2_d3d, buf2Desc);
            }
            buf2_handle.deviceChild->Release();
        }

        GET_NATIVE_HANDLE(Direct3D11, buf3)
        {
            GET_D3D_INTERFACE(buf3, ID3D11Buffer)
            {
                TestBufferDescD3D11(buf3_d3d, buf3Desc);
            }
            buf3_handle.deviceChild->Release();
        }

        auto TestTextureDescD3D11 = [this, &result, &FailedToQueryInterface](ID3D11Resource* d3dResource, const TextureDescriptor& inDesc) -> void
        {
            D3D11_RESOURCE_DIMENSION d3dResourceDimension = D3D11_RESOURCE_DIMENSION_UNKNOWN;
            d3dResource->GetType(&d3dResourceDimension);

            // Get native texture dimension
            const std::uint32_t inTexDim = NumTextureDimensions(inDesc.type);
            const bool texDimensionsMatch =
            (
                (d3dResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE1D && inTexDim == 1) ||
                (d3dResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D && inTexDim == 2) ||
                (d3dResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D && inTexDim == 3)
            );

            const std::uint32_t numMips = NumMipLevels(inDesc);

            if (!texDimensionsMatch)
            {
                Log::Errorf(
                    "Mismatch between internal resource dimension (D3D11_RESOURCE_DIMENSION = 0x%02X)"
                    " of native resource \"%s\" and requested type (%s)",
                    static_cast<int>(d3dResourceDimension),
                    inDesc.debugName,
                    ToString(inDesc.type)
                );
                result = TestResult::FailedMismatch;
            }
            else
            {
                switch (d3dResourceDimension)
                {
                    case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
                    {
                        ID3D11Texture1D* d3dTex1D = nullptr;
                        HRESULT hr = d3dResource->QueryInterface(IID_PPV_ARGS(&d3dTex1D));
                        if (SUCCEEDED(hr))
                        {
                            D3D11_TEXTURE1D_DESC d3dTex1DDesc = {};
                            d3dTex1D->GetDesc(&d3dTex1DDesc);

                            if (opt.sanityCheck)
                            {
                                Log::Printf(
                                    Log::ColorFlags::StdAnnotation,
                                    "D3D11_TEXTURE1D_DESC \"%s\": Width=%u, ArraySize=%u, MipLevels=%u\n",
                                    inDesc.debugName,
                                    static_cast<std::uint32_t>(d3dTex1DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex1DDesc.ArraySize),
                                    static_cast<std::uint32_t>(d3dTex1DDesc.MipLevels)
                                );
                            }

                            // Internal texture size must match the requested size.
                            if (d3dTex1DDesc.Width     != inDesc.extent.width ||
                                d3dTex1DDesc.ArraySize != inDesc.arrayLayers  ||
                                d3dTex1DDesc.MipLevels != numMips)
                            {
                                Log::Errorf(
                                    "Mismatch between internal extent (D3D11_TEXTURE1D_DESC.Width = %u, .ArraySize = %u, .MipLevels = %u)"
                                    " of native resource \"%s\" and requested extent (%u, %u)\n",
                                    static_cast<std::uint32_t>(d3dTex1DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex1DDesc.ArraySize),
                                    static_cast<std::uint32_t>(d3dTex1DDesc.MipLevels),
                                    inDesc.debugName,
                                    inDesc.extent.width,
                                    inDesc.arrayLayers,
                                    numMips
                                );
                                result = TestResult::FailedMismatch;
                            }
                        }
                        else
                        {
                            FailedToQueryInterface("ID3D11Texture1D", inDesc.debugName, hr);
                        }

                        d3dTex1D->Release();
                    }
                    break;

                    case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                    {
                        ID3D11Texture2D* d3dTex2D = nullptr;
                        HRESULT hr = d3dResource->QueryInterface(IID_PPV_ARGS(&d3dTex2D));
                        if (SUCCEEDED(hr))
                        {
                            D3D11_TEXTURE2D_DESC d3dTex2DDesc = {};
                            d3dTex2D->GetDesc(&d3dTex2DDesc);

                            if (opt.sanityCheck)
                            {
                                Log::Printf(
                                    Log::ColorFlags::StdAnnotation,
                                    "D3D11_TEXTURE2D_DESC \"%s\": Width=%u, Height=%u, ArraySize=%u, MipLevels=%u\n",
                                    inDesc.debugName,
                                    static_cast<std::uint32_t>(d3dTex2DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.Height),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.ArraySize),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.MipLevels)
                                );
                            }

                            // Internal texture size must match the requested size.
                            if (d3dTex2DDesc.Width     != inDesc.extent.width  ||
                                d3dTex2DDesc.Height    != inDesc.extent.height ||
                                d3dTex2DDesc.ArraySize != inDesc.arrayLayers   ||
                                d3dTex2DDesc.MipLevels != numMips)
                            {
                                Log::Errorf(
                                    "Mismatch between internal extent (D3D11_TEXTURE2D_DESC.Width = %u, .Height = %u, .ArraySize = %u, .MipLevels = %u)"
                                    " of native resource \"%s\" and requested extent (%u, %u, %u, %u)\n",
                                    static_cast<std::uint32_t>(d3dTex2DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.Height),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.ArraySize),
                                    static_cast<std::uint32_t>(d3dTex2DDesc.MipLevels),
                                    inDesc.debugName,
                                    inDesc.extent.width,
                                    inDesc.extent.height,
                                    inDesc.arrayLayers,
                                    numMips
                                );
                                result = TestResult::FailedMismatch;
                            }
                        }
                        else
                        {
                            FailedToQueryInterface("ID3D11Texture2D", inDesc.debugName, hr);
                        }

                        d3dTex2D->Release();
                    }
                    break;

                    case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
                    {
                        ID3D11Texture3D* d3dTex3D = nullptr;
                        HRESULT hr = d3dResource->QueryInterface(IID_PPV_ARGS(&d3dTex3D));
                        if (SUCCEEDED(hr))
                        {
                            D3D11_TEXTURE3D_DESC d3dTex3DDesc = {};
                            d3dTex3D->GetDesc(&d3dTex3DDesc);

                            if (opt.sanityCheck)
                            {
                                Log::Printf(
                                    Log::ColorFlags::StdAnnotation,
                                    "D3D11_TEXTURE3D_DESC \"%s\": Width=%u, Height=%u, Depth=%u, MipLevels=%u\n",
                                    inDesc.debugName,
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Height),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Depth),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.MipLevels)
                                );
                            }

                            // Internal texture size must match the requested size.
                            if (d3dTex3DDesc.Width     != inDesc.extent.width  ||
                                d3dTex3DDesc.Height    != inDesc.extent.height ||
                                d3dTex3DDesc.Depth     != inDesc.extent.depth  ||
                                d3dTex3DDesc.MipLevels != numMips)
                            {
                                Log::Errorf(
                                    "Mismatch between internal extent (D3D11_TEXTURE3D_DESC.Width = %u, .Height = %u, .Depth = %u, .MipLevels = %u)"
                                    " of native resource \"%s\" and requested extent (%u, %u, %u, %u)\n",
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Width),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Height),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.Depth),
                                    static_cast<std::uint32_t>(d3dTex3DDesc.MipLevels),
                                    inDesc.debugName,
                                    inDesc.extent.width,
                                    inDesc.extent.height,
                                    inDesc.extent.depth,
                                    numMips
                                );
                                result = TestResult::FailedMismatch;
                            }
                        }
                        else
                        {
                            FailedToQueryInterface("ID3D11Texture3D", inDesc.debugName, hr);
                        }

                        d3dTex3D->Release();
                    }
                    break;

                    default:
                    break;
                }
            }

            d3dResource->Release();
        };

        GET_NATIVE_HANDLE(Direct3D11, tex1)
        {
            GET_D3D_INTERFACE(tex1, ID3D11Resource)
            {
                TestTextureDescD3D11(tex1_d3d, tex1Desc);
            }
            tex1_handle.deviceChild->Release();
        }

        GET_NATIVE_HANDLE(Direct3D11, tex2)
        {
            GET_D3D_INTERFACE(tex2, ID3D11Resource)
            {
                TestTextureDescD3D11(tex2_d3d, tex2Desc);
            }
            tex2_handle.deviceChild->Release();
        }

        GET_NATIVE_HANDLE(Direct3D11, tex3)
        {
            GET_D3D_INTERFACE(tex3, ID3D11Resource)
            {
                TestTextureDescD3D11(tex3_d3d, tex3Desc);
            }
            tex3_handle.deviceChild->Release();
        }

        auto TestSamplerDescD3D11 = [&result](ID3D11SamplerState* d3dSampler, const SamplerDescriptor& inDesc) -> void
        {
            D3D11_SAMPLER_DESC d3dSamplerDesc = {};
            d3dSampler->GetDesc(&d3dSamplerDesc);

            auto MatchAddressMode = [](D3D11_TEXTURE_ADDRESS_MODE d3dMode, SamplerAddressMode llglMode) -> bool
            {
                return
                (
                    (d3dMode == D3D11_TEXTURE_ADDRESS_WRAP        && llglMode == SamplerAddressMode::Repeat    ) ||
                    (d3dMode == D3D11_TEXTURE_ADDRESS_MIRROR      && llglMode == SamplerAddressMode::Mirror    ) ||
                    (d3dMode == D3D11_TEXTURE_ADDRESS_CLAMP       && llglMode == SamplerAddressMode::Clamp     ) ||
                    (d3dMode == D3D11_TEXTURE_ADDRESS_BORDER      && llglMode == SamplerAddressMode::Border    ) ||
                    (d3dMode == D3D11_TEXTURE_ADDRESS_MIRROR_ONCE && llglMode == SamplerAddressMode::MirrorOnce)
                );
            };

            constexpr float k_epsilon = 0.00001f;

            const bool matchDescs =
            (
                MatchAddressMode(d3dSamplerDesc.AddressU, inDesc.addressModeU) &&
                MatchAddressMode(d3dSamplerDesc.AddressV, inDesc.addressModeV) &&
                MatchAddressMode(d3dSamplerDesc.AddressW, inDesc.addressModeW) &&
                (inDesc.maxAnisotropy <= 1 || d3dSamplerDesc.MaxAnisotropy == inDesc.maxAnisotropy) &&
                std::abs(d3dSamplerDesc.MipLODBias - inDesc.mipMapLODBias) < k_epsilon &&
                std::abs(d3dSamplerDesc.MinLOD - inDesc.minLOD) < k_epsilon &&
                std::abs(d3dSamplerDesc.MaxLOD - inDesc.maxLOD) < k_epsilon
            );

            if (!matchDescs)
            {
                Log::Errorf("Mismatch between native sampler \"%s\" and requested descriptor\n", inDesc.debugName);
                result = TestResult::FailedMismatch;
            }
        };

        GET_NATIVE_HANDLE(Direct3D11, smpl1)
        {
            GET_D3D_INTERFACE(smpl1, ID3D11SamplerState)
            {
                TestSamplerDescD3D11(smpl1_d3d, smpl1Desc);
            }
            smpl1_handle.deviceChild->Release();
        }

        GET_NATIVE_HANDLE(Direct3D11, smpl2)
        {
            GET_D3D_INTERFACE(smpl2, ID3D11SamplerState)
            {
                TestSamplerDescD3D11(smpl2_d3d, smpl2Desc);
            }
            smpl2_handle.deviceChild->Release();
        }

        #undef GET_D3D_INTERFACE
    }

    if (rendererID == RendererID::Direct3D12)
    {
        #define GET_D3D_INTERFACE(OBJ, INTERFACE)                                                                                   \
            INTERFACE* OBJ ## _d3d = nullptr;                                                                                       \
            HRESULT hr_ ## OBJ = S_OK;                                                                                              \
            if (!(OBJ ## _handle).resource.resource)                                                                                \
            {                                                                                                                       \
                Log::Errorf("LLGL::Resource::GetNativeHandle() returned null pointer for \"%s\"\n", #OBJ);                          \
                result = TestResult::FailedMismatch;                                                                                \
            }                                                                                                                       \
            else if ( ( hr_ ## OBJ = (OBJ ## _handle).resource.resource->QueryInterface(IID_PPV_ARGS(&(OBJ ## _d3d))) ) != S_OK )   \
            {                                                                                                                       \
                Log::Errorf(                                                                                                        \
                    "LLGL::Resource::GetNativeHandle() did not provide the COM interface '%s' for \"%s\" (Error=0x%08X)\n",         \
                    #INTERFACE, #OBJ, static_cast<int>(hr_ ## OBJ)                                                                  \
                );                                                                                                                  \
                result = TestResult::FailedMismatch;                                                                                \
            }                                                                                                                       \
            else

        auto TestBufferDescD3D12 = [this, &result](ID3D12Resource* d3dResource, const BufferDescriptor& inDesc) -> void
        {
            D3D12_RESOURCE_DESC d3dResourceDesc = d3dResource->GetDesc();

            if (opt.sanityCheck)
            {
                Log::Printf(
                    Log::ColorFlags::StdAnnotation,
                    "D3D12_RESOURCE_DESC \"%s\": Dimension=0x%02X, Width=%u\n",
                    inDesc.debugName, d3dResourceDesc.Dimension, d3dResourceDesc.Width
                );
            }

            // Resource must be a buffer
            if (d3dResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                Log::Errorf(
                    "Mismatch between internal source type (%d) and expected buffer type (%d) for native resource \"%s\"\n",
                    d3dResourceDesc.Dimension, D3D12_RESOURCE_DIMENSION_BUFFER, inDesc.debugName
                );
                result = TestResult::FailedMismatch;
            }
            else
            // Internal buffer is allowed to be larger than the requested size, but it must have at least that amount.
            if (d3dResourceDesc.Width < inDesc.size)
            {
                Log::Errorf(
                    "Mismatch between internal size (D3D12_RESOURCE_DESC.Width = %u) of native resource \"%s\" and requested size (%u bytes)\n",
                    static_cast<std::uint32_t>(d3dResourceDesc.Width), inDesc.debugName, static_cast<std::uint32_t>(inDesc.size)
                );
                result = TestResult::FailedMismatch;
            }

            d3dResource->Release();
        };

        GET_NATIVE_HANDLE(Direct3D12, buf1)
        {
            GET_D3D_INTERFACE(buf1, ID3D12Resource)
            {
                TestBufferDescD3D12(buf1_d3d, buf1Desc);
            }
            buf1_handle.resource.resource->Release();
        }

        GET_NATIVE_HANDLE(Direct3D12, buf2)
        {
            GET_D3D_INTERFACE(buf2, ID3D12Resource)
            {
                TestBufferDescD3D12(buf2_d3d, buf2Desc);
            }
            buf2_handle.resource.resource->Release();
        }

        GET_NATIVE_HANDLE(Direct3D12, buf3)
        {
            GET_D3D_INTERFACE(buf3, ID3D12Resource)
            {
                TestBufferDescD3D12(buf3_d3d, buf3Desc);
            }
            buf3_handle.resource.resource->Release();
        }

        auto TestTextureDescD3D12 = [this, &result](ID3D12Resource* d3dResource, const TextureDescriptor& inDesc) -> void
        {
            D3D12_RESOURCE_DESC d3dResourceDesc = d3dResource->GetDesc();

            if (opt.sanityCheck)
            {
                Log::Printf(
                    Log::ColorFlags::StdAnnotation,
                    "D3D12_RESOURCE_DESC \"%s\": Dimension=0x%02X, Width=%u, Height=%u, DepthOrArraySize=%u\n",
                    inDesc.debugName,
                    d3dResourceDesc.Dimension,
                    static_cast<std::uint32_t>(d3dResourceDesc.Width),
                    static_cast<std::uint32_t>(d3dResourceDesc.Height),
                    static_cast<std::uint32_t>(d3dResourceDesc.DepthOrArraySize)
                );
            }

            // Get native texture dimension
            const std::uint32_t inTexDim = NumTextureDimensions(inDesc.type);
            const bool texDimensionsMatch =
            (
                (d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D && inTexDim == 1) ||
                (d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && inTexDim == 2) ||
                (d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D && inTexDim == 3)
            );

            // Convert input texture extent to native extent; Array layers are encoded differently between D3D12 and LLGL.
            const Extent3D texExtent = GetMipExtent(inDesc);

            const std::uint64_t texWidth    = texExtent.width;
            const std::uint32_t texHeight   = (inDesc.type == TextureType::Texture1DArray ? 1u : texExtent.height);
            const std::uint16_t texDepth    = static_cast<std::uint16_t>(inDesc.type == TextureType::Texture1DArray ? texExtent.height : texExtent.depth);

            // Resource must be a texture
            if (!texDimensionsMatch)
            {
                Log::Errorf(
                    "Mismatch between internal source type (%d) and expected texture type (%s) for native resource \"%s\"\n",
                    d3dResourceDesc.Dimension, ToString(inDesc.type), inDesc.debugName
                );
                result = TestResult::FailedMismatch;
            }
            else
            // Internal texture size must match the requested size.
            if (d3dResourceDesc.Width            != texWidth  ||
                d3dResourceDesc.Height           != texHeight ||
                d3dResourceDesc.DepthOrArraySize != texDepth)
            {
                Log::Errorf(
                    "Mismatch between internal extent (D3D12_RESOURCE_DESC.Width = %u, .Height = %u, .DepthOrArraySize = %u)"
                    " of native resource \"%s\" and requested extent (%u, %u, %u)\n",
                    static_cast<std::uint32_t>(d3dResourceDesc.Width),
                    static_cast<std::uint32_t>(d3dResourceDesc.Height),
                    static_cast<std::uint32_t>(d3dResourceDesc.DepthOrArraySize),
                    inDesc.debugName,
                    static_cast<std::uint32_t>(texWidth),
                    static_cast<std::uint32_t>(texHeight),
                    static_cast<std::uint32_t>(texDepth)
                );
                result = TestResult::FailedMismatch;
            }

            d3dResource->Release();
        };

        GET_NATIVE_HANDLE(Direct3D12, tex1)
        {
            GET_D3D_INTERFACE(tex1, ID3D12Resource)
            {
                TestTextureDescD3D12(tex1_d3d, tex1Desc);
            }
            tex1_handle.resource.resource->Release();
        }

        GET_NATIVE_HANDLE(Direct3D12, tex2)
        {
            GET_D3D_INTERFACE(tex2, ID3D12Resource)
            {
                TestTextureDescD3D12(tex2_d3d, tex2Desc);
            }
            tex2_handle.resource.resource->Release();
        }

        GET_NATIVE_HANDLE(Direct3D12, tex3)
        {
            GET_D3D_INTERFACE(tex3, ID3D12Resource)
            {
                TestTextureDescD3D12(tex3_d3d, tex3Desc);
            }
            tex3_handle.resource.resource->Release();
        }

        auto TestSamplerDescD3D12 = [&result](const D3D12_SAMPLER_DESC& d3dSamplerDesc, const SamplerDescriptor& inDesc) -> void
        {
            auto MatchAddressMode = [](D3D12_TEXTURE_ADDRESS_MODE d3dMode, SamplerAddressMode llglMode) -> bool
            {
                return
                (
                    (d3dMode == D3D12_TEXTURE_ADDRESS_MODE_WRAP        && llglMode == SamplerAddressMode::Repeat    ) ||
                    (d3dMode == D3D12_TEXTURE_ADDRESS_MODE_MIRROR      && llglMode == SamplerAddressMode::Mirror    ) ||
                    (d3dMode == D3D12_TEXTURE_ADDRESS_MODE_CLAMP       && llglMode == SamplerAddressMode::Clamp     ) ||
                    (d3dMode == D3D12_TEXTURE_ADDRESS_MODE_BORDER      && llglMode == SamplerAddressMode::Border    ) ||
                    (d3dMode == D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE && llglMode == SamplerAddressMode::MirrorOnce)
                );
            };

            constexpr float k_epsilon = 0.00001f;

            const bool matchDescs =
            (
                MatchAddressMode(d3dSamplerDesc.AddressU, inDesc.addressModeU) &&
                MatchAddressMode(d3dSamplerDesc.AddressV, inDesc.addressModeV) &&
                MatchAddressMode(d3dSamplerDesc.AddressW, inDesc.addressModeW) &&
                (inDesc.maxAnisotropy <= 1 || d3dSamplerDesc.MaxAnisotropy == inDesc.maxAnisotropy) &&
                std::abs(d3dSamplerDesc.MipLODBias - inDesc.mipMapLODBias) < k_epsilon &&
                std::abs(d3dSamplerDesc.MinLOD - inDesc.minLOD) < k_epsilon &&
                std::abs(d3dSamplerDesc.MaxLOD - inDesc.maxLOD) < k_epsilon
            );

            if (!matchDescs)
            {
                Log::Errorf("Mismatch between native sampler \"%s\" and requested descriptor\n", inDesc.debugName);
                result = TestResult::FailedMismatch;
            }
        };

        GET_NATIVE_HANDLE(Direct3D12, smpl1)
        {
            TestSamplerDescD3D12(smpl1_handle.samplerDesc.samplerDesc, smpl1Desc);
        }

        GET_NATIVE_HANDLE(Direct3D12, smpl2)
        {
            TestSamplerDescD3D12(smpl2_handle.samplerDesc.samplerDesc, smpl2Desc);
        }

        #undef GET_D3D_INTERFACE
    }

    #endif // /LLGL_TEST_NATIVEHANDLE_D3D

    #if LLGL_TEST_NATIVEHANDLE_MT

    if (rendererID == RendererID::Metal)
    {
        //TODO
    }

    #endif // /LLGL_TEST_NATIVEHANDLE_MT

    #if LLGL_TEST_NATIVEHANDLE_VK

    if (rendererID == RendererID::Vulkan)
    {
        GET_NATIVE_HANDLE(Vulkan, buf1)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, buf2)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, buf3)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, tex1)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, tex2)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, tex3)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, smpl1)
        {
            //TODO
        }

        GET_NATIVE_HANDLE(Vulkan, smpl2)
        {
            //TODO
        }
    }

    #endif // /LLGL_TEST_NATIVEHANDLE_VK

    #if LLGL_TEST_NATIVEHANDLE_GL

    if (rendererID == RendererID::OpenGL)
    {
        auto TestGLId = [&result](const OpenGL::ResourceNativeHandle& inGLHandle, const char* debugName) -> void
        {
            if (inGLHandle.id == 0)
            {
                Log::Errorf("Internal GL object must not be zero for native resource \"%s\"\n", debugName);
                result = TestResult::FailedMismatch;
            }
        };

        auto TestBufferDescGL = [&result, &TestGLId](const OpenGL::ResourceNativeHandle& inGLHandle, const BufferDescriptor& inDesc) -> void
        {
            TestGLId(inGLHandle, inDesc.debugName);

            if (inGLHandle.type != OpenGL::ResourceNativeType::Buffer &&
                inGLHandle.type != OpenGL::ResourceNativeType::ImmutableBuffer)
            {
                Log::Errorf(
                    "Mismatch between internal GL type (0x%02X) for native resource \"%s\" and requested type (LLGL::ResourceType::Buffer)\n",
                    static_cast<int>(inGLHandle.type), inDesc.debugName
                );
                result = TestResult::FailedMismatch;
            }
        };

        GET_NATIVE_HANDLE(OpenGL, buf1)
        {
            TestBufferDescGL(buf1_handle, buf1Desc);
        }

        GET_NATIVE_HANDLE(OpenGL, buf2)
        {
            TestBufferDescGL(buf2_handle, buf2Desc);
        }

        GET_NATIVE_HANDLE(OpenGL, buf3)
        {
            TestBufferDescGL(buf3_handle, buf3Desc);
        }

        auto TestTextureDescGL = [this, &result, &TestGLId](const OpenGL::ResourceNativeHandle& inGLHandle, const TextureDescriptor& inDesc) -> void
        {
            if (opt.sanityCheck)
            {
                Log::Printf(
                    Log::ColorFlags::StdAnnotation,
                    "GL texture \"%s\": Type=0x%02X, Extent=(%d, %d, %d), Samples=%d\n",
                    inDesc.debugName,
                    static_cast<int>(inGLHandle.type),
                    inGLHandle.texture.extent[0],
                    inGLHandle.texture.extent[1],
                    inGLHandle.texture.extent[2],
                    inGLHandle.texture.samples
                );
            }

            TestGLId(inGLHandle, inDesc.debugName);

            if (inGLHandle.type != OpenGL::ResourceNativeType::Texture          &&
                inGLHandle.type != OpenGL::ResourceNativeType::ImmutableTexture &&
                inGLHandle.type != OpenGL::ResourceNativeType::Renderbuffer     &&
                inGLHandle.type != OpenGL::ResourceNativeType::ImmutableRenderbuffer)
            {
                Log::Errorf(
                    "Mismatch between internal GL type (0x%02X) for native resource \"%s\" and requested type (%s)\n",
                    static_cast<int>(inGLHandle.type), inDesc.debugName, ToString(inDesc.type)
                );
                result = TestResult::FailedMismatch;
            }
            else
            {
                const Extent3D texExtent = GetMipExtent(inDesc);

                if (inGLHandle.texture.extent[0] != static_cast<GLint>(texExtent.width ) &&
                    inGLHandle.texture.extent[1] != static_cast<GLint>(texExtent.height) &&
                    inGLHandle.texture.extent[2] != static_cast<GLint>(texExtent.depth ))
                {
                    Log::Errorf(
                        "Mismatch between internal GL texture dimension (%d, %d, %d) for native resource \"%s\" and requested extent (%u, %u, %u)\n",
                        inGLHandle.texture.extent[0],
                        inGLHandle.texture.extent[1],
                        inGLHandle.texture.extent[2],
                        inDesc.debugName,
                        texExtent.width,
                        texExtent.height,
                        texExtent.depth
                    );
                    result = TestResult::FailedMismatch;
                }
            }
        };

        GET_NATIVE_HANDLE(OpenGL, tex1)
        {
            TestTextureDescGL(tex1_handle, tex1Desc);
        }

        GET_NATIVE_HANDLE(OpenGL, tex2)
        {
            TestTextureDescGL(tex2_handle, tex2Desc);
        }

        GET_NATIVE_HANDLE(OpenGL, tex3)
        {
            TestTextureDescGL(tex3_handle, tex3Desc);
        }

        GET_NATIVE_HANDLE(OpenGL, smpl1)
        {
            TestGLId(smpl1_handle, smpl1Desc.debugName);
        }

        GET_NATIVE_HANDLE(OpenGL, smpl2)
        {
            TestGLId(smpl2_handle, smpl2Desc.debugName);
        }
    }

    #endif // /LLGL_TEST_NATIVEHANDLE_GL

    // Release resources
    renderer->Release(*buf1);
    renderer->Release(*buf2);
    renderer->Release(*buf3);

    renderer->Release(*tex1);
    renderer->Release(*tex2);
    renderer->Release(*tex3);

    renderer->Release(*smpl1);
    renderer->Release(*smpl2);

    return result;
}

#else // LLGL_BUILD_STATIC_LIB

DEF_TEST( NativeHandle )
{
    // Only included if LLGL was built with as static lib.
    // Otherwise, all backend dependencies (D3D11.lib etc.) would have to be included separately in CMake script.
    return TestResult::Skipped;
}

#endif // /LLGL_BUILD_STATIC_LIB

