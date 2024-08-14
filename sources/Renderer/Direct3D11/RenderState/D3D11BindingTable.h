/*
 * D3D11BindingTable.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BINDING_TABLE_H
#define LLGL_D3D11_BINDING_TABLE_H


#include "D3D11BindingLocator.h"
#include "../../DXCommon/ComPtr.h"
#include "../../../Core/SparseForwardIterator.h"
#include <LLGL/ResourceFlags.h>
#include <d3d11.h>
#include <cstdint>
#include <cstddef>
#include <cstring>


namespace LLGL
{


/*
Class to manage the binding table of a D3D11 device context.
Automatically evicts buffer and texture resources from input bindings when used as output and vice-versa.

This does not handle constant buffers nor sampler states since those resources cannot be used for output bindings;
D3D11_BIND_CONSTANT_BUFFER must not be combined with any other binding flag and sampler states are always read-only.

This class also does not evict RTVs and the DSV, because those will always be unbound at the end of a render pass.
Moreover, depth textures can potentially be read from a shader while also bound as DSV during a render pass as long as depth-writing is disabled.
*/
class D3D11BindingTable
{

    public:

        D3D11BindingTable(const ComPtr<ID3D11DeviceContext>& context);

        void SetVertexBuffer(
            UINT                    startSlot,
            ID3D11Buffer*           buffer,
            UINT                    stride,
            UINT                    offset,
            D3D11BindingLocator*    locator
        );

        void SetVertexBuffers(
            UINT                            startSlot,
            UINT                            count,
            ID3D11Buffer* const*            buffers,
            const UINT*                     strides,
            const UINT*                     offsets,
            D3D11BindingLocator* const *    locators
        );

        void SetIndexBuffer(
            ID3D11Buffer*           buffer,
            DXGI_FORMAT             format,
            UINT                    offset,
            D3D11BindingLocator*    locator
        );

        void SetStreamOutputBuffers(
            UINT                            count,
            ID3D11Buffer* const*            buffers,
            const UINT*                     offsets,
            D3D11BindingLocator* const *    locators
        );

        void SetShaderResourceViews(
            UINT                                startSlot,
            UINT                                count,
            ID3D11ShaderResourceView* const *   views,
            D3D11BindingLocator* const *        locators,
            const D3D11SubresourceRange*        subresourceRanges,
            long                                stageFlags
        );

        void SetUnorderedAccessViews(
            UINT                                startSlot,
            UINT                                count,
            ID3D11UnorderedAccessView* const *  views,
            const UINT*                         initialCounts,
            D3D11BindingLocator* const *        locators,
            const D3D11SubresourceRange*        subresourceRanges,
            long                                stageFlags
        );

        void SetRenderTargets(
            UINT                            count,
            ID3D11RenderTargetView* const * renderTargetViews,
            ID3D11DepthStencilView*         depthStencilView,
            D3D11BindingLocator* const *    renderTargetLocators,
            const D3D11SubresourceRange*    renderTargetSubresourceRanges,
            D3D11BindingLocator*            depthStencilLocators
        );

        // Clears the binding table state but doesn't perform any operations on the D3D device context.
        void ClearState();

        // Binds all pending output merger UAVs to the device context if they have previosuly changed.
        void FlushOutputMergerUAVs();

        // Notifies the binding table that a resource, represented by the specified binding locator, is about to be released.
        void NotifyResourceRelease(D3D11BindingLocator* locator);

    private:

        using D3D11BindingLocatorIterator = SparseForwardIterator<D3D11BindingLocator*>;

        template <UINT Size>
        struct ResourceLocatorContainer
        {
            constexpr UINT size() const noexcept
            {
                return Size;
            }

            void clear()
            {
                std::memset(locators, 0, sizeof(locators));
            }

            D3D11BindingLocatorIterator begin() noexcept
            {
                return D3D11BindingLocatorIterator{ locators, locators + Size };
            }

            D3D11BindingLocatorIterator end() noexcept
            {
                return D3D11BindingLocatorIterator{ locators + Size };
            }

            D3D11BindingLocator* locators[Size] = {};
        };

        template <UINT Size>
        struct SubresourceLocatorContainer
        {
            constexpr UINT size() const noexcept
            {
                return Size;
            }

            void clear()
            {
                std::memset(locators, 0, sizeof(locators));
                std::memset(subresourceRanges, 0, sizeof(subresourceRanges));
            }

            D3D11BindingLocatorIterator begin() noexcept
            {
                return D3D11BindingLocatorIterator{ locators, locators + Size };
            }

            D3D11BindingLocatorIterator end() noexcept
            {
                return D3D11BindingLocatorIterator{ locators + Size };
            }

            D3D11BindingLocator*    locators[Size]          = {};
            D3D11SubresourceRange   subresourceRanges[Size] = {}; // Ranges to determine overlaps between SRV and UAV subresources of the same parent resource
        };

    private:

        void InsertInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot, D3D11BindingLocator* locator);
        bool RemoveSubresourceInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot);
        bool RemoveWholeResourceInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot);

        void InsertOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot, D3D11BindingLocator* locator);
        bool RemoveSubresourceOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot);
        void RemoveWholeResourceOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot);

        void PutVertexBuffer(D3D11BindingLocator* locator, UINT slot);
        void PutIndexBuffer(D3D11BindingLocator* locator);

        void PutShaderResourceViewVS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutShaderResourceViewHS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutShaderResourceViewDS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutShaderResourceViewGS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutShaderResourceViewPS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutShaderResourceViewCS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);

        void PutStreamOutputBuffer(D3D11BindingLocator* locator, UINT slot);
        void PutUnorderedAccessViewPS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);
        void PutUnorderedAccessViewCS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot);

        void EvictAllBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange = nullptr);

        void EvictAllOutputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange = nullptr);
        void EvictSingleOutputBinding(D3D11BindingLocator* locator);
        void EvictSingleSubresourceOutputBinding(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange);
        void EvictMultipleOutputBindings(D3D11BindingLocator* locator);
        void EvictMultipleSubresourceOutputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange);

        void EvictAllInputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange = nullptr);
        void EvictSingleInputBinding(D3D11BindingLocator* locator);
        void EvictSingleSubresourceInputBinding(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange);
        void EvictMultipleInputBindings(D3D11BindingLocator* locator);
        void EvictMultipleSubresourceInputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange);

        void EvictAllVertexBuffers();
        void EvictAllStreamOutputTargets();
        void EvictSingleUnorderedAccessViewPS(UINT slot);
        void EvictSingleUnorderedAccessViewCS(UINT slot);

        void BindCachedOutputMergerUAVs();

        template <typename TContainer>
        void ClearBindingLocators(TContainer& container)
        {
            for (D3D11BindingLocator* locator : container)
            {
                locator->ClearInput();
                locator->ClearOutput();
            }
            container.clear();
        }

        template <typename TContainer>
        bool HasLocatorAt(const TContainer& container, UINT slot, const D3D11BindingLocator* locator) const
        {
            return (slot < container.size() && container.locators[slot] == locator);
        }

        template <typename TContainer>
        bool HasLocatorAndRangesOverlapAt(const TContainer& container, UINT slot, const D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange) const
        {
            return (HasLocatorAt(container, slot, locator) && D3D11SubresourceRange::Overlap(container.subresourceRanges[slot], subresourceRange));
        }

    private:

        ComPtr<ID3D11DeviceContext>                                                 context_;

        ResourceLocatorContainer<D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT>         vb_;    // D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT (32)
        ResourceLocatorContainer<1>                                                 ib_;

        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvVS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)
        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvHS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)
        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvDS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)
        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvGS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)
        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvPS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)
        SubresourceLocatorContainer<D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>   srvCS_; // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT (128)

        ResourceLocatorContainer<D3D11_SO_BUFFER_SLOT_COUNT>                        so_;    // D3D11_SO_BUFFER_SLOT_COUNT (4)
        SubresourceLocatorContainer<D3D11_1_UAV_SLOT_COUNT>                         uavPS_; // D3D11_PS_CS_UAV_REGISTER_COUNT (8), D3D11_1_UAV_SLOT_COUNT (64)
        SubresourceLocatorContainer<D3D11_1_UAV_SLOT_COUNT>                         uavCS_; // D3D11_PS_CS_UAV_REGISTER_COUNT (8), D3D11_1_UAV_SLOT_COUNT (64)

        ID3D11UnorderedAccessView*                                                  uavOMRefs_[D3D11_1_UAV_SLOT_COUNT]          = {};
        UINT                                                                        uavOMInitialCounts_[D3D11_1_UAV_SLOT_COUNT] = {};

        UINT                                                                        vbCount_                                    = 0;
        UINT                                                                        soCount_                                    = 0;
        UINT                                                                        rtvCount_                                   = 0;

        UINT                                                                        omUAVStartSlot_ : 15;
        UINT                                                                        omNumUAVs_      : 16; // Number of UAVs for the output-merger stage
        UINT                                                                        omUAVDirtyBit_  : 1;

};


} // /namespace LLGL


#endif



// ================================================================================
