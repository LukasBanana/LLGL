/*
 * D3D11ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RESOURCE_HEAP_H
#define LLGL_D3D11_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceFlags.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <functional>
#include "../Direct3D11.h"


namespace LLGL
{


class D3D11Texture;
class D3D11BufferWithRV;
class ResourceBindingIterator;
struct ResourceHeapDescriptor;
struct TextureViewDescriptor;
struct BufferViewDescriptor;
struct D3DResourceBinding;

/*
This class emulates the behavior of a descriptor heap like in D3D12,
by binding all shader resources within one bind call in the command buffer.
*/
class D3D11ResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        D3D11ResourceHeap(const ResourceHeapDescriptor& desc, bool hasDeviceContextD3D11_1);

        void BindForGraphicsPipeline(ID3D11DeviceContext* context, std::uint32_t firstSet);
        void BindForComputePipeline(ID3D11DeviceContext* context, std::uint32_t firstSet);

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

        void BindForGraphicsPipeline1(ID3D11DeviceContext1* context1, std::uint32_t firstSet);
        void BindForComputePipeline1(ID3D11DeviceContext1* context1, std::uint32_t firstSet);

        #endif

        // Returns true if this resource heap contains non-default constant-buffer ranges (requires feature level D3D_FEATURE_LEVEL_11_1).
        bool HasCbufferRanges() const;

    private:

        using D3DResourceBindingIter = std::vector<D3DResourceBinding>::const_iterator;
        using BuildSegmentFunc = std::function<void(D3DResourceBindingIter begin, UINT count)>;

        void BuildSegmentsForStage(ResourceBindingIterator& resourceIterator, long stage);
        void BuildConstantBufferRangeSegments(ResourceBindingIterator& resourceIterator, long stage);
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
            const std::vector<D3DResourceBinding>&  resourceBindings,
            long                                    affectedStage,
            std::uint8_t&                           numSegments
        );

        void BuildAllSegmentsType2(
            const std::vector<D3DResourceBinding>&  resourceBindings,
            long                                    affectedStage,
            std::uint8_t&                           numSegments
        );

        void BuildAllSegmentsType3(
            const std::vector<D3DResourceBinding>&  resourceBindings,
            long                                    affectedStage,
            std::uint8_t&                           numSegments
        );

        void BuildSegment1(D3DResourceBindingIter it, UINT count);
        void BuildSegment2(D3DResourceBindingIter it, UINT count);
        void BuildSegment3(D3DResourceBindingIter it, UINT count);

        void StoreResourceUsage();

        void BindVSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);
        void BindHSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);
        void BindDSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);
        void BindGSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);
        void BindPSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);
        void BindCSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer);

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

        void BindVSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);
        void BindHSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);
        void BindDSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);
        void BindGSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);
        void BindPSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);
        void BindCSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer);

        #endif

        ID3D11ShaderResourceView* GetOrCreateTextureSRV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc);
        ID3D11UnorderedAccessView* GetOrCreateTextureUAV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc);

        ID3D11ShaderResourceView* GetOrCreateBufferSRV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc);
        ID3D11UnorderedAccessView* GetOrCreateBufferUAV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc);

        const std::int8_t* GetSegmentationHeapStart(std::uint32_t firstSet) const;

    private:

        /*
        Describes the segments within the raw buffer (per descriptor set).
        - Constant buffers       are limited to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT ( 14) ==> 4 bits
        - Samplers               are limited to D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT             ( 16) ==> 5 bits
        - Shader resource views  are limited to D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT      (128) ==> 8 bits
        - Unordered access views are limited to D3D11_1_UAV_SLOT_COUNT                            ( 64) ==> 7 bits
        */
        struct BufferSegmentation
        {
            std::uint32_t hasConstantBufferRanges           : 1;

            std::uint32_t hasVSResources                    : 1;
            std::uint32_t numVSConstantBufferSegments       : 4;
            std::uint32_t numVSConstantBufferRangeSegments  : 4;
            std::uint32_t numVSSamplerSegments              : 5;
            std::uint32_t numVSShaderResourceViewSegments   : 8;

            std::uint32_t hasHSResources                    : 1;
            std::uint32_t numHSConstantBufferSegments       : 4;
            std::uint32_t numHSConstantBufferRangeSegments  : 4;
            std::uint32_t numHSSamplerSegments              : 5;
            std::uint32_t numHSShaderResourceViewSegments   : 8;

            std::uint32_t hasDSResources                    : 1;
            std::uint32_t numDSConstantBufferSegments       : 4;
            std::uint32_t numDSConstantBufferRangeSegments  : 4;
            std::uint32_t numDSSamplerSegments              : 5;
            std::uint32_t numDSShaderResourceViewSegments   : 8;

            std::uint32_t hasGSResources                    : 1;
            std::uint32_t numGSConstantBufferSegments       : 4;
            std::uint32_t numGSConstantBufferRangeSegments  : 4;
            std::uint32_t numGSSamplerSegments              : 5;
            std::uint32_t numGSShaderResourceViewSegments   : 8;

            std::uint32_t hasCSResources                    : 1;
            std::uint32_t numPSUnorderedAccessViewSegments  : 7;
            std::uint32_t numPSConstantBufferSegments       : 4;
            std::uint32_t numPSConstantBufferRangeSegments  : 4;
            std::uint32_t numPSSamplerSegments              : 5;
            std::uint32_t numPSShaderResourceViewSegments   : 8;

            std::uint32_t hasPSResources                    : 1;
            std::uint32_t numCSUnorderedAccessViewSegments  : 7;
            std::uint32_t numCSConstantBufferSegments       : 4;
            std::uint32_t numCSConstantBufferRangeSegments  : 4;
            std::uint32_t numCSSamplerSegments              : 5;
            std::uint32_t numCSShaderResourceViewSegments   : 8;
        };

    private:

        BufferSegmentation                              segmentation_;

        std::size_t                                     stride_         = 0;
        std::uint16_t                                   bufferOffsetCS_ = 0;
        std::vector<std::int8_t>                        buffer_;

        std::vector<ComPtr<ID3D11ShaderResourceView>>   srvs_;
        std::vector<ComPtr<ID3D11UnorderedAccessView>>  uavs_;

};


} // /namespace LLGL


#endif



// ================================================================================
