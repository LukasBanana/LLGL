/*
 * D3D11PrimaryCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11PrimaryCommandBuffer.h"
#include "D3D11SecondaryCommandBuffer.h"
#include "D3D11CommandExecutor.h"
#include "../D3D11SwapChain.h"
#include "../D3D11Types.h"
#include "../D3D11ResourceFlags.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../ResourceUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/TypeInfo.h>
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/Assertion.h"
#include "../../TextureUtils.h"
#include <algorithm>

#include "../RenderState/D3D11StateManager.h"
#include "../RenderState/D3D11PipelineState.h"
#include "../RenderState/D3D11PipelineLayout.h"
#include "../RenderState/D3D11ConstantsCache.h"
#include "../RenderState/D3D11QueryHeap.h"
#include "../RenderState/D3D11ResourceType.h"
#include "../RenderState/D3D11ResourceHeap.h"
#include "../RenderState/D3D11RenderPass.h"

#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11BufferArray.h"
#include "../Buffer/D3D11BufferWithRV.h"

#include "../Texture/D3D11Texture.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11RenderTarget.h"
#include "../Texture/D3D11MipGenerator.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>


namespace LLGL
{


D3D11PrimaryCommandBuffer::D3D11PrimaryCommandBuffer(
    ID3D11Device*                               device,
    const ComPtr<ID3D11DeviceContext>&          context,
    const std::shared_ptr<D3D11StateManager>&   stateMngr,
    const CommandBufferDescriptor&              desc)
:
    D3D11CommandBuffer  { /*isSecondaryCmdBuffer:*/ false                           },
    device_             { device                                                    },
    context_            { context, stateMngr                                        },
    hasDeferredContext_ { ((desc.flags & CommandBufferFlags::ImmediateSubmit) == 0) }
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    context->QueryInterface(IID_PPV_ARGS(&annotation_));
    #endif
}

/* ----- Encoding ----- */

void D3D11PrimaryCommandBuffer::Begin()
{
    GetStateManager().ResetCbufferPool();
}

void D3D11PrimaryCommandBuffer::End()
{
    if (hasDeferredContext_)
    {
        /* Encode commands from deferred context into command list */
        GetNative()->FinishCommandList(TRUE, commandList_.ReleaseAndGetAddressOf());
    }
    context_.ResetBindingStates();
}

void D3D11PrimaryCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D11CommandBuffer&, secondaryCommandBuffer);
    ExecuteD3D11CommandBuffer(cmdBufferD3D, context_);
}

/* ----- Blitting ----- */

void D3D11PrimaryCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    dstBufferD3D.WriteSubresource(GetNative(), data, static_cast<UINT>(dataSize), static_cast<UINT>(dstOffset));
}

void D3D11PrimaryCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    D3D11Buffer& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    D3D11Buffer& srcBufferD3D = LLGL_CAST(D3D11Buffer&, srcBuffer);

    const D3D11_BOX srcBox
    {
        static_cast<UINT>(srcOffset), 0u, 0u,
        static_cast<UINT>(srcOffset + size), 1u, 1u
    };

    GetNative()->CopySubresourceRegion(
        dstBufferD3D.GetNative(),       // pDstResource
        0,                              // DstSubresource
        static_cast<UINT>(dstOffset),   // DstX
        0,                              // DstY
        0,                              // DstZ
        srcBufferD3D.GetNative(),       // pSrcResource
        0,                              // SrcSubresource
        &srcBox                         // pSrcBox
    );
}

// private
void D3D11PrimaryCommandBuffer::ClearWithIntermediateUAV(ID3D11Buffer* buffer, UINT offset, UINT size, const UINT (&valuesVec4)[4])
{
    /* Create intermediate UAV for fill range */
    D3D11_BUFFER_DESC bufferDesc;
    buffer->GetDesc(&bufferDesc);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        if (bufferDesc.StructureByteStride > 0)
        {
            uavDesc.Format              = DXGI_FORMAT_UNKNOWN; // Must be DXGI_FORMAT_UNKNOWN for structured buffers
            uavDesc.Buffer.FirstElement = offset / bufferDesc.StructureByteStride;
            uavDesc.Buffer.NumElements  = size / bufferDesc.StructureByteStride;
            uavDesc.Buffer.Flags        = 0;
        }
        else
        {
            uavDesc.Format              = DXGI_FORMAT_R32_UINT;
            uavDesc.Buffer.FirstElement = offset / sizeof(UINT);
            uavDesc.Buffer.NumElements  = size / sizeof(UINT);
            uavDesc.Buffer.Flags        = 0;
        }
    };
    ComPtr<ID3D11UnorderedAccessView> intermediateUAV;
    HRESULT hr = device_->CreateUnorderedAccessView(buffer, &uavDesc, &intermediateUAV);
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", "intermediateUAV");

    /* Clear destination buffer with intermediate UAV */
    GetNative()->ClearUnorderedAccessViewUint(intermediateUAV.Get(), valuesVec4);
}

// Internal use only (see D3D11PrimaryCommandBuffer::CopyTextureFromBuffer)
struct CopyTextureBufferCbuffer
{
    std::uint32_t texOffset[3];
    std::uint32_t bufOffset;        // Source buffer offset: multiple of 4
    std::uint32_t texExtent[3];
    std::uint32_t bufIndexStride;   // Source index stride is format size clamped to [4, inf+), or 4, 8, 12, 16
    std::uint32_t formatSize;       // Bytes per pixel: 1, 2, 4, 8, 12, 16
    std::uint32_t components;       // Destination color components: 1, 2, 3, 4
    std::uint32_t componentBits;    // Bits per component: 8, 16, 32
    std::uint32_t rowStride;
    std::uint32_t layerStride;
    std::uint32_t pad0[3];          // Padding to fill up current 16-byte register
    std::uint32_t pad1[12 * 4];     // Padding to fill up constant buffer range of 256 bytes
};

// Returns a suitable array texture type if the input type allows an array texture as subresource view
static TextureType ToArrayTextureType(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return TextureType::Texture1DArray;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            return TextureType::Texture2DArray;

        default:
            return type;
    }
}

/*
D3D11 does not support copying data between buffers and textures natively,
so this function dispatches a builtin compute shader to achieve the desired effect.
Because byte address buffers are incompatible with other buffer types (like constant buffers or structured buffers),
an intermediate buffer must be copied to the destination buffer afterwards (i.e. CopySubresourceRegion from RWByteAddressBuffer to destination buffer).
*/
void D3D11PrimaryCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    auto& srcTextureD3D = LLGL_CAST(D3D11Texture&, srcTexture);

    /* Check if offsets are out of bounds or destination extent is zero */
    const auto& srcOffset = srcRegion.offset;
    if (srcOffset.x < 0 || srcOffset.y < 0 || srcOffset.z < 0)
        return;

    if (dstOffset > UINT32_MAX)
        return;

    const auto& srcExtent = srcRegion.extent;
    if (srcExtent.width == 0 || srcExtent.height == 0 || srcExtent.depth == 0)
        return;

    const UINT dstOffsetU32 = static_cast<UINT>(dstOffset);

    /* Get destination texture attributes */
    const auto& formatAttribs = GetFormatAttribs(srcTextureD3D.GetFormat());
    if ((formatAttribs.flags & (FormatFlags::IsCompressed | FormatFlags::IsPacked)) != 0 || formatAttribs.components == 0)
        return;

    /* An intermediate texture copy is required if the destination texture's format is not unsigned integer or it is normalized */
    const bool useIntermediateTexture =
    (
        (formatAttribs.flags & FormatFlags::IsUnsignedInteger) != FormatFlags::IsUnsignedInteger ||
        (formatAttribs.flags & FormatFlags::IsNormalized) != 0
    );

    /* Get actual row and layer stride */
    if (rowStride == 0 || layerStride == 0)
    {
        if (rowStride == 0)
            rowStride = (srcExtent.width * formatAttribs.bitSize / 8);
        if (layerStride == 0)
            layerStride = (srcExtent.height * rowStride);
    }

    const std::uint32_t copySize = (layerStride * srcExtent.depth);

    /* Create intermediate SRV for source texture (RWTexture1D/2D/3D) */
    const auto& subresource = srcRegion.subresource;
    const auto textureArrayType = ToArrayTextureType(srcTextureD3D.GetType());

    ComPtr<ID3D11Resource> intermediateTexture;
    ComPtr<ID3D11ShaderResourceView> intermediateSRV;

    if (useIntermediateTexture)
    {
        /* Create an intermediate copy of the destination texture with unsigned integer format */
        srcTextureD3D.CreateSubresourceCopyWithUIntFormat(
            device_,
            intermediateTexture,
            intermediateSRV.GetAddressOf(),
            nullptr,
            srcRegion,
            textureArrayType
        );

        /* Copy source texture into intermediate texture */
        const UINT      mipLevel    = subresource.baseMipLevel;
        const D3D11_BOX srcBox      =
        {
            static_cast<UINT>(srcOffset.x),
            static_cast<UINT>(srcOffset.y),
            static_cast<UINT>(srcOffset.z),
            static_cast<UINT>(srcOffset.x) + srcExtent.width,
            static_cast<UINT>(srcOffset.y) + srcExtent.height,
            static_cast<UINT>(srcOffset.z) + srcExtent.depth
        };

        for_range(i, subresource.numArrayLayers)
        {
            const UINT arrayLayer = subresource.baseArrayLayer + i;
            GetNative()->CopySubresourceRegion(
                intermediateTexture.Get(),                                                      // pDstResource
                D3D11CalcSubresource(0, i, 1),                                                  // DstSubresource
                0,                                                                              // DstX
                0,                                                                              // DstY
                0,                                                                              // DstZ
                srcTextureD3D.GetNative(),                                                      // pSrcResource
                D3D11CalcSubresource(mipLevel, arrayLayer, srcTextureD3D.GetNumMipLevels()),    // SrcSubresource
                &srcBox                                                                         // pSrcBox
            );
        }
    }
    else
    {
        /* Create intermediate UAV directly from destination texture if the texture already has an unsigned integer format */
        srcTextureD3D.CreateSubresourceSRV(
            device_,
            intermediateSRV.GetAddressOf(),
            textureArrayType,
            srcTextureD3D.GetBaseDXFormat(),
            subresource.baseMipLevel,
            1,
            subresource.baseArrayLayer,
            subresource.numArrayLayers
        );
    }

    //TODO: check if intermediate UAV is necessary
    /* Create intermediate byte-addressable buffer with UAV (RWByteAddressBuffer) */
    ComPtr<ID3D11Buffer> intermediateBuffer;
    ComPtr<ID3D11UnorderedAccessView> intermediateUAV;

    CreateByteAddressBufferR32Typeless(
        device_,
        GetNative(),
        intermediateBuffer.GetAddressOf(),
        nullptr,
        intermediateUAV.GetAddressOf(),
        copySize
    );

    /* Set shader parameters with intermediate constant buffer */
    CopyTextureBufferCbuffer cbufferData;
    {
        if (useIntermediateTexture)
        {
            cbufferData.texOffset[0]    = 0;
            cbufferData.texOffset[1]    = 0;
            cbufferData.texOffset[2]    = 0;
        }
        else
        {
            cbufferData.texOffset[0]    = static_cast<std::uint32_t>(srcOffset.x);
            cbufferData.texOffset[1]    = static_cast<std::uint32_t>(srcOffset.y);
            cbufferData.texOffset[2]    = static_cast<std::uint32_t>(srcOffset.z);
        }
        cbufferData.bufOffset           = 0;
        cbufferData.texExtent[0]        = srcExtent.width;
        cbufferData.texExtent[1]        = srcExtent.height;
        cbufferData.texExtent[2]        = srcExtent.depth;
        cbufferData.bufIndexStride      = std::max(4u, formatAttribs.bitSize / 8u);
        cbufferData.formatSize          = formatAttribs.bitSize / 8;
        cbufferData.components          = formatAttribs.components;
        cbufferData.componentBits       = formatAttribs.bitSize / formatAttribs.components;
        cbufferData.rowStride           = rowStride;
        cbufferData.layerStride         = layerStride;
    }
    GetStateManager().SetConstants(0, &cbufferData, sizeof(cbufferData), StageFlags::ComputeStage);

    /* Store currently bound resource views */
    ID3D11UnorderedAccessView* prevUAVs[1];
    ID3D11ShaderResourceView* prevSRVs[1];

    GetNative()->CSGetUnorderedAccessViews(0, 1, prevUAVs);
    GetNative()->CSGetShaderResources(0, 1, prevSRVs);

    /* Bind destination texture and source buffer resourves */
    GetNative()->CSSetUnorderedAccessViews(0, 1, intermediateUAV.GetAddressOf(), nullptr);
    GetNative()->CSSetShaderResources(0, 1, intermediateSRV.GetAddressOf());

    /* Dispatch compute kernels with builtin shader */
    switch (textureArrayType)
    {
        case TextureType::Texture1DArray:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture1DCS, srcExtent.width, srcExtent.height, 1u);
            break;
        case TextureType::Texture2DArray:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture2DCS, srcExtent.width, srcExtent.height, srcExtent.depth);
            break;
        case TextureType::Texture3D:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture3DCS, srcExtent.width, srcExtent.height, srcExtent.depth);
            break;
        default:
            break;
    }

    /* Restore previous resource views */
    GetNative()->CSSetUnorderedAccessViews(0, 1, prevUAVs, nullptr);
    GetNative()->CSSetShaderResources(0, 1, prevSRVs);

    /* Copy UAV content into destination buffer, if an intermediate texture was used */
    //if (useIntermediateBuffer)
    {
        /* Copy content from intermediate buffer to destination buffer */
        const D3D11_BOX srcBox{ 0, 0, 0, copySize, 1, 1 };
        GetNative()->CopySubresourceRegion(dstBufferD3D.GetNative(), 0, dstOffsetU32, 0, 0, intermediateBuffer.Get(), 0, &srcBox);
    }

    GetStateManager().ResetCbufferPool();
}

void D3D11PrimaryCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);

    /* Copy value to 4D vector to be used with native D3D11 clear functions */
    UINT valuesVec4[4] = { value, value, value, value };

    /* Clamp range to buffer size if whole buffer is meant to be filled */
    if (fillSize == LLGL_WHOLE_SIZE)
    {
        dstOffset   = 0;
        fillSize    = dstBufferD3D.GetSize();
    }

    const bool isWholeBufferRange   = (dstOffset == 0 && fillSize == dstBufferD3D.GetSize());
    const UINT offset               = static_cast<UINT>(dstOffset);
    const UINT size                 = static_cast<UINT>(fillSize);

    if ((dstBufferD3D.GetBindFlags() & BindFlags::Storage) != 0)
    {
        auto& dstBufferUAV = LLGL_CAST(D3D11BufferWithRV&, dstBufferD3D);
        auto uav = dstBufferUAV.GetUAV();

        if (uav != nullptr &&
            isWholeBufferRange &&
            DXTypes::MakeUAVClearVector(dstBufferUAV.GetDXFormat(), valuesVec4, value))
        {
            /* Fill destination buffer directly with primary UAV */
            GetNative()->ClearUnorderedAccessViewUint(uav, valuesVec4);
        }
        else
        {
            /* Fill destination buffer with intermediate UAV */
            ClearWithIntermediateUAV(dstBufferD3D.GetNative(), offset, size, valuesVec4);
        }
    }
    else
    {
        /* Create intermediate buffer with UAV */
        D3D11_BUFFER_DESC bufferDesc;
        {
            bufferDesc.ByteWidth            = static_cast<UINT>(fillSize);
            bufferDesc.Usage                = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags            = D3D11_BIND_UNORDERED_ACCESS;
            bufferDesc.CPUAccessFlags       = 0;
            bufferDesc.MiscFlags            = 0;
            bufferDesc.StructureByteStride  = sizeof(UINT);
        }
        ComPtr<ID3D11Buffer> intermediateBuffer;
        device_->CreateBuffer(&bufferDesc, nullptr, &intermediateBuffer);

        /* Fill destination buffer with intermediate UAV */
        ClearWithIntermediateUAV(intermediateBuffer.Get(), 0, size, valuesVec4);

        /* Copy intermediate buffer into destination buffer */
        if (isWholeBufferRange)
            GetNative()->CopyResource(dstBufferD3D.GetNative(), intermediateBuffer.Get());
        else
            GetNative()->CopySubresourceRegion(dstBufferD3D.GetNative(), 0, offset, 0, 0, intermediateBuffer.Get(), 0, nullptr);
    }
}

void D3D11PrimaryCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureD3D = LLGL_CAST(D3D11Texture&, dstTexture);
    auto& srcTextureD3D = LLGL_CAST(D3D11Texture&, srcTexture);

    const Offset3D  dstOffset   = CalcTextureOffset(dstTexture.GetType(), dstLocation.offset);
    const D3D11_BOX srcBox      = srcTextureD3D.CalcRegion(srcLocation.offset, extent);

    GetNative()->CopySubresourceRegion(
        dstTextureD3D.GetNative(),                  // pDstResource
        dstTextureD3D.CalcSubresource(dstLocation), // DstSubresource
        static_cast<UINT>(dstOffset.x),             // DstX
        static_cast<UINT>(dstOffset.y),             // DstY
        static_cast<UINT>(dstOffset.z),             // DstZ
        srcTextureD3D.GetNative(),                  // pSrcResource
        srcTextureD3D.CalcSubresource(srcLocation), // SrcSubresource
        &srcBox                                     // pSrcBox
    );
}

/*
D3D11 does not support copying data between buffers and textures natively,
so this function dispatches a builtin compute shader to achieve the desired effect.
Because byte address buffers are incompatible with other buffer types (like constant buffers or structured buffers),
an intermediate buffer must be copied from the source buffer first (i.e. CopySubresourceRegion from source buffer into ByteAddressBuffer).
*/
void D3D11PrimaryCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureD3D = LLGL_CAST(D3D11Texture&, dstTexture);
    auto& srcBufferD3D = LLGL_CAST(D3D11Buffer&, srcBuffer);

    /* Check if offsets are out of bounds or destination extent is zero */
    const auto& dstOffset = dstRegion.offset;
    if (dstOffset.x < 0 || dstOffset.y < 0 || dstOffset.z < 0)
        return;

    if (srcOffset > UINT32_MAX)
        return;

    const auto& dstExtent = dstRegion.extent;
    if (dstExtent.width == 0 || dstExtent.height == 0 || dstExtent.depth == 0)
        return;

    const UINT srcOffsetU32 = static_cast<UINT>(srcOffset);

    /* Get destination texture attributes */
    const auto& formatAttribs = GetFormatAttribs(dstTextureD3D.GetFormat());
    if ((formatAttribs.flags & (FormatFlags::IsCompressed | FormatFlags::IsPacked)) != 0 || formatAttribs.components == 0)
        return;

    /* An intermediate texture copy is required if the destination texture's format is not unsigned integer or it is normalized */
    const bool useIntermediateTexture =
    (
        (formatAttribs.flags & FormatFlags::IsUnsignedInteger) != FormatFlags::IsUnsignedInteger ||
        (formatAttribs.flags & FormatFlags::IsNormalized) != 0
    );

    /* Get actual row and layer stride */
    if (rowStride == 0 || layerStride == 0)
    {
        if (rowStride == 0)
            rowStride = (dstExtent.width * formatAttribs.bitSize / 8);
        if (layerStride == 0)
            layerStride = (dstExtent.height * rowStride);
    }

    const std::uint32_t copySize = (layerStride * dstExtent.depth);

    /* Create intermediate UAV for destination texture (RWTexture1D/2D/3D) */
    const auto& subresource = dstRegion.subresource;
    const auto textureArrayType = ToArrayTextureType(dstTextureD3D.GetType());

    ComPtr<ID3D11Resource> intermediateTexture;
    ComPtr<ID3D11UnorderedAccessView> intermediateUAV;

    if (useIntermediateTexture)
    {
        /* Create an intermediate copy of the destination texture with unsigned integer format */
        dstTextureD3D.CreateSubresourceCopyWithUIntFormat(
            device_,
            intermediateTexture,
            nullptr,
            intermediateUAV.GetAddressOf(),
            dstRegion,
            textureArrayType
        );
    }
    else
    {
        /* Create intermediate UAV directly from destination texture if the texture already has an unsigned integer format */
        dstTextureD3D.CreateSubresourceUAV(
            device_,
            intermediateUAV.GetAddressOf(),
            textureArrayType,
            dstTextureD3D.GetBaseDXFormat(),
            subresource.baseMipLevel,
            subresource.baseArrayLayer,
            subresource.numArrayLayers
        );
    }

    //TODO: check if intermediate SRV is necessary
    /* Create intermediate byte-addressable buffer with SRV (ByteAddressBuffer) */
    ComPtr<ID3D11Buffer> intermediateBuffer;
    ComPtr<ID3D11ShaderResourceView> intermediateSRV;

    CreateByteAddressBufferR32Typeless(
        device_,
        GetNative(),
        intermediateBuffer.GetAddressOf(),
        intermediateSRV.GetAddressOf(),
        nullptr,
        copySize
    );

    /* Copy content from source buffer into the intermediate buffer */
    const D3D11_BOX srcBox{ srcOffsetU32, 0, 0, srcOffsetU32 + copySize, 1, 1 };
    GetNative()->CopySubresourceRegion(intermediateBuffer.Get(), 0, 0, 0, 0, srcBufferD3D.GetNative(), 0, &srcBox);

    /* Set shader parameters with intermediate constant buffer */
    CopyTextureBufferCbuffer cbufferData;
    {
        if (useIntermediateTexture)
        {
            cbufferData.texOffset[0]    = 0;
            cbufferData.texOffset[1]    = 0;
            cbufferData.texOffset[2]    = 0;
        }
        else
        {
            cbufferData.texOffset[0]    = static_cast<std::uint32_t>(dstOffset.x);
            cbufferData.texOffset[1]    = static_cast<std::uint32_t>(dstOffset.y);
            cbufferData.texOffset[2]    = static_cast<std::uint32_t>(dstOffset.z);
        }
        cbufferData.bufOffset           = 0;
        cbufferData.texExtent[0]        = dstExtent.width;
        cbufferData.texExtent[1]        = dstExtent.height;
        cbufferData.texExtent[2]        = dstExtent.depth;
        cbufferData.bufIndexStride      = std::max(4u, formatAttribs.bitSize / 8u);
        cbufferData.formatSize          = formatAttribs.bitSize / 8;
        cbufferData.components          = formatAttribs.components;
        cbufferData.componentBits       = formatAttribs.bitSize / formatAttribs.components;
        cbufferData.rowStride           = rowStride;
        cbufferData.layerStride         = layerStride;
    }
    GetStateManager().SetConstants(0, &cbufferData, sizeof(cbufferData), StageFlags::ComputeStage);

    /* Store currently bound resource views */
    ID3D11UnorderedAccessView* prevUAVs[1];
    ID3D11ShaderResourceView* prevSRVs[1];

    GetNative()->CSGetUnorderedAccessViews(0, 1, prevUAVs);
    GetNative()->CSGetShaderResources(0, 1, prevSRVs);

    /* Bind destination texture and source buffer resourves */
    GetNative()->CSSetUnorderedAccessViews(0, 1, intermediateUAV.GetAddressOf(), nullptr);
    GetNative()->CSSetShaderResources(0, 1, intermediateSRV.GetAddressOf());

    /* Dispatch compute kernels with builtin shader */
    switch (textureArrayType)
    {
        case TextureType::Texture1DArray:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyTexture1DFromBufferCS, dstExtent.width, dstExtent.height, 1u);
            break;
        case TextureType::Texture2DArray:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyTexture2DFromBufferCS, dstExtent.width, dstExtent.height, dstExtent.depth);
            break;
        case TextureType::Texture3D:
            GetStateManager().DispatchBuiltin(D3D11BuiltinShader::CopyTexture3DFromBufferCS, dstExtent.width, dstExtent.height, dstExtent.depth);
            break;
        default:
            break;
    }

    /* Restore previous resource views */
    GetNative()->CSSetUnorderedAccessViews(0, 1, prevUAVs, nullptr);
    GetNative()->CSSetShaderResources(0, 1, prevSRVs);

    /* Copy UAV content into destination texture, if an intermediate texture was used */
    if (useIntermediateTexture)
    {
        const UINT      mipLevel    = subresource.baseMipLevel;
        const D3D11_BOX srcBox      = { 0, 0, 0, dstExtent.width, dstExtent.height, dstExtent.depth };

        for_range(i, subresource.numArrayLayers)
        {
            const UINT arrayLayer = subresource.baseArrayLayer + i;
            GetNative()->CopySubresourceRegion(
                dstTextureD3D.GetNative(),                                                      // pDstResource
                D3D11CalcSubresource(mipLevel, arrayLayer, dstTextureD3D.GetNumMipLevels()),    // DstSubresource
                static_cast<UINT>(dstOffset.x),                                                 // DstX
                static_cast<UINT>(dstOffset.y),                                                 // DstY
                static_cast<UINT>(dstOffset.z),                                                 // DstZ
                intermediateTexture.Get(),                                                      // pSrcResource
                D3D11CalcSubresource(0, i, 1),                                                  // SrcSubresource
                &srcBox                                                                         // pSrcBox
            );
        }
    }

    GetStateManager().ResetCbufferPool();
}

void D3D11PrimaryCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    if (dstRegion.extent.depth != 1 ||
        dstRegion.offset.x < 0      ||
        dstRegion.offset.y < 0      ||
        dstRegion.offset.z < 0)
    {
        return /*E_INVALIDARG*/;
    }

    auto& dstTextureD3D = LLGL_CAST(D3D11Texture&, dstTexture);

    ID3D11Resource* dstResource     = dstTextureD3D.GetNative();
    UINT            dstSubresource  = dstTextureD3D.CalcSubresource(dstRegion.subresource.baseMipLevel, dstRegion.subresource.baseArrayLayer);
    UINT            dstX            = static_cast<UINT>(dstRegion.offset.x);
    UINT            dstY            = static_cast<UINT>(dstRegion.offset.y);
    UINT            dstZ            = static_cast<UINT>(dstRegion.offset.z);

    const D3D11_BOX srcBox
    {
        static_cast<UINT>(srcOffset.x),
        static_cast<UINT>(srcOffset.y),
        0u,
        static_cast<UINT>(srcOffset.x) + dstRegion.extent.width,
        static_cast<UINT>(srcOffset.y) + dstRegion.extent.height,
        1u,
    };

    if (D3D11SwapChain* boundSwapChain = context_.GetBoundSwapChain())
        boundSwapChain->CopySubresourceRegion(GetNative(), dstResource, dstSubresource, dstX, dstY, dstZ, srcBox, dstTextureD3D.GetDXFormat());
    #if 0 //TODO
    else if (D3D11RenderTarget* boundRenderTarget = context_.GetBoundRenderTarget())
        boundRenderTarget->CopySubresourceRegion();
    #endif
}

void D3D11PrimaryCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    D3D11MipGenerator::Get().GenerateMips(GetNative(), textureD3D);
}

void D3D11PrimaryCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    D3D11MipGenerator::Get().GenerateMipsRange(
        GetNative(),
        textureD3D,
        subresource.baseMipLevel,
        subresource.numMipLevels,
        subresource.baseArrayLayer,
        subresource.numArrayLayers
    );
}

/* ----- Viewport and Scissor ----- */

void D3D11PrimaryCommandBuffer::SetViewport(const Viewport& viewport)
{
    GetStateManager().SetViewports(1, &viewport);
}

void D3D11PrimaryCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    GetStateManager().SetViewports(numViewports, viewports);
}

void D3D11PrimaryCommandBuffer::SetScissor(const Scissor& scissor)
{
    GetStateManager().SetScissors(1, &scissor);
}

void D3D11PrimaryCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    GetStateManager().SetScissors(numScissors, scissors);
}

/* ----- Input Assembly ------ */

void D3D11PrimaryCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.SetVertexBuffer(bufferD3D);
}

void D3D11PrimaryCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D11BufferArray&, bufferArray);
    context_.SetVertexBufferArray(bufferArrayD3D);
}

void D3D11PrimaryCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.SetIndexBuffer(bufferD3D, bufferD3D.GetDXFormat(), 0);
}

void D3D11PrimaryCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.SetIndexBuffer(bufferD3D, DXTypes::ToDXGIFormat(format), static_cast<UINT>(offset));
}

/* ----- Resources ----- */

void D3D11PrimaryCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);
    (void)context_.SetResourceHeap(resourceHeapD3D, descriptorSet);
}

void D3D11PrimaryCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    (void)context_.SetResource(descriptor, resource);
}

void D3D11PrimaryCommandBuffer::ResourceBarrier(
    std::uint32_t       /*numBuffers*/,
    Buffer* const *     /*buffers*/,
    std::uint32_t       /*numTextures*/,
    Texture* const *    /*textures*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void D3D11PrimaryCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    /* Bind render target/context */
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainD3D = LLGL_CAST(D3D11SwapChain&, renderTarget);
        context_.BindSwapChainRenderTargets(swapChainD3D);
    }
    else
    {
        auto& renderTargetD3D = LLGL_CAST(D3D11RenderTarget&, renderTarget);
        context_.BindOffscreenRenderTargets(renderTargetD3D);
    }

    /* Clear attachments */
    if (renderPass != nullptr)
    {
        auto* renderPassD3D = LLGL_CAST(const D3D11RenderPass*, renderPass);
        context_.ClearFramebufferViewsOrdered(
            numClearValues,
            clearValues,
            renderPassD3D->GetClearColorAttachments(),
            renderPassD3D->GetClearFlagsDSV()
        );
    }
}

void D3D11PrimaryCommandBuffer::EndRenderPass()
{
    /* Resolve previously bound render target (in case mutli-sampling is used) */
    context_.ResolveAndUnbindRenderTargets();
}

void D3D11PrimaryCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    context_.ClearFramebufferViewsSimple(flags, clearValue);
}

void D3D11PrimaryCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    context_.ClearFramebufferViewsIndexed(numAttachments, attachments);
}

/* ----- Pipeline States ----- */

void D3D11PrimaryCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto* pipelineStateD3D = LLGL_CAST(D3D11PipelineState*, &pipelineState);
    context_.SetPipelineState(pipelineStateD3D);
}

void D3D11PrimaryCommandBuffer::SetBlendFactor(const float color[4])
{
    GetStateManager().SetBlendFactor(color);
}

void D3D11PrimaryCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    GetStateManager().SetStencilRef(reference);
}

void D3D11PrimaryCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    context_.SetUniforms(first, data, dataSize);
}

/* ----- Queries ----- */

void D3D11PrimaryCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Begin disjoint query first, and insert the beginning timestamp query */
        GetNative()->Begin(queryHeapD3D.GetNative(query));
        GetNative()->End(queryHeapD3D.GetNative(query + 1));
    }
    else
    {
        /* Begin standard query */
        GetNative()->Begin(queryHeapD3D.GetNative(query));
    }
}

void D3D11PrimaryCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Insert the ending timestamp query, and end the disjoint query */
        GetNative()->End(queryHeapD3D.GetNative(query + 2));
        GetNative()->End(queryHeapD3D.GetNative(query));
    }
    else
    {
        /* End standard query */
        GetNative()->End(queryHeapD3D.GetNative(query));
    }
}

void D3D11PrimaryCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);
    GetNative()->SetPredication(
        queryHeapD3D.GetPredicate(query * queryHeapD3D.GetGroupSize()),
        (mode >= RenderConditionMode::WaitInverted)
    );
}

void D3D11PrimaryCommandBuffer::EndRenderCondition()
{
    GetNative()->SetPredication(nullptr, FALSE);
}

/* ----- Stream Output ------ */

void D3D11PrimaryCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    D3D11BindingLocator* locators[LLGL_MAX_NUM_SO_BUFFERS];
    ID3D11Buffer* soTargets[LLGL_MAX_NUM_SO_BUFFERS];
    UINT offsets[LLGL_MAX_NUM_SO_BUFFERS];

    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    for_range(i, numBuffers)
    {
        auto* bufferD3D = LLGL_CAST(D3D11Buffer*, buffers[i]);
        locators[i]     = bufferD3D->GetBindingLocator();
        soTargets[i]    = bufferD3D->GetNative();
        offsets[i]      = 0;
    }

    GetBindingTable().SetStreamOutputBuffers(numBuffers, soTargets, offsets, locators);
}

void D3D11PrimaryCommandBuffer::EndStreamOutput()
{
    GetBindingTable().SetStreamOutputBuffers(0, nullptr, nullptr, nullptr);
}

/* ----- Drawing ----- */

void D3D11PrimaryCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    context_.Draw(numVertices, firstVertex);
}

void D3D11PrimaryCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    context_.DrawIndexed(numIndices, firstIndex, 0);
}

void D3D11PrimaryCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    context_.DrawIndexed(numIndices, firstIndex, vertexOffset);
}

void D3D11PrimaryCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    context_.DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D11PrimaryCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    context_.DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D11PrimaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    context_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D11PrimaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    context_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D11PrimaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    context_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D11PrimaryCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.DrawInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11PrimaryCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.DrawInstancedIndirectN(bufferD3D.GetNative(), static_cast<UINT>(offset), numCommands, stride);
}

void D3D11PrimaryCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.DrawIndexedInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11PrimaryCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.DrawIndexedInstancedIndirectN(bufferD3D.GetNative(), static_cast<UINT>(offset), numCommands, stride);
}

void D3D11PrimaryCommandBuffer::DrawStreamOutput()
{
    context_.DrawAuto();
}

/* ----- Compute ----- */

void D3D11PrimaryCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    context_.Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D11PrimaryCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_.DispatchIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

/* ----- Debugging ----- */

void D3D11PrimaryCommandBuffer::PushDebugGroup(const char* name)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (annotation_)
    {
        const std::wstring nameWStr = ToWideString(name);
        annotation_->BeginEvent(nameWStr.c_str());
    }
    #endif
}

void D3D11PrimaryCommandBuffer::PopDebugGroup()
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (annotation_)
        annotation_->EndEvent();
    #endif
}

/* ----- Extensions ----- */

void D3D11PrimaryCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool D3D11PrimaryCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D11::CommandBufferNativeHandle))
    {
        auto* nativeHandleD3D = static_cast<Direct3D11::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleD3D->deviceContext = GetNative();
        nativeHandleD3D->deviceContext->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void D3D11PrimaryCommandBuffer::ClearStateAndResetDeferredCommandList()
{
    if (hasDeferredContext_)
    {
        /* Clear state of deferred device context and discard partially built command list */
        if (commandList_)
        {
            GetNative()->FinishCommandList(TRUE, commandList_.ReleaseAndGetAddressOf());
            commandList_.Reset();
        }
    }
}

/*
Creates a buffer copy for the HLSL type ByteAddressBuffer.
The format must be DXGI_FORMAT_R32_TYPELESS for raw-views.
*/
void D3D11PrimaryCommandBuffer::CreateByteAddressBufferR32Typeless(
    ID3D11Device*               device,
    ID3D11DeviceContext*        context,
    ID3D11Buffer**              bufferOutput,
    ID3D11ShaderResourceView**  srvOutput,
    ID3D11UnorderedAccessView** uavOutput,
    UINT                        size,
    D3D11_USAGE                 usage)
{
    LLGL_ASSERT_PTR(bufferOutput);

    /* Align size to R32 format size */
    size = GetAlignedSize(size, 4u);

    /* Determine binding flags depending on resource-view output */
    UINT bindFlags = 0;
    if (srvOutput != nullptr)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (uavOutput != nullptr)
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    /* Create output buffer with raw view accesss */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = size;
        descD3D.Usage               = usage;
        descD3D.BindFlags           = bindFlags;
        descD3D.CPUAccessFlags      = 0;
        descD3D.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        descD3D.StructureByteStride = 0;
    }
    HRESULT hr = device->CreateBuffer(&descD3D, nullptr, bufferOutput);
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for byte addressable copy");

    if (ID3D11Resource* resource = *bufferOutput)
    {
        /* Create shader-resource-view (SRV) */
        if (srvOutput != nullptr)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            {
                srvDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
                srvDesc.BufferEx.FirstElement   = 0;
                srvDesc.BufferEx.NumElements    = size / 4;
                srvDesc.BufferEx.Flags          = D3D11_BUFFEREX_SRV_FLAG_RAW;
            }
            device->CreateShaderResourceView(resource, &srvDesc, srvOutput);
        }

        /* Create optional unordered-access-view (UAV) */
        if (uavOutput != nullptr)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            {
                uavDesc.Format              = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements  = size / 4;
                uavDesc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_RAW;
            }
            device->CreateUnorderedAccessView(resource, &uavDesc, uavOutput);
        }
    }
}


} // /namespace LLGL



// ================================================================================
