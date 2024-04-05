/*
 * D3D11CommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11CommandBuffer.h"
#include "D3D11SwapChain.h"
#include "D3D11Types.h"
#include "D3D11ResourceFlags.h"
#include "../DXCommon/DXTypes.h"
#include "../CheckedCast.h"
#include "../ResourceUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/TypeInfo.h>
#include "../../Core/CoreUtils.h"
#include "../../Core/MacroUtils.h"
#include "../../Core/StringUtils.h"
#include "../../Core/Assertion.h"
#include "../TextureUtils.h"
#include <algorithm>
#include <codecvt>

#include "RenderState/D3D11StateManager.h"
#include "RenderState/D3D11PipelineState.h"
#include "RenderState/D3D11PipelineLayout.h"
#include "RenderState/D3D11ConstantsCache.h"
#include "RenderState/D3D11QueryHeap.h"
#include "RenderState/D3D11ResourceType.h"
#include "RenderState/D3D11ResourceHeap.h"
#include "RenderState/D3D11RenderPass.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11BufferWithRV.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"
#include "Texture/D3D11MipGenerator.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>


namespace LLGL
{


// Global array of null pointers to unbind resource slots
static void* const  g_nullResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]   = {};
static const UINT   g_zeroCounters[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]       = {};


D3D11CommandBuffer::D3D11CommandBuffer(
    ID3D11Device*                               device,
    const ComPtr<ID3D11DeviceContext>&          context,
    const std::shared_ptr<D3D11StateManager>&   stateMngr,
    const CommandBufferDescriptor&              desc)
:
    device_    { device    },
    context_   { context   },
    stateMngr_ { stateMngr }
{
    /* Store information whether the command buffer has an immediate or deferred context */
    if ((desc.flags & CommandBufferFlags::ImmediateSubmit) == 0)
    {
        hasDeferredContext_     = true;
        isSecondaryCmdBuffer_   = ((desc.flags & CommandBufferFlags::Secondary) != 0);
    }

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    context_->QueryInterface(IID_PPV_ARGS(&context1_));
    context_->QueryInterface(IID_PPV_ARGS(&annotation_));
    #endif
}

/* ----- Encoding ----- */

void D3D11CommandBuffer::Begin()
{
    stateMngr_->ResetStagingBufferPools();
}

void D3D11CommandBuffer::End()
{
    if (hasDeferredContext_)
    {
        /* Encode commands from deferred context into command list */
        context_->FinishCommandList(TRUE, commandList_.ReleaseAndGetAddressOf());
    }
    ResetBindingStates();
}

void D3D11CommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D11CommandBuffer&, deferredCommandBuffer);
    if (cmdBufferD3D.IsSecondaryCmdBuffer())
    {
        if (ID3D11CommandList* commandList = cmdBufferD3D.GetDeferredCommandList())
        {
            /* Execute encoded command list with immediate context and restore previous state */
            context_->ExecuteCommandList(commandList, TRUE);
        }
    }
}

/* ----- Blitting ----- */

void D3D11CommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    dstBufferD3D.WriteSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(dstOffset));
}

void D3D11CommandBuffer::CopyBuffer(
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

    context_->CopySubresourceRegion(
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
void D3D11CommandBuffer::ClearWithIntermediateUAV(ID3D11Buffer* buffer, UINT offset, UINT size, const UINT (&valuesVec4)[4])
{
    /* Create intermediate UAV for fill range */
    D3D11_BUFFER_DESC bufferDesc;
    buffer->GetDesc(&bufferDesc);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.Format              = (bufferDesc.StructureByteStride > 0 ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_UINT); // Must be DXGI_FORMAT_UNKNOWN for structured buffers
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = offset / sizeof(UINT);
        uavDesc.Buffer.NumElements  = size / sizeof(UINT);
        uavDesc.Buffer.Flags        = 0;
    };
    ComPtr<ID3D11UnorderedAccessView> intermediateUAV;
    HRESULT hr = device_->CreateUnorderedAccessView(buffer, &uavDesc, &intermediateUAV);
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", "intermediateUAV");

    /* Clear destination buffer with intermediate UAV */
    context_->ClearUnorderedAccessViewUint(intermediateUAV.Get(), valuesVec4);
}

// Internal use only (see D3D11CommandBuffer::CopyTextureFromBuffer)
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
void D3D11CommandBuffer::CopyBufferFromTexture(
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

    D3D11NativeTexture intermediateTexture;
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
            context_->CopySubresourceRegion(
                intermediateTexture.resource.Get(),                                             // pDstResource
                D3D11CalcSubresource(0, i, 1),                                                  // DstSubresource
                0,                                                                              // DstX
                0,                                                                              // DstY
                0,                                                                              // DstZ
                srcTextureD3D.GetNative().resource.Get(),                                       // pSrcResource
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
        context_.Get(),
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
    stateMngr_->SetConstants(0, &cbufferData, sizeof(cbufferData), StageFlags::ComputeStage);

    /* Store currently bound resource views */
    ID3D11UnorderedAccessView* prevUAVs[1];
    ID3D11ShaderResourceView* prevSRVs[1];

    context_->CSGetUnorderedAccessViews(0, 1, prevUAVs);
    context_->CSGetShaderResources(0, 1, prevSRVs);

    /* Bind destination texture and source buffer resourves */
    context_->CSSetUnorderedAccessViews(0, 1, intermediateUAV.GetAddressOf(), nullptr);
    context_->CSSetShaderResources(0, 1, intermediateSRV.GetAddressOf());

    /* Dispatch compute kernels with builtin shader */
    switch (textureArrayType)
    {
        case TextureType::Texture1DArray:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture1DCS, srcExtent.width, srcExtent.height, 1u);
            break;
        case TextureType::Texture2DArray:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture2DCS, srcExtent.width, srcExtent.height, srcExtent.depth);
            break;
        case TextureType::Texture3D:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyBufferFromTexture3DCS, srcExtent.width, srcExtent.height, srcExtent.depth);
            break;
        default:
            break;
    }

    /* Restore previous resource views */
    context_->CSSetUnorderedAccessViews(0, 1, prevUAVs, nullptr);
    context_->CSSetShaderResources(0, 1, prevSRVs);

    /* Copy UAV content into destination buffer, if an intermediate texture was used */
    //if (useIntermediateBuffer)
    {
        /* Copy content from intermediate buffer to destination buffer */
        const D3D11_BOX srcBox{ 0, 0, 0, copySize, 1, 1 };
        context_->CopySubresourceRegion(dstBufferD3D.GetNative(), 0, dstOffsetU32, 0, 0, intermediateBuffer.Get(), 0, &srcBox);
    }
}

void D3D11CommandBuffer::FillBuffer(
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
            context_->ClearUnorderedAccessViewUint(uav, valuesVec4);
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
            context_->CopyResource(dstBufferD3D.GetNative(), intermediateBuffer.Get());
        else
            context_->CopySubresourceRegion(dstBufferD3D.GetNative(), 0, offset, 0, 0, intermediateBuffer.Get(), 0, nullptr);
    }
}

void D3D11CommandBuffer::CopyTexture(
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

    context_->CopySubresourceRegion(
        dstTextureD3D.GetNative().resource.Get(),   // pDstResource
        dstTextureD3D.CalcSubresource(dstLocation), // DstSubresource
        static_cast<UINT>(dstOffset.x),             // DstX
        static_cast<UINT>(dstOffset.y),             // DstY
        static_cast<UINT>(dstOffset.z),             // DstZ
        srcTextureD3D.GetNative().resource.Get(),   // pSrcResource
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
void D3D11CommandBuffer::CopyTextureFromBuffer(
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

    D3D11NativeTexture intermediateTexture;
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
        context_.Get(),
        intermediateBuffer.GetAddressOf(),
        intermediateSRV.GetAddressOf(),
        nullptr,
        copySize
    );

    /* Copy content from source buffer into the intermediate buffer */
    const D3D11_BOX srcBox{ srcOffsetU32, 0, 0, srcOffsetU32 + copySize, 1, 1 };
    context_->CopySubresourceRegion(intermediateBuffer.Get(), 0, 0, 0, 0, srcBufferD3D.GetNative(), 0, &srcBox);

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
    stateMngr_->SetConstants(0, &cbufferData, sizeof(cbufferData), StageFlags::ComputeStage);

    /* Store currently bound resource views */
    ID3D11UnorderedAccessView* prevUAVs[1];
    ID3D11ShaderResourceView* prevSRVs[1];

    context_->CSGetUnorderedAccessViews(0, 1, prevUAVs);
    context_->CSGetShaderResources(0, 1, prevSRVs);

    /* Bind destination texture and source buffer resourves */
    context_->CSSetUnorderedAccessViews(0, 1, intermediateUAV.GetAddressOf(), nullptr);
    context_->CSSetShaderResources(0, 1, intermediateSRV.GetAddressOf());

    /* Dispatch compute kernels with builtin shader */
    switch (textureArrayType)
    {
        case TextureType::Texture1DArray:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyTexture1DFromBufferCS, dstExtent.width, dstExtent.height, 1u);
            break;
        case TextureType::Texture2DArray:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyTexture2DFromBufferCS, dstExtent.width, dstExtent.height, dstExtent.depth);
            break;
        case TextureType::Texture3D:
            stateMngr_->DispatchBuiltin(D3D11BuiltinShader::CopyTexture3DFromBufferCS, dstExtent.width, dstExtent.height, dstExtent.depth);
            break;
        default:
            break;
    }

    /* Restore previous resource views */
    context_->CSSetUnorderedAccessViews(0, 1, prevUAVs, nullptr);
    context_->CSSetShaderResources(0, 1, prevSRVs);

    /* Copy UAV content into destination texture, if an intermediate texture was used */
    if (useIntermediateTexture)
    {
        const UINT      mipLevel    = subresource.baseMipLevel;
        const D3D11_BOX srcBox      = { 0, 0, 0, dstExtent.width, dstExtent.height, dstExtent.depth };

        for_range(i, subresource.numArrayLayers)
        {
            const UINT arrayLayer = subresource.baseArrayLayer + i;
            context_->CopySubresourceRegion(
                dstTextureD3D.GetNative().resource.Get(),                                       // pDstResource
                D3D11CalcSubresource(mipLevel, arrayLayer, dstTextureD3D.GetNumMipLevels()),    // DstSubresource
                static_cast<UINT>(dstOffset.x),                                                 // DstX
                static_cast<UINT>(dstOffset.y),                                                 // DstY
                static_cast<UINT>(dstOffset.z),                                                 // DstZ
                intermediateTexture.resource.Get(),                                             // pSrcResource
                D3D11CalcSubresource(0, i, 1),                                                  // SrcSubresource
                &srcBox                                                                         // pSrcBox
            );
        }
    }
}

void D3D11CommandBuffer::CopyTextureFromFramebuffer(
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

    ID3D11Resource* dstResource     = dstTextureD3D.GetNativeResource();
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

    if (boundSwapChain_)
        boundSwapChain_->CopySubresourceRegion(context_.Get(), dstResource, dstSubresource, dstX, dstY, dstZ, srcBox, dstTextureD3D.GetDXFormat());
    #if 0 //TODO
    else if (boundRenderTarget_)
        boundRenderTarget_->CopySubresourceRegion();
    #endif
}

void D3D11CommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    D3D11MipGenerator::Get().GenerateMips(context_.Get(), textureD3D);
}

void D3D11CommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    D3D11MipGenerator::Get().GenerateMipsRange(
        context_.Get(),
        textureD3D,
        subresource.baseMipLevel,
        subresource.numMipLevels,
        subresource.baseArrayLayer,
        subresource.numArrayLayers
    );
}

/* ----- Viewport and Scissor ----- */

void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
{
    stateMngr_->SetViewports(1, &viewport);
}

void D3D11CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    stateMngr_->SetViewports(numViewports, viewports);
}

void D3D11CommandBuffer::SetScissor(const Scissor& scissor)
{
    stateMngr_->SetScissors(1, &scissor);
}

void D3D11CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    stateMngr_->SetScissors(numScissors, scissors);
}

/* ----- Input Assembly ------ */

void D3D11CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);

    ID3D11Buffer* buffers[] = { bufferD3D.GetNative() };
    UINT strides[] = { bufferD3D.GetStride() };
    UINT offsets[] = { 0 };

    context_->IASetVertexBuffers(0, 1, buffers, strides, offsets);
}

void D3D11CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D11BufferArray&, bufferArray);
    context_->IASetVertexBuffers(
        0,
        bufferArrayD3D.GetCount(),
        bufferArrayD3D.GetBuffers(),
        bufferArrayD3D.GetStrides(),
        bufferArrayD3D.GetOffsets()
    );
}

void D3D11CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->IASetIndexBuffer(bufferD3D.GetNative(), bufferD3D.GetDXFormat(), 0);
}

void D3D11CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->IASetIndexBuffer(bufferD3D.GetNative(), DXTypes::ToDXGIFormat(format), static_cast<UINT>(offset));
}

/* ----- Resources ----- */

void D3D11CommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    if (boundPipelineState_ == nullptr)
        return /*E_POINTER*/;

    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (context1_.Get() != nullptr)
    {
        if (boundPipelineState_->IsGraphicsPSO())
            resourceHeapD3D.BindForGraphicsPipeline1(context1_.Get(), descriptorSet);
        else
            resourceHeapD3D.BindForComputePipeline1(context1_.Get(), descriptorSet);
    }
    else
    #endif // /LLGL_D3D11_ENABLE_FEATURELEVEL
    {
        if (boundPipelineState_->IsGraphicsPSO())
            resourceHeapD3D.BindForGraphicsPipeline(context_.Get(), descriptorSet);
        else
            resourceHeapD3D.BindForComputePipeline(context_.Get(), descriptorSet);
    }
}

void D3D11CommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (boundPipelineLayout_ == nullptr)
        return /*E_POINTER*/;

    const auto& bindingList = boundPipelineLayout_->GetBindings();
    if (!(descriptor < bindingList.size()))
        return /*E_INVALIDARG*/;

    const auto& binding = bindingList[descriptor];
    switch (binding.type)
    {
        case D3DResourceType_CBV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11Buffer&, resource);
            ID3D11Buffer* cbv[] = { bufferD3D.GetNative() };
            stateMngr_->SetConstantBuffers(binding.slot, 1, cbv, binding.stageFlags);
        }
        break;

        case D3DResourceType_BufferSRV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, resource);
            ID3D11ShaderResourceView* srv[] = { bufferD3D.GetSRV() };
            stateMngr_->SetShaderResources(binding.slot, 1, srv, binding.stageFlags);
        }
        break;

        case D3DResourceType_BufferUAV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, resource);
            ID3D11UnorderedAccessView* uav[] = { bufferD3D.GetUAV() };
            UINT auvCounts[] = { bufferD3D.GetInitialCount() };
            stateMngr_->SetUnorderedAccessViews(binding.slot, 1, uav, auvCounts, binding.stageFlags);
        }
        break;

        case D3DResourceType_TextureSRV:
        {
            auto& textureD3D = LLGL_CAST(D3D11Texture&, resource);
            ID3D11ShaderResourceView* srv[] = { textureD3D.GetSRV() };
            stateMngr_->SetShaderResources(binding.slot, 1, srv, binding.stageFlags);
        }
        break;

        case D3DResourceType_TextureUAV:
        {
            auto& textureD3D = LLGL_CAST(D3D11Texture&, resource);
            ID3D11UnorderedAccessView* uav[] = { textureD3D.GetUAV() };
            UINT auvCounts[] = { 0 };
            stateMngr_->SetUnorderedAccessViews(binding.slot, 1, uav, auvCounts, binding.stageFlags);
        }
        break;

        case D3DResourceType_Sampler:
        {
            /* Set sampler state object to all shader stages */
            auto& samplerD3D = LLGL_CAST(D3D11Sampler&, resource);
            ID3D11SamplerState* samplerStates[] = { samplerD3D.GetNative() };
            stateMngr_->SetSamplers(binding.slot, 1, samplerStates, binding.stageFlags);
        }
        break;
    }
}

void D3D11CommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{
    if (numSlots > 0)
    {
        /* Reset resource binding slots */
        switch (resourceType)
        {
            case ResourceType::Undefined:
                break;
            case ResourceType::Buffer:
                ResetBufferResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
            case ResourceType::Texture:
                ResetTextureResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
            case ResourceType::Sampler:
                ResetSamplerResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
        }
    }
}

/* ----- Render Passes ----- */

void D3D11CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    /* Bind render target/context */
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
        BindSwapChain(LLGL_CAST(D3D11SwapChain&, renderTarget));
    else
        BindRenderTarget(LLGL_CAST(D3D11RenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto* renderPassD3D = LLGL_CAST(const D3D11RenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassD3D, numClearValues, clearValues);
    }
}

void D3D11CommandBuffer::EndRenderPass()
{
    /* Resolve previously bound render target (in case mutli-sampling is used) */
    ResolveAndUnbindRenderTarget();
}

static UINT GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D11_CLEAR_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D11_CLEAR_STENCIL;

    return clearFlagsDSV;
}

void D3D11CommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    /* Clear color buffers */
    if ((flags & ClearFlags::Color) != 0)
    {
        for_range(i, framebufferView_.numRenderTargetViews)
            context_->ClearRenderTargetView(framebufferView_.renderTargetViews[i], clearValue.color);
    }

    /* Clear depth-stencil buffer */
    if (framebufferView_.depthStencilView != nullptr)
    {
        if (auto clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            context_->ClearDepthStencilView(
                framebufferView_.depthStencilView,
                clearFlagsDSV,
                clearValue.depth,
                static_cast<UINT8>(clearValue.stencil & 0xFF)
            );
        }
    }
}

void D3D11CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Clear color attachment */
            if (attachments->colorAttachment < framebufferView_.numRenderTargetViews)
            {
                context_->ClearRenderTargetView(
                    framebufferView_.renderTargetViews[attachments->colorAttachment],
                    attachments->clearValue.color
                );
            }
        }
        else if (framebufferView_.depthStencilView != nullptr)
        {
            /* Clear depth and stencil buffer simultaneously */
            if (auto clearFlagsDSV = GetClearFlagsDSV(attachments->flags))
            {
                context_->ClearDepthStencilView(
                    framebufferView_.depthStencilView,
                    clearFlagsDSV,
                    attachments->clearValue.depth,
                    static_cast<UINT8>(attachments->clearValue.stencil & 0xff)
                );
            }
        }
    }
}

/* ----- Pipeline States ----- */

void D3D11CommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto pipelineStateD3D = LLGL_CAST(D3D11PipelineState*, &pipelineState);
    if (boundPipelineState_ != pipelineStateD3D)
    {
        boundPipelineState_ = pipelineStateD3D;
        boundPipelineState_->Bind(*stateMngr_);
        boundPipelineLayout_ = boundPipelineState_->GetPipelineLayout();
        boundConstantsCache_ = boundPipelineState_->GetConstantsCache();
        if (boundConstantsCache_ != nullptr)
            boundConstantsCache_->Reset();
    }
}

void D3D11CommandBuffer::SetBlendFactor(const float color[4])
{
    stateMngr_->SetBlendFactor(color);
}

void D3D11CommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    stateMngr_->SetStencilRef(reference);
}

void D3D11CommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->SetUniforms(first, data, dataSize);
}

/* ----- Queries ----- */

void D3D11CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Begin disjoint query first, and insert the beginning timestamp query */
        context_->Begin(queryHeapD3D.GetNative(query));
        context_->End(queryHeapD3D.GetNative(query + 1));
    }
    else
    {
        /* Begin standard query */
        context_->Begin(queryHeapD3D.GetNative(query));
    }
}

void D3D11CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Insert the ending timestamp query, and end the disjoint query */
        context_->End(queryHeapD3D.GetNative(query + 2));
        context_->End(queryHeapD3D.GetNative(query));
    }
    else
    {
        /* End standard query */
        context_->End(queryHeapD3D.GetNative(query));
    }
}

void D3D11CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);
    context_->SetPredication(
        queryHeapD3D.GetPredicate(query * queryHeapD3D.GetGroupSize()),
        (mode >= RenderConditionMode::WaitInverted)
    );
}

void D3D11CommandBuffer::EndRenderCondition()
{
    context_->SetPredication(nullptr, FALSE);
}

/* ----- Stream Output ------ */

void D3D11CommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    ID3D11Buffer* soTargets[LLGL_MAX_NUM_SO_BUFFERS];
    UINT offsets[LLGL_MAX_NUM_SO_BUFFERS];

    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        auto bufferD3D = LLGL_CAST(D3D11Buffer*, buffers[i]);
        soTargets[i] = bufferD3D->GetNative();
        offsets[i] = 0;
    }

    context_->SOSetTargets(numBuffers, soTargets, offsets);
}

void D3D11CommandBuffer::EndStreamOutput()
{
    context_->SOSetTargets(0, nullptr, nullptr);
}

/* ----- Drawing ----- */

void D3D11CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    FlushConstantsCache();
    context_->Draw(numVertices, firstVertex);
}

void D3D11CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    FlushConstantsCache();
    context_->DrawIndexed(numIndices, firstIndex, 0);
}

void D3D11CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushConstantsCache();
    context_->DrawIndexed(numIndices, firstIndex, vertexOffset);
}

void D3D11CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    FlushConstantsCache();
    context_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D11CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    FlushConstantsCache();
    context_->DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    FlushConstantsCache();
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushConstantsCache();
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    FlushConstantsCache();
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D11CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushConstantsCache();
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DrawInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    FlushConstantsCache();
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    while (numCommands-- > 0)
    {
        context_->DrawInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
        offset += stride;
    }
}

void D3D11CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushConstantsCache();
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DrawIndexedInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    FlushConstantsCache();
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    while (numCommands-- > 0)
    {
        context_->DrawIndexedInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
        offset += stride;
    }
}

/* ----- Compute ----- */

void D3D11CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    FlushConstantsCache();
    context_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D11CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushConstantsCache();
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DispatchIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

/* ----- Debugging ----- */

void D3D11CommandBuffer::PushDebugGroup(const char* name)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (annotation_)
    {
        std::wstring nameWStr = ToUTF16String(name);
        annotation_->BeginEvent(nameWStr.c_str());
    }
    #endif
}

void D3D11CommandBuffer::PopDebugGroup()
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (annotation_)
        annotation_->EndEvent();
    #endif
}

/* ----- Extensions ----- */

void D3D11CommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool D3D11CommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D11::CommandBufferNativeHandle))
    {
        auto* nativeHandleD3D = reinterpret_cast<Direct3D11::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleD3D->deviceContext = context_.Get();
        nativeHandleD3D->deviceContext->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void D3D11CommandBuffer::ResetBufferResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Reset vertex buffer slots */
    if ((bindFlags & BindFlags::VertexBuffer) != 0)
    {
        if ((stageFlags & StageFlags::VertexStage) != 0)
        {
            /* Clamp slot indices */
            firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) - 1);
            numSlots    = std::min(numSlots, std::uint32_t(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) - firstSlot);

            /* Unbind vertex buffers */
            context_->IASetVertexBuffers(
                firstSlot,
                numSlots,
                reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
                g_zeroCounters,
                g_zeroCounters
            );
        }
    }

    /* Reset index buffer slot */
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
    {
        if (firstSlot == 0 && (stageFlags & StageFlags::VertexStage) != 0)
            context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    }

    /* Reset constant buffer slots */
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Clamp slot indices */
        firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT) - 1);
        numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT) - firstSlot);

        /* Unbind constant buffers */
        stateMngr_->SetConstantBuffers(
            firstSlot,
            numSlots,
            reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
            stageFlags
        );
    }

    /* Reset stream-output buffer slots */
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
    {
        if (firstSlot == 0 && (stageFlags & (StageFlags::VertexStage | StageFlags::GeometryStage)) !=0 )
        {
            /* Clamp slot indices */
            firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_SO_BUFFER_SLOT_COUNT) - 1);
            numSlots    = std::min(numSlots, std::uint32_t(D3D11_SO_BUFFER_SLOT_COUNT) - firstSlot);

            /* Unbind stream-output buffers */
            context_->SOSetTargets(
                numSlots,
                reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
                g_zeroCounters
            );
        }
    }

    /* Reset sample buffer slots */
    if ((bindFlags & BindFlags::Sampled) != 0)
        ResetResourceSlotsSRV(firstSlot, numSlots, stageFlags);

    /* Reset read/write storage buffer slots */
    if ((bindFlags & BindFlags::Storage) != 0)
        ResetResourceSlotsUAV(firstSlot, numSlots, stageFlags);
}

void D3D11CommandBuffer::ResetTextureResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Reset sample buffer slots */
    if ((bindFlags & BindFlags::Sampled) != 0)
        ResetResourceSlotsSRV(firstSlot, numSlots, stageFlags);

    /* Reset read/write storage buffer slots */
    if ((bindFlags & BindFlags::Storage) != 0)
        ResetResourceSlotsUAV(firstSlot, numSlots, stageFlags);
}

void D3D11CommandBuffer::ResetSamplerResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) - firstSlot);

    /* Unbind sampler states */
    stateMngr_->SetSamplers(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11SamplerState* const*>(g_nullResources),
        stageFlags
    );
}

void D3D11CommandBuffer::ResetResourceSlotsSRV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) - firstSlot);

    /* Unbind SRVs */
    stateMngr_->SetShaderResources(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11ShaderResourceView* const*>(g_nullResources),
        stageFlags
    );
}

void D3D11CommandBuffer::ResetResourceSlotsUAV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_1_UAV_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_1_UAV_SLOT_COUNT) - firstSlot);

    /* Unbind UAVs */
    stateMngr_->SetUnorderedAccessViews(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11UnorderedAccessView* const*>(g_nullResources),
        nullptr,
        stageFlags
    );
}

void D3D11CommandBuffer::ResolveAndUnbindRenderTarget()
{
    if (boundRenderTarget_ != nullptr)
    {
        boundRenderTarget_->ResolveSubresources(context_.Get());
        boundRenderTarget_ = nullptr;
    }

    /* Set RTV list and DSV in framebuffer view */
    BindFramebufferView(0, nullptr, nullptr);

    boundSwapChain_ = nullptr;
}

void D3D11CommandBuffer::BindFramebufferView(
    UINT                            numRenderTargetViews,
    ID3D11RenderTargetView* const * renderTargetViews,
    ID3D11DepthStencilView*         depthStencilView)
{
    /* Set output-merger render target views */
    context_->OMSetRenderTargets(
        numRenderTargetViews,
        renderTargetViews,
        depthStencilView
    );

    /* Store new render-target configuration */
    framebufferView_.numRenderTargetViews   = numRenderTargetViews;
    framebufferView_.renderTargetViews      = renderTargetViews;
    framebufferView_.depthStencilView       = depthStencilView;
}

void D3D11CommandBuffer::ClearStateAndResetDeferredCommandList()
{
    if (hasDeferredContext_)
    {
        /* Clear state of deferred device context and discard partially built command list */
        if (commandList_)
        {
            context_->FinishCommandList(TRUE, commandList_.ReleaseAndGetAddressOf());
            commandList_.Reset();
        }
    }
}

void D3D11CommandBuffer::BindRenderTarget(D3D11RenderTarget& renderTargetD3D)
{
    /* Set RTV list and DSV in framebuffer view */
    BindFramebufferView(
        static_cast<UINT>(renderTargetD3D.GetRenderTargetViews().size()),
        renderTargetD3D.GetRenderTargetViews().data(),
        renderTargetD3D.GetDepthStencilView()
    );

    /* Reset references to current render target */
    boundRenderTarget_ = &renderTargetD3D;
}

void D3D11CommandBuffer::BindSwapChain(D3D11SwapChain& swapChainD3D)
{
    /* Set default RTVs to OM-stage */
    BindFramebufferView(
        1,
        swapChainD3D.GetRenderTargetViews(),
        swapChainD3D.GetDepthStencilView()
    );

    /* Reset references to current render target */
    boundSwapChain_ = &swapChainD3D;
}

void D3D11CommandBuffer::ClearAttachmentsWithRenderPass(
    const D3D11RenderPass&  renderPassD3D,
    std::uint32_t           numClearValues,
    const ClearValue*       clearValues)
{
    /* Clear color attachments */
    std::uint32_t clearValueIndex = ClearColorBuffers(renderPassD3D.GetClearColorAttachments(), numClearValues, clearValues);

    /* Clear depth-stencil attachment */
    if (framebufferView_.depthStencilView != nullptr)
    {
        if (auto clearFlagsDSV = renderPassD3D.GetClearFlagsDSV())
        {
            /* Get clear values */
            FLOAT depth     = 1.0f;
            UINT8 stencil   = 0;

            if (clearValueIndex < numClearValues)
            {
                depth   = clearValues[clearValueIndex].depth;
                stencil = static_cast<UINT8>(clearValues[clearValueIndex].stencil & 0xff);
            }

            /* Clear depth-stencil view */
            context_->ClearDepthStencilView(framebufferView_.depthStencilView, clearFlagsDSV, depth, stencil);
        }
    }
}

std::uint32_t D3D11CommandBuffer::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    numClearValues = std::min(numClearValues, framebufferView_.numRenderTargetViews);

    std::uint32_t clearValueIndex = 0;

    /* Use specified clear values */
    for_range(i, numClearValues)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        context_->ClearRenderTargetView(
            framebufferView_.renderTargetViews[colorBuffers[i]],
            clearValues[clearValueIndex].color
        );
        ++clearValueIndex;
    }

    /* Use default clear values */
    const float defaultClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for_subrange(i, numClearValues, framebufferView_.numRenderTargetViews)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        context_->ClearRenderTargetView(
            framebufferView_.renderTargetViews[colorBuffers[i]],
            defaultClearColor
        );
    }

    return clearValueIndex;
}

/*
Creates a buffer copy for the HLSL type ByteAddressBuffer.
The format must be DXGI_FORMAT_R32_TYPELESS for raw-views.
*/
void D3D11CommandBuffer::CreateByteAddressBufferR32Typeless(
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

void D3D11CommandBuffer::FlushConstantsCache()
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->Flush(*stateMngr_);
}

void D3D11CommandBuffer::ResetBindingStates()
{
    boundRenderTarget_      = nullptr;
    boundSwapChain_         = nullptr;
    boundPipelineLayout_    = nullptr;
    boundPipelineState_     = nullptr;
    boundConstantsCache_    = nullptr;
}


} // /namespace LLGL



// ================================================================================
