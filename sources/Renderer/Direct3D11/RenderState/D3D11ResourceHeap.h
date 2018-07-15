/*
 * D3D11ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RESOURCE_HEAP_H
#define LLGL_D3D11_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceFlags.h>
#include <vector>
#include <functional>
#include <d3d11.h>


namespace LLGL
{


class ResourceBindingIterator;
struct D3DResourceBinding;

/*
This class emulates the behavior of a descriptor heap like in D3D12,
by binding all shader resources within one bind call in the command buffer.
*/
class D3D11ResourceHeap final : public ResourceHeap
{

    public:

        D3D11ResourceHeap(const ResourceHeapDescriptor& desc);

        void BindForGraphicsPipeline(ID3D11DeviceContext* context);
        void BindForComputePipeline(ID3D11DeviceContext* context);

    private:

        using D3DResourceBindingIter = std::vector<D3DResourceBinding>::const_iterator;
        using BuildSegmentFunc = std::function<void(D3DResourceBindingIter begin, UINT count)>;

        void BuildSegmentsForStage(ResourceBindingIterator& resourceIterator, long stage);
        void BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator, long stage);
        void BuildShaderResourceViewSegments(ResourceBindingIterator& resourceIterator, long stage);
        void BuildUnorderedAccessViewSegments(ResourceBindingIterator& resourceIterator, long stage);
        void BuildSamplerSegments(ResourceBindingIterator& resourceIterator, long stage);

        void BuildAllSegments(
            const std::vector<D3DResourceBinding>&  resourceBindings,
            const BuildSegmentFunc&                 buildSegmentFunc,
            long                                    affectedStage,
            std::uint8_t&                           numSegments
        );

        void BuildAllSegmentsType1(
            const std::vector<D3DResourceBinding>& resourceBindings,
            long affectedStage,
            std::uint8_t& numSegments);
        void BuildAllSegmentsType2(const std::vector<D3DResourceBinding>& resourceBindings, long affectedStage, std::uint8_t& numSegments);

        void BuildSegment1(D3DResourceBindingIter it, UINT count);
        void BuildSegment2(D3DResourceBindingIter it, UINT count);

        void StoreResourceUsage();

        void BindVSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);
        void BindHSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);
        void BindDSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);
        void BindGSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);
        void BindPSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);
        void BindCSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer);

        /*
        Header structure to describe all segments within the raw buffer.
        - Constant buffers       are limited to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT ( 14) ==> 4 bits
        - Samplers               are limited to D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT             ( 16) ==> 5 bits
        - Shader resource views  are limited to D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT      (128) ==> 8 bits
        - Unordered access views are limited to D3D11_1_UAV_SLOT_COUNT                            ( 64) ==> 7 bits
        */
        struct SegmentationHeader
        {
            std::uint8_t hasVSResources                     : 1;
            std::uint8_t numVSConstantBufferSegments        : 4;
            std::uint8_t numVSSamplerSegments               : 5;
            std::uint8_t numVSShaderResourceViewSegments;

            std::uint8_t hasHSResources                     : 1;
            std::uint8_t numHSConstantBufferSegments        : 4;
            std::uint8_t numHSSamplerSegments               : 5;
            std::uint8_t numHSShaderResourceViewSegments;

            std::uint8_t hasDSResources                     : 1;
            std::uint8_t numDSConstantBufferSegments        : 4;
            std::uint8_t numDSSamplerSegments               : 5;
            std::uint8_t numDSShaderResourceViewSegments;

            std::uint8_t hasGSResources                     : 1;
            std::uint8_t numGSConstantBufferSegments        : 4;
            std::uint8_t numGSSamplerSegments               : 5;
            std::uint8_t numGSShaderResourceViewSegments;

            std::uint8_t hasCSResources                     : 1;
            std::uint8_t numPSUnorderedAccessViewSegments   : 7;
            std::uint8_t numPSConstantBufferSegments        : 4;
            std::uint8_t numPSSamplerSegments               : 5;
            std::uint8_t numPSShaderResourceViewSegments;

            std::uint8_t hasPSResources                     : 1;
            std::uint8_t numCSUnorderedAccessViewSegments   : 7;
            std::uint8_t numCSConstantBufferSegments        : 4;
            std::uint8_t numCSSamplerSegments               : 5;
            std::uint8_t numCSShaderResourceViewSegments;
        };

        SegmentationHeader          segmentationHeader_;
        std::uint16_t               bufferOffsetCS_         = 0;
        std::vector<std::int8_t>    buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
