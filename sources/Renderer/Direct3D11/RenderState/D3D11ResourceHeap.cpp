/*
 * D3D11ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ResourceHeap.h"
#include "D3D11PipelineLayout.h"
#include "../Buffer/D3D11Buffer.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11Texture.h"
#include "../../CheckedCast.h"
#include "../../ResourceBindingIterator.h"


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of D3D11ResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two StorageBuffer resources (at binding points 5 and 6)
on a 32-bit build, both for the fragment shader stage only:

Offset      Attribute                                   Value   Description                                         Segment
--------------------------------------------------------------------------------------------------------------------------------------------
0x00000000  GLResourceViewHeapSegment2::segmentSize     16      Size of this segment                                \
0x00000004  GLResourceViewHeapSegment2::startSlot       4       First binding point                                  |-- Texture segment
0x00000008  GLResourceViewHeapSegment2::numSlots        1       Number of binding points                             |
0x0000000C  srv[0]                                      <ptr>   1st ID3D11ShaderResourceView for texture            /
0x00000010  GLResourceViewHeapSegment1::segmentSize     20      Size of this segment                                \
0x00000014  GLResourceViewHeapSegment1::offsetEnd0      20      Relative offset to initialCount[0] (at 0x00000028)   |
0x00000018  GLResourceViewHeapSegment1::startSlot       5       First binding point                                  |
0x0000001C  GLResourceViewHeapSegment1::numSlots        2       Number of binding points                             |-- StorageBuffer segment
0x00000020  uav[0]                                      <ptr>   1st ID3D11UnorderedAccessView for storage buffer     |
0x00000024  uav[1]                                      <ptr>   2nd ID3D11UnorderedAccessView for storage buffer     |
0x00000028  initialCount[0]                             0       1st initial count                                    |
0x0000002C  initialCount[1]                             0       2nd initial count                                   /

*/

/*
Resource view heap (RVH) segment structure with one dynamic sub-buffer
for <ID3D11SamplerState*>, <ID3D11ShaderResourceView*>, or <ID3D11Buffer*>
*/
struct D3DResourceViewHeapSegment1
{
    std::size_t segmentSize;
    UINT        startSlot;
    UINT        numViews;
};

/*
Resource view heap (RVH) segment structure with tow dynamic sub-buffers,
one for <ID3D11UnorderedAccessView*> and one for <UINT>
*/
struct D3DResourceViewHeapSegment2
{
    std::size_t segmentSize;
    std::size_t offsetEnd0;
    UINT        startSlot;
    UINT        numViews;
};

struct D3DResourceBinding
{
    UINT                            slot;
    union
    {
        ID3D11Buffer*               buffer;
        ID3D11UnorderedAccessView*  uav;
        ID3D11ShaderResourceView*   srv;
        ID3D11SamplerState*         sampler;
    };
};


/*
 * D3D11ResourceHeap class
 */

D3D11ResourceHeap::D3D11ResourceHeap(const ResourceHeapDescriptor& desc)
{
    /* Get pipeline layout object */
    auto pipelineLayoutD3D = LLGL_CAST(D3D11PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        throw std::invalid_argument("failed to create resource view heap due to missing pipeline layout");

    /* Validate binding descriptors */
    const auto& bindings = pipelineLayoutD3D->GetBindings();
    if (desc.resourceViews.size() != bindings.size())
        throw std::invalid_argument("failed to create resource vied heap due to mismatch between number of resources and bindings");

    /* Build buffer segments */
    ResourceBindingIterator resourceIterator { desc.resourceViews, bindings };

    //TODO
    #if 0
    BuildConstantBufferSegments(resourceIterator);
    BuildStorageBufferSegments(resourceIterator);
    BuildTextureSegments(resourceIterator);
    BuildSamplerSegments(resourceIterator);
    #endif
}

void D3D11ResourceHeap::BindForGraphicsPipeline(ID3D11DeviceContext* context)
{
    auto byteAlignedBuffer = buffer_.data();
    if (segmentationHeader_.hasVSResources) { BindVSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasHSResources) { BindHSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasDSResources) { BindDSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasGSResources) { BindGSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasPSResources) { BindPSResources(context, byteAlignedBuffer); }
}

void D3D11ResourceHeap::BindForComputePipeline(ID3D11DeviceContext* context)
{
    auto byteAlignedBuffer = buffer_.data();
    if (segmentationHeader_.hasCSResources) { BindCSResources(context, byteAlignedBuffer); }
}


/*
 * ======= Private: =======
 */

template <typename T>
T* const* CastToD3DViews(const std::int8_t* byteAlignedBuffer)
{
    return reinterpret_cast<T* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment1));
}

static ID3D11Buffer* const* CastToD3D11Buffers(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11Buffer>(byteAlignedBuffer);
}

static ID3D11SamplerState* const* CastToD3D11SamplerStates(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11SamplerState>(byteAlignedBuffer);
}

static ID3D11ShaderResourceView* const* CastToD3D11ShaderResourceViews(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11ShaderResourceView>(byteAlignedBuffer);
}

static ID3D11UnorderedAccessView* const* CastToD3D11UnorderedAccessView(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11UnorderedAccessView>(byteAlignedBuffer);
}

void D3D11ResourceHeap::BindVSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindHSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindDSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindGSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindPSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
            nullptr,
            nullptr,
            segment->startSlot,
            segment->numViews,
            reinterpret_cast<ID3D11UnorderedAccessView* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment2)),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindCSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->CSSetUnorderedAccessViews(
            segment->startSlot,
            segment->numViews,
            reinterpret_cast<ID3D11UnorderedAccessView* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment2)),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}


} // /namespace LLGL



// ================================================================================
