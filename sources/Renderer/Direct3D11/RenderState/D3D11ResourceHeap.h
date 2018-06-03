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

class D3D11ResourceHeap : public ResourceHeap
{

    public:

        D3D11ResourceHeap(const ResourceHeapDescriptor& desc);

        void BindForGraphicsPipeline(ID3D11DeviceContext* context);
        void BindForComputePipeline(ID3D11DeviceContext* context);

    private:

        //TODO
        #if 0
        void BuildBufferSegments(ResourceBindingIterator& resourceIterator, const ResourceType resourceType, std::uint8_t& numSegments);
        void BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator);
        void BuildShaderResourceViewSegments(ResourceBindingIterator& resourceIterator);
        void BuildUnorderedAccessViewSegments(ResourceBindingIterator& resourceIterator);
        void BuildSamplerSegments(ResourceBindingIterator& resourceIterator);
        #endif

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
            std::uint8_t numVSSamplerSegments               : 5;
            std::uint8_t numVSConstantBufferSegments        : 4;
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
        std::vector<std::int8_t>    buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
