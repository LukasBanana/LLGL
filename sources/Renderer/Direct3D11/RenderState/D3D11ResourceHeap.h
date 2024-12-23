/*
 * D3D11ResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RESOURCE_HEAP_H
#define LLGL_D3D11_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Container/ArrayView.h>
#include "D3D11BindingLocator.h"
#include "../../BindingIterator.h"
#include "../../SegmentedBuffer.h"
#include "../../DXCommon/DXManagedComPtrArray.h"
#include <vector>
#include <functional>
#include <initializer_list>
#include "../Direct3D11.h"


namespace LLGL
{


enum D3DResourceType : std::uint32_t;
class D3D11Texture;
class D3D11BufferWithRV;
class D3D11BindingTable;
struct ResourceHeapDescriptor;
struct TextureViewDescriptor;
struct BufferViewDescriptor;

/*
This class emulates the behavior of a descriptor heap like in D3D12,
by binding all shader resources within one bind call in the command buffer.
*/
class D3D11ResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        D3D11ResourceHeap(
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );

        // Writes the specified resource views to this resource heap and generates SRVs and UAVs as required.
        std::uint32_t WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews);

        void BindForGraphicsPipeline(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet);
        void BindForComputePipeline(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet);

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

        void BindForGraphicsPipeline1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet);
        void BindForComputePipeline1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet);

        #endif

    private:

        struct D3DResourceBinding;

        using AllocSegmentFunc = std::function<void(const D3DResourceBinding* first, UINT count)>;

        enum D3DShaderStage : std::uint32_t
        {
            D3DShaderStage_VS = 0,
            D3DShaderStage_HS,
            D3DShaderStage_DS,
            D3DShaderStage_GS,
            D3DShaderStage_PS,
            D3DShaderStage_CS,

            D3DShaderStage_Count,
        };

        /*
        Describes the segments within the raw buffer (per descriptor set).
        - Constant buffers       are limited to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT ( 14) ==> 4 bits
        - Samplers               are limited to D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT             ( 16) ==> 5 bits
        - Shader resource views  are limited to D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT      (128) ==> 8 bits
        - Unordered access views are limited to D3D11_1_UAV_SLOT_COUNT                            ( 64) ==> 7 bits
        */
        struct BufferSegmentation
        {
            std::uint32_t hasResourcesVS        : 1;
            std::uint32_t numCBVSegmentsVS      : 4;
            std::uint32_t numSRVSegmentsVS      : 8;
            std::uint32_t numSamplerSegmentsVS  : 5;

            std::uint32_t hasResourcesHS        : 1;
            std::uint32_t numCBVSegmentsHS      : 4;
            std::uint32_t numSRVSegmentsHS      : 8;
            std::uint32_t numSamplerSegmentsHS  : 5;

            std::uint32_t hasResourcesDS        : 1;
            std::uint32_t numCBVSegmentsDS      : 4;
            std::uint32_t numSRVSegmentsDS      : 8;
            std::uint32_t numSamplerSegmentsDS  : 5;

            std::uint32_t hasResourcesGS        : 1;
            std::uint32_t numCBVSegmentsGS      : 4;
            std::uint32_t numSRVSegmentsGS      : 8;
            std::uint32_t numSamplerSegmentsGS  : 5;

            std::uint32_t hasResourcesPS        : 1;
            std::uint32_t numCBVSegmentsPS      : 4;
            std::uint32_t numSRVSegmentsPS      : 8;
            std::uint32_t numUAVSegmentsPS      : 7;
            std::uint32_t numSamplerSegmentsPS  : 5;

            std::uint32_t hasResourcesCS        : 1;
            std::uint32_t numCBVSegmentsCS      : 4;
            std::uint32_t numSRVSegmentsCS      : 8;
            std::uint32_t numUAVSegmentsCS      : 7;
            std::uint32_t numSamplerSegmentsCS  : 5;
        };

        struct BindingSegmentLocation
        {
            static constexpr std::uint32_t invalidOffset = 0x00FFFFFF;

            struct Stage
            {
                inline Stage() :
                    segmentOffset   { BindingSegmentLocation::invalidOffset },
                    descriptorIndex { 0                                     }
                {
                }

                std::uint32_t segmentOffset     : 24; // Byte offset to the first segment within a segment set for the respective shader stage.
                std::uint32_t descriptorIndex   :  8; // Index of the descriptor the binding maps to (within each segment).
            }
            stages[D3DShaderStage_Count];
            D3DResourceType type;
        };

        struct SubresourceIndexContext
        {
            // Puts 'newIndex' into the specified storage and collects the old value.
            void Exchange(std::uint16_t& storage);

            std::size_t oldIndex = -1; // Index to subresource in SRV or UAV list.
            std::size_t newIndex = -1; // New index to override previous subresource in heap.
        };

        // D3D resource binding slot with index to the input binding list
        struct D3DResourceBinding
        {
            UINT        slot;
            long        stages; // bitwise OR combination of StageFlags entries
            std::size_t index;  // Index to the input bindings list
        };

        // Helper structure for SRV and UAV output.
        struct D3DSubresourceLocator
        {
            std::size_t             index   = -1;       // Index into the subresource containers (subresourceSRVs_ and subresourceUAVs_).
            D3D11BindingLocator*    locator = nullptr;  // Binding table locator.
            D3D11SubresourceRange   range   = {};       // Binding table subresource range.
        };

    private:

        void AllocStageSegments(BindingDescriptorIterator& bindingIter, long stage);

        void AllocConstantBufferSegments(BindingDescriptorIterator& bindingIter, long stage);
        void AllocShaderResourceViewSegments(BindingDescriptorIterator& bindingIter, long stage);
        void AllocUnorderedAccessViewSegments(BindingDescriptorIterator& bindingIter, long stage, long affectedStages);
        void AllocSamplerSegments(BindingDescriptorIterator& bindingIter, long stage);

        void Alloc1PartSegment(
            D3DShaderStage              stage,
            D3DResourceType             type,
            const D3DResourceBinding*   first,
            UINT                        count,
            std::size_t                 payload0Stride
        );

        void Alloc2PartSegment(
            D3DShaderStage              stage,
            D3DResourceType             type,
            const D3DResourceBinding*   first,
            UINT                        count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride,
            int                         payload1Initial
        );

        void Alloc3PartSegment(
            D3DShaderStage              stage,
            D3DResourceType             type,
            const D3DResourceBinding*   first,
            UINT                        count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride,
            std::size_t                 payload2Stride,
            int                         payload2Initial
        );

        void WriteBindingMappings(D3DShaderStage stage, D3DResourceType type, const D3DResourceBinding* first, UINT count);
        void CacheResourceUsage();

        const char* BindVSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindHSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindDSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindGSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindPSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindCSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr);

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

        const char* BindVSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindHSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindDSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindGSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindPSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);
        const char* BindCSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr);

        #endif

        void WriteResourceViewCBV(
            const ResourceViewDescriptor&   desc,
            char*                           heapPtr,
            std::uint32_t                   index
        );

        void WriteResourceViewSRV(
            ID3D11ShaderResourceView*       srv,
            D3D11BindingLocator*            locator,
            const D3D11SubresourceRange&    range,
            char*                           heapPtr,
            std::uint32_t                   index,
            SubresourceIndexContext&        subresourceContext
        );

        void WriteResourceViewUAV(
            ID3D11UnorderedAccessView*      uav,
            D3D11BindingLocator*            locator,
            const D3D11SubresourceRange&    range,
            char*                           heapPtr,
            std::uint32_t                   index,
            UINT                            initialCount,
            SubresourceIndexContext&        subresourceContext
        );

        void WriteResourceViewSampler(
            const ResourceViewDescriptor&   desc,
            char*                           heapPtr,
            std::uint32_t                   index
        );

        ID3D11ShaderResourceView* GetOrCreateSRV(const ResourceViewDescriptor& desc, D3DSubresourceLocator& outLocator);
        ID3D11UnorderedAccessView* GetOrCreateUAV(const ResourceViewDescriptor& desc, D3DSubresourceLocator& outLocator);

        ID3D11ShaderResourceView* GetOrCreateTextureSRV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc, D3DSubresourceLocator& outLocator);
        ID3D11UnorderedAccessView* GetOrCreateTextureUAV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc, D3DSubresourceLocator& outLocator);

        ID3D11ShaderResourceView* GetOrCreateBufferSRV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc, D3DSubresourceLocator& outLocator);
        ID3D11UnorderedAccessView* GetOrCreateBufferUAV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc, D3DSubresourceLocator& outLocator);

    private:

        static std::vector<D3DResourceBinding> FilterAndSortD3DBindingSlots(
            BindingDescriptorIterator&                  bindingIter,
            const std::initializer_list<ResourceType>&  resourceTypes,
            long                                        resourceBindFlags,
            long                                        affectedStage
        );

        static std::uint32_t ConsolidateSegments(
            const ArrayView<D3DResourceBinding>&    bindingSlots,
            const AllocSegmentFunc&                 allocSegmentFunc
        );

        static D3DShaderStage StageFlagsToD3DShaderStage(long stage);

        template <typename T>
        static void GarbageCollectSubresource(DXManagedComPtrArray<T>& subresources, SubresourceIndexContext& subresourceContext);

    private:

        SmallVector<BindingSegmentLocation>             bindingMap_;            // Maps a binding index to a descriptor location.
        BufferSegmentation                              segmentation_   = {};

        SegmentedBuffer                                 heap_;
        std::uint32_t                                   heapOffsetCS_   = 0;

        DXManagedComPtrArray<ID3D11ShaderResourceView>  subresourceSRVs_;
        DXManagedComPtrArray<ID3D11UnorderedAccessView> subresourceUAVs_;

};


} // /namespace LLGL


#endif



// ================================================================================
