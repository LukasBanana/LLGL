/*
 * D3D11BindingTable.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11BindingTable.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/ShaderFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


D3D11BindingTable::D3D11BindingTable(const ComPtr<ID3D11DeviceContext>& context) :
    context_        { context },
    omUAVStartSlot_ { ~0u     },
    omNumUAVs_      { 0       },
    omUAVDirtyBit_  { 0       }
{
}

void D3D11BindingTable::SetVertexBuffer(
    UINT                    startSlot,
    ID3D11Buffer*           buffer,
    UINT                    stride,
    UINT                    offset,
    D3D11BindingLocator*    locator)
{
    if (locator != nullptr && locator->type == D3D11BindingLocator::D3DLocator_RWBuffer)
    {
        EvictAllOutputBindings(locator);
        PutVertexBuffer(locator, startSlot);
    }
    context_->IASetVertexBuffers(startSlot, 1, &buffer, &stride, &offset);
    vbCount_ = 1;
}

void D3D11BindingTable::SetVertexBuffers(
    UINT                            startSlot,
    UINT                            count,
    ID3D11Buffer* const*            buffers,
    const UINT*                     strides,
    const UINT*                     offsets,
    D3D11BindingLocator* const *    locators)
{
    if (locators != nullptr)
    {
        for_range(i, count)
        {
            LLGL_ASSERT_PTR(locators[i]);
            if (locators[i]->type == D3D11BindingLocator::D3DLocator_RWBuffer)
            {
                EvictAllOutputBindings(locators[i]);
                PutVertexBuffer(locators[i], startSlot + i);
            }
        }
    }
    context_->IASetVertexBuffers(startSlot, count, buffers, strides, offsets);
    vbCount_ = count;
}

void D3D11BindingTable::SetIndexBuffer(
    ID3D11Buffer*           buffer,
    DXGI_FORMAT             format,
    UINT                    offset,
    D3D11BindingLocator*    locator)
{
    if (locator != nullptr && locator->type == D3D11BindingLocator::D3DLocator_RWBuffer)
    {
        EvictAllOutputBindings(locator);
        PutIndexBuffer(locator);
    }
    context_->IASetIndexBuffer(buffer, format, offset);
}

void D3D11BindingTable::SetStreamOutputBuffers(
    UINT                            count,
    ID3D11Buffer* const*            buffers,
    const UINT*                     offsets,
    D3D11BindingLocator* const *    locators)
{
    if (locators != nullptr)
    {
        for_range(i, count)
            PutStreamOutputBuffer(locators[i], i);
        for_subrange(i, count, soCount_)
            RemoveWholeResourceOutput(so_.locators, D3D11BindingLocator::D3DOutput_SO, i);
        soCount_ = count;
    }
    else if (soCount_ > 0)
    {
        for_range(i, soCount_)
            RemoveWholeResourceOutput(so_.locators, D3D11BindingLocator::D3DOutput_SO, i);
        soCount_ = 0;
    }
    context_->SOSetTargets(count, buffers, offsets);
}

void D3D11BindingTable::SetShaderResourceViews(
    UINT                                startSlot,
    UINT                                count,
    ID3D11ShaderResourceView* const *   views,
    D3D11BindingLocator* const *        locators,
    const D3D11SubresourceRange*        subresourceRanges,
    long                                stageFlags)
{
    if (locators != nullptr)
    {
        if (subresourceRanges != nullptr)
        {
            for_range(i, count)
            {
                LLGL_ASSERT_PTR(locators[i]);
                if (locators[i]->type != D3D11BindingLocator::D3DLocator_ReadOnly)
                {
                    EvictAllOutputBindings(locators[i], &subresourceRanges[i]);
                    if (LLGL_VS_STAGE(stageFlags)) { PutShaderResourceViewVS(locators[i], subresourceRanges[i], startSlot + i); }
                    if (LLGL_HS_STAGE(stageFlags)) { PutShaderResourceViewHS(locators[i], subresourceRanges[i], startSlot + i); }
                    if (LLGL_DS_STAGE(stageFlags)) { PutShaderResourceViewDS(locators[i], subresourceRanges[i], startSlot + i); }
                    if (LLGL_GS_STAGE(stageFlags)) { PutShaderResourceViewGS(locators[i], subresourceRanges[i], startSlot + i); }
                    if (LLGL_PS_STAGE(stageFlags)) { PutShaderResourceViewPS(locators[i], subresourceRanges[i], startSlot + i); }
                    if (LLGL_CS_STAGE(stageFlags)) { PutShaderResourceViewCS(locators[i], subresourceRanges[i], startSlot + i); }
                }
            }
        }
        else
        {
            const D3D11SubresourceRange fullRange{ 0u, ~0u };
            for_range(i, count)
            {
                LLGL_ASSERT_PTR(locators[i]);
                if (locators[i]->type != D3D11BindingLocator::D3DLocator_ReadOnly)
                {
                    EvictAllOutputBindings(locators[i]);
                    if (LLGL_VS_STAGE(stageFlags)) { PutShaderResourceViewVS(locators[i], fullRange, startSlot + i); }
                    if (LLGL_HS_STAGE(stageFlags)) { PutShaderResourceViewHS(locators[i], fullRange, startSlot + i); }
                    if (LLGL_DS_STAGE(stageFlags)) { PutShaderResourceViewDS(locators[i], fullRange, startSlot + i); }
                    if (LLGL_GS_STAGE(stageFlags)) { PutShaderResourceViewGS(locators[i], fullRange, startSlot + i); }
                    if (LLGL_PS_STAGE(stageFlags)) { PutShaderResourceViewPS(locators[i], fullRange, startSlot + i); }
                    if (LLGL_CS_STAGE(stageFlags)) { PutShaderResourceViewCS(locators[i], fullRange, startSlot + i); }
                }
            }
        }
    }

    if (LLGL_VS_STAGE(stageFlags)) { context_->VSSetShaderResources(startSlot, count, views); }
    if (LLGL_HS_STAGE(stageFlags)) { context_->HSSetShaderResources(startSlot, count, views); }
    if (LLGL_DS_STAGE(stageFlags)) { context_->DSSetShaderResources(startSlot, count, views); }
    if (LLGL_GS_STAGE(stageFlags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (LLGL_PS_STAGE(stageFlags)) { context_->PSSetShaderResources(startSlot, count, views); }
    if (LLGL_CS_STAGE(stageFlags)) { context_->CSSetShaderResources(startSlot, count, views); }
}

void D3D11BindingTable::SetUnorderedAccessViews(
    UINT                                startSlot,
    UINT                                count,
    ID3D11UnorderedAccessView* const *  views,
    const UINT*                         initialCounts,
    D3D11BindingLocator* const *        locators,
    const D3D11SubresourceRange*        subresourceRanges,
    long                                stageFlags)
{
    if (locators != nullptr)
    {
        if (subresourceRanges != nullptr)
        {
            for_range(i, count)
            {
                LLGL_ASSERT_PTR(locators[i]);
                if (locators[i]->type != D3D11BindingLocator::D3DLocator_ReadOnly)
                {
                    EvictAllBindings(locators[i], &subresourceRanges[i]);
                    if (LLGL_GRAPHICS_STAGE(stageFlags)) { PutUnorderedAccessViewPS(locators[i], subresourceRanges[i], startSlot + i); }
                    else if (LLGL_CS_STAGE(stageFlags))  { PutUnorderedAccessViewCS(locators[i], subresourceRanges[i], startSlot + i); }
                }
            }
        }
        else
        {
            const D3D11SubresourceRange fullRange{ 0u, ~0u };
            for_range(i, count)
            {
                LLGL_ASSERT_PTR(locators[i]);
                if (locators[i]->type != D3D11BindingLocator::D3DLocator_ReadOnly)
                {
                    EvictAllBindings(locators[i]);
                    if (LLGL_GRAPHICS_STAGE(stageFlags)) { PutUnorderedAccessViewPS(locators[i], fullRange, startSlot + i); }
                    else if (LLGL_CS_STAGE(stageFlags))  { PutUnorderedAccessViewCS(locators[i], fullRange, startSlot + i); }
                }
            }
        }
    }

    if (LLGL_GRAPHICS_STAGE(stageFlags))
    {
        /* Cache UAVs for graphics pipeline and flush before next draw call since UAVs (and RTVs) must be set at once */
        for_range(i, count)
        {
            uavOMRefs_[startSlot + i] = views[i];
            uavOMInitialCounts_[startSlot + i] = initialCounts[i];
        }
        omUAVStartSlot_ = std::min<UINT>(omUAVStartSlot_, startSlot);
        omNumUAVs_      = std::max<UINT>(omUAVStartSlot_ + omNumUAVs_, startSlot + count) - omUAVStartSlot_;
        omUAVDirtyBit_  = 1;
    }
    else if (LLGL_CS_STAGE(stageFlags))
    {
        /* Set UAVs for compute shader stage */
        context_->CSSetUnorderedAccessViews(startSlot, count, views, initialCounts);
    }
}

void D3D11BindingTable::SetRenderTargets(
    UINT                            count,
    ID3D11RenderTargetView* const * renderTargetViews,
    ID3D11DepthStencilView*         depthStencilView,
    D3D11BindingLocator* const *    renderTargetLocators,
    const D3D11SubresourceRange*    renderTargetSubresourceRanges,
    D3D11BindingLocator*            depthStencilLocators)
{
    /* Evict all bindings for RTVs */
    if (renderTargetLocators != nullptr)
    {
        LLGL_ASSERT_PTR(renderTargetSubresourceRanges);
        for_range(slot, count)
        {
            if (D3D11BindingLocator* locator = renderTargetLocators[slot])
                EvictAllBindings(locator, renderTargetSubresourceRanges);
        }
        rtvCount_ = count;
    }
    else
        rtvCount_ = 0;

    /* Evict all bindings for DSV */
    if (depthStencilLocators != nullptr)
        EvictAllBindings(depthStencilLocators);

    /* Bind new RTVs and DSV */
    context_->OMSetRenderTargets(count, renderTargetViews, depthStencilView);
}

void D3D11BindingTable::ClearState()
{
    /* Clear binding locators from all tables */
    ClearBindingLocators(vb_);
    ClearBindingLocators(ib_);
    ClearBindingLocators(srvVS_);
    ClearBindingLocators(srvHS_);
    ClearBindingLocators(srvDS_);
    ClearBindingLocators(srvGS_);
    ClearBindingLocators(srvPS_);
    ClearBindingLocators(srvCS_);
    ClearBindingLocators(so_);
    ClearBindingLocators(uavPS_);
    ClearBindingLocators(uavCS_);

    /* Reset all resource counters */
    vbCount_        = 0;
    soCount_        = 0;
    rtvCount_       = 0;
    omUAVStartSlot_ = ~0u;
    omNumUAVs_      = 0;
    omUAVDirtyBit_  = 0;
}

void D3D11BindingTable::FlushOutputMergerUAVs()
{
    if (omUAVDirtyBit_ != 0)
    {
        BindCachedOutputMergerUAVs();
        omUAVDirtyBit_ = 0;
    }
}

void D3D11BindingTable::NotifyResourceRelease(D3D11BindingLocator* locator)
{
    /* When a resource is about to be released, evict all of its bindings from this binding table */
    EvictAllBindings(locator);
}


/*
 * ======= Private: =======
 */

static void* const g_nullArray[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};

template <typename T>
static T* const* GetNullPointerArray()
{
    return reinterpret_cast<T* const*>(g_nullArray);
}

void D3D11BindingTable::InsertInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot, D3D11BindingLocator* locator)
{
    if (container[slot] != locator)
    {
        /* Try to unset previous locator */
        if (container[slot] != nullptr)
            container[slot]->TryRemoveInputAt(input, slot);

        /* Put locator into table */
        container[slot] = locator;

        /* Set new locator */
        if (locator != nullptr)
            locator->InsertInputAt(input, slot);
    }
}

bool D3D11BindingTable::RemoveSubresourceInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot)
{
    /* Try to unset previous locator */
    bool clearedInputBitmask = false;
    if (container[slot] != nullptr)
    {
        clearedInputBitmask = container[slot]->TryRemoveInputAt(input, slot);
        container[slot] = nullptr;
    }
    return clearedInputBitmask;
}

bool D3D11BindingTable::RemoveWholeResourceInput(D3D11BindingLocator** container, D3D11BindingLocator::D3DInputs input, UINT slot)
{
    /* Remove input of type that can only be bound for the whole resource (such as vertex-buffer or index-buffer) */
    bool clearedInputBitmask = false;
    if (container[slot] != nullptr)
    {
        clearedInputBitmask = container[slot]->RemoveInput(input);
        container[slot] = nullptr;
    }
    return clearedInputBitmask;
}

void D3D11BindingTable::InsertOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot, D3D11BindingLocator* locator)
{
    if (container[slot] != locator)
    {
        /* Try to unset previous locator */
        if (container[slot] != nullptr)
            container[slot]->TryRemoveOutputAt(output, slot);

        /* Put locator into table */
        container[slot] = locator;

        /* Set new locator */
        if (locator != nullptr)
            locator->InsertOutputAt(output, slot);
    }
}

bool D3D11BindingTable::RemoveSubresourceOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot)
{
    /* Try to unset previous locator */
    bool clearedOutputBitmask = false;
    if (container[slot] != nullptr)
    {
        clearedOutputBitmask = container[slot]->TryRemoveOutputAt(output, slot);
        container[slot] = nullptr;
    }
    return clearedOutputBitmask;
}

void D3D11BindingTable::RemoveWholeResourceOutput(D3D11BindingLocator** container, D3D11BindingLocator::D3DOutputs output, UINT slot)
{
    /* Remove output of type that can only be bound for the whole resource (such as stream-output) */
    if (container[slot] != nullptr)
    {
        container[slot]->RemoveOutput(output);
        container[slot] = nullptr;
    }
}

void D3D11BindingTable::PutVertexBuffer(D3D11BindingLocator* locator, UINT slot)
{
    InsertInput(vb_.locators, D3D11BindingLocator::D3DInput_VB, slot, locator);
}

void D3D11BindingTable::PutIndexBuffer(D3D11BindingLocator* locator)
{
    InsertInput(ib_.locators, D3D11BindingLocator::D3DInput_IB, 0, locator);
}

void D3D11BindingTable::PutShaderResourceViewVS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvVS_.locators, D3D11BindingLocator::D3DInput_SRV_VS, slot, locator);
    srvVS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutShaderResourceViewHS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvHS_.locators, D3D11BindingLocator::D3DInput_SRV_HS, slot, locator);
    srvHS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutShaderResourceViewDS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvDS_.locators, D3D11BindingLocator::D3DInput_SRV_DS, slot, locator);
    srvDS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutShaderResourceViewGS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvGS_.locators, D3D11BindingLocator::D3DInput_SRV_GS, slot, locator);
    srvGS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutShaderResourceViewPS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvPS_.locators, D3D11BindingLocator::D3DInput_SRV_PS, slot, locator);
    srvPS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutShaderResourceViewCS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertInput(srvCS_.locators, D3D11BindingLocator::D3DInput_SRV_CS, slot, locator);
    srvCS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutStreamOutputBuffer(D3D11BindingLocator* locator, UINT slot)
{
    InsertOutput(so_.locators, D3D11BindingLocator::D3DOutput_SO, slot, locator);
}

void D3D11BindingTable::PutUnorderedAccessViewPS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertOutput(uavPS_.locators, D3D11BindingLocator::D3DOutput_UAV_PS, slot, locator);
    uavPS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::PutUnorderedAccessViewCS(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange, UINT slot)
{
    InsertOutput(uavCS_.locators, D3D11BindingLocator::D3DOutput_UAV_CS, slot, locator);
    uavCS_.subresourceRanges[slot] = subresourceRange;
}

void D3D11BindingTable::EvictAllBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange)
{
    EvictAllInputBindings(locator, subresourceRange);
    EvictAllOutputBindings(locator, subresourceRange);
}

void D3D11BindingTable::EvictAllOutputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange)
{
    if (locator->outBitmask != 0)
    {
        if (subresourceRange != nullptr)
        {
            if (locator->HasSingleOutputBinding())
                EvictSingleSubresourceOutputBinding(locator, *subresourceRange);
            else
                EvictMultipleSubresourceOutputBindings(locator, *subresourceRange);
        }
        else
        {
            if (locator->HasSingleOutputBinding())
                EvictSingleOutputBinding(locator);
            else
                EvictMultipleOutputBindings(locator);
        }
    }
}

void D3D11BindingTable::EvictSingleOutputBinding(D3D11BindingLocator* locator)
{
    const UINT slot = locator->outRangeBegin;

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_SO)) != 0 && HasLocatorAt(so_, slot, locator))
    {
        /* SO targets must be set/unset all at once */
        EvictAllStreamOutputTargets();
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_PS)) != 0 && HasLocatorAt(uavPS_, slot, locator))
    {
        EvictSingleUnorderedAccessViewPS(slot);
        RemoveWholeResourceOutput(uavPS_.locators, D3D11BindingLocator::D3DOutput_UAV_PS, slot);
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_CS)) != 0 && HasLocatorAt(uavCS_, slot, locator))
    {
        EvictSingleUnorderedAccessViewCS(slot);
        RemoveWholeResourceOutput(uavCS_.locators, D3D11BindingLocator::D3DOutput_UAV_CS, slot);
    }
}

void D3D11BindingTable::EvictSingleSubresourceOutputBinding(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange)
{
    const UINT slot = locator->outRangeBegin;

    /* Stream-output buffers cannot have subresource views, so always unbind the entire resource */
    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_SO)) != 0 && HasLocatorAt(so_, slot, locator))
    {
        /* SO targets must be set/unset all at once */
        EvictAllStreamOutputTargets();
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_PS)) != 0 && HasLocatorAndRangesOverlapAt(uavPS_, slot, locator, subresourceRange))
    {
        EvictSingleUnorderedAccessViewPS(slot);
        if (RemoveSubresourceOutput(uavPS_.locators, D3D11BindingLocator::D3DOutput_UAV_PS, slot))
            return;
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_CS)) != 0 && HasLocatorAndRangesOverlapAt(uavCS_, slot, locator, subresourceRange))
    {
        EvictSingleUnorderedAccessViewCS(slot);
        if (RemoveSubresourceOutput(uavCS_.locators, D3D11BindingLocator::D3DOutput_UAV_CS, slot))
            return;
    }
}

void D3D11BindingTable::EvictMultipleOutputBindings(D3D11BindingLocator* locator)
{
    /* Stream-output buffers cannot have subresource views, so always unbind the entire resource */
    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_SO)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, so_.size()))
        {
            if (so_.locators[slot] == locator)
            {
                /* SO targets must be set/unset all at once */
                EvictAllStreamOutputTargets();
                break;
            }
        }
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_PS)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, uavPS_.size()))
        {
            if (uavPS_.locators[slot] == locator)
            {
                EvictSingleUnorderedAccessViewPS(slot);
                uavPS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveOutput(D3D11BindingLocator::D3DOutput_UAV_PS))
            return;
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_CS)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, uavCS_.size()))
        {
            if (uavCS_.locators[slot] == locator)
            {
                EvictSingleUnorderedAccessViewCS(slot);
                uavCS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveOutput(D3D11BindingLocator::D3DOutput_UAV_CS))
            return;
    }
}

void D3D11BindingTable::EvictMultipleSubresourceOutputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange)
{
    /* Stream-output buffers cannot have subresource views, so always unbind the entire resource */
    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_SO)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, so_.size()))
        {
            if (so_.locators[slot] == locator)
            {
                /* SO targets must be set/unset all at once */
                EvictAllStreamOutputTargets();
                break;
            }
        }
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_PS)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, uavPS_.size()))
        {
            if (uavPS_.locators[slot] == locator && D3D11SubresourceRange::Overlap(uavPS_.subresourceRanges[slot], subresourceRange))
            {
                EvictSingleUnorderedAccessViewPS(slot);
                uavPS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveOutput(D3D11BindingLocator::D3DOutput_UAV_PS))
            return;
    }

    if ((locator->outBitmask & (1u << D3D11BindingLocator::D3DOutput_UAV_CS)) != 0)
    {
        for_subrange(slot, locator->outRangeBegin, std::min<UINT>(locator->outRangeEnd, uavCS_.size()))
        {
            if (uavCS_.locators[slot] == locator && D3D11SubresourceRange::Overlap(uavCS_.subresourceRanges[slot], subresourceRange))
            {
                EvictSingleUnorderedAccessViewCS(slot);
                uavCS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveOutput(D3D11BindingLocator::D3DOutput_UAV_CS))
            return;
    }
}

void D3D11BindingTable::EvictSingleInputBinding(D3D11BindingLocator* locator)
{
    const UINT slot = locator->inRangeBegin;
    ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_VB)) != 0 && HasLocatorAt(vb_, slot, locator))
    {
        /* D3D11 allows to bind vertex buffer slots independently, but LLGL always sets/unsets vertex buffers all at once, so evict all at once */
        EvictAllVertexBuffers();
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_IB)) != 0 && HasLocatorAt(ib_, slot, locator))
    {
        context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
        if (RemoveWholeResourceInput(ib_.locators, D3D11BindingLocator::D3DInput_IB, 0))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_VS)) != 0 && HasLocatorAt(srvVS_, slot, locator))
    {
        context_->VSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvVS_.locators, D3D11BindingLocator::D3DInput_SRV_VS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_HS)) != 0 && HasLocatorAt(srvHS_, slot, locator))
    {
        context_->HSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvHS_.locators, D3D11BindingLocator::D3DInput_SRV_HS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_DS)) != 0 && HasLocatorAt(srvDS_, slot, locator))
    {
        context_->DSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvDS_.locators, D3D11BindingLocator::D3DInput_SRV_DS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_GS)) != 0 && HasLocatorAt(srvGS_, slot, locator))
    {
        context_->GSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvGS_.locators, D3D11BindingLocator::D3DInput_SRV_GS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_PS)) != 0 && HasLocatorAt(srvPS_, slot, locator))
    {
        context_->PSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvPS_.locators, D3D11BindingLocator::D3DInput_SRV_PS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_CS)) != 0 && HasLocatorAt(srvCS_, slot, locator))
    {
        context_->CSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvCS_.locators, D3D11BindingLocator::D3DInput_SRV_CS, slot))
            return;
    }
}

void D3D11BindingTable::EvictSingleSubresourceInputBinding(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange)
{
    const UINT slot = locator->inRangeBegin;
    ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_VB)) != 0 && HasLocatorAt(vb_, slot, locator))
    {
        /* D3D11 allows to bind vertex buffer slots independently, but LLGL always sets/unsets vertex buffers all at once, so evict all at once */
        EvictAllVertexBuffers();
        if (locator->inBitmask == 0)
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_IB)) != 0 && HasLocatorAt(ib_, slot, locator))
    {
        context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
        if (RemoveWholeResourceInput(ib_.locators, D3D11BindingLocator::D3DInput_IB, 0))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_VS)) != 0 && HasLocatorAndRangesOverlapAt(srvVS_, slot, locator, subresourceRange))
    {
        context_->VSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvVS_.locators, D3D11BindingLocator::D3DInput_SRV_VS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_HS)) != 0 && HasLocatorAndRangesOverlapAt(srvHS_, slot, locator, subresourceRange))
    {
        LLGL_ASSERT(slot < srvHS_.size());
        context_->HSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvHS_.locators, D3D11BindingLocator::D3DInput_SRV_HS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_DS)) != 0 && HasLocatorAndRangesOverlapAt(srvDS_, slot, locator, subresourceRange))
    {
        LLGL_ASSERT(slot < srvDS_.size());
        context_->DSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvDS_.locators, D3D11BindingLocator::D3DInput_SRV_DS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_GS)) != 0 && HasLocatorAndRangesOverlapAt(srvGS_, slot, locator, subresourceRange))
    {
        LLGL_ASSERT(slot < srvGS_.size());
        context_->GSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvGS_.locators, D3D11BindingLocator::D3DInput_SRV_GS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_PS)) != 0 && HasLocatorAndRangesOverlapAt(srvPS_, slot, locator, subresourceRange))
    {
        LLGL_ASSERT(slot < srvPS_.size());
        context_->PSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvPS_.locators, D3D11BindingLocator::D3DInput_SRV_PS, slot))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_CS)) != 0 && HasLocatorAndRangesOverlapAt(srvCS_, slot, locator, subresourceRange))
    {
        LLGL_ASSERT(slot < srvCS_.size());
        context_->CSSetShaderResources(slot, 1, nullSRVs);
        if (RemoveSubresourceInput(srvCS_.locators, D3D11BindingLocator::D3DInput_SRV_CS, slot))
            return;
    }
}

void D3D11BindingTable::EvictMultipleInputBindings(D3D11BindingLocator* locator)
{
    ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_VB)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, vb_.size()))
        {
            if (vb_.locators[slot] == locator)
            {
                /* D3D11 allows to bind vertex buffer slots independently, but LLGL always sets/unsets vertex buffers all at once, so evict all at once */
                EvictAllVertexBuffers();
                break;
            }
        }
        if (locator->inBitmask == 0)
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_IB)) != 0)
    {
        if (ib_.locators[0] == locator)
        {
            context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
            ib_.locators[0] = nullptr;
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_IB))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_VS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvVS_.size()))
        {
            if (srvVS_.locators[slot] == locator)
            {
                context_->VSSetShaderResources(slot, 1, nullSRVs);
                srvVS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_VS))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_HS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvHS_.size()))
        {
            if (srvHS_.locators[slot] == locator)
            {
                context_->HSSetShaderResources(slot, 1, nullSRVs);
                srvHS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_HS))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_DS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvDS_.size()))
        {
            if (srvDS_.locators[slot] == locator)
            {
                context_->DSSetShaderResources(slot, 1, nullSRVs);
                srvDS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_DS))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_GS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvGS_.size()))
        {
            if (srvGS_.locators[slot] == locator)
            {
                context_->GSSetShaderResources(slot, 1, nullSRVs);
                srvGS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_GS))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_PS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvPS_.size()))
        {
            if (srvPS_.locators[slot] == locator)
            {
                context_->PSSetShaderResources(slot, 1, nullSRVs);
                srvPS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_PS))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_CS)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvCS_.size()))
        {
            if (srvCS_.locators[slot] == locator)
            {
                context_->CSSetShaderResources(slot, 1, nullSRVs);
                srvCS_.locators[slot] = nullptr;
            }
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_CS))
            return;
    }
}

void D3D11BindingTable::EvictMultipleSubresourceInputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange)
{
    ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_VB)) != 0)
    {
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, vb_.size()))
        {
            if (vb_.locators[slot] == locator)
            {
                /* D3D11 allows to bind vertex buffer slots independently, but LLGL always sets/unsets vertex buffers all at once, so evict all at once */
                EvictAllVertexBuffers();
                break;
            }
        }
        if (locator->inBitmask == 0)
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_IB)) != 0)
    {
        if (ib_.locators[0] == locator)
        {
            context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
            ib_.locators[0] = nullptr;
        }
        if (locator->RemoveInput(D3D11BindingLocator::D3DInput_IB))
            return;
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_VS)) != 0)
    {
        bool hasRemainingVSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvVS_.size()))
        {
            if (srvVS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvVS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->VSSetShaderResources(slot, 1, nullSRVs);
                    srvVS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingVSBindings = true;
            }
        }
        if (!hasRemainingVSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_VS))
                return;
        }
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_HS)) != 0)
    {
        bool hasRemainingHSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvHS_.size()))
        {
            if (srvHS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvHS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->HSSetShaderResources(slot, 1, nullSRVs);
                    srvHS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingHSBindings = true;
            }
        }
        if (!hasRemainingHSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_HS))
                return;
        }
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_DS)) != 0)
    {
        bool hasRemainingDSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvDS_.size()))
        {
            if (srvDS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvDS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->DSSetShaderResources(slot, 1, nullSRVs);
                    srvDS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingDSBindings = true;
            }
        }
        if (!hasRemainingDSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_DS))
                return;
        }
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_GS)) != 0)
    {
        bool hasRemainingGSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvGS_.size()))
        {
            if (srvGS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvGS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->GSSetShaderResources(slot, 1, nullSRVs);
                    srvGS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingGSBindings = true;
            }
        }
        if (!hasRemainingGSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_GS))
                return;
        }
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_PS)) != 0)
    {
        bool hasRemainingPSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvPS_.size()))
        {
            if (srvPS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvPS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->PSSetShaderResources(slot, 1, nullSRVs);
                    srvPS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingPSBindings = true;
            }
        }
        if (!hasRemainingPSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_PS))
                return;
        }
    }

    if ((locator->inBitmask & (1u << D3D11BindingLocator::D3DInput_SRV_CS)) != 0)
    {
        bool hasRemainingCSBindings = false;
        for_subrange(slot, locator->inRangeBegin, std::min<UINT>(locator->inRangeEnd, srvCS_.size()))
        {
            if (srvCS_.locators[slot] == locator)
            {
                if (D3D11SubresourceRange::Overlap(srvCS_.subresourceRanges[slot], subresourceRange))
                {
                    context_->CSSetShaderResources(slot, 1, nullSRVs);
                    srvCS_.locators[slot] = nullptr;
                }
                else
                    hasRemainingCSBindings = true;
            }
        }
        if (!hasRemainingCSBindings)
        {
            if (locator->RemoveInput(D3D11BindingLocator::D3DInput_SRV_CS))
                return;
        }
    }
}

void D3D11BindingTable::EvictAllVertexBuffers()
{
    static const UINT nullValues[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {};
    for_range(slot, vbCount_)
    {
        if (vb_.locators[slot] != nullptr)
        {
            vb_.locators[slot]->RemoveInput(D3D11BindingLocator::D3DInput_VB);
            vb_.locators[slot] = nullptr;
        }
    }
    context_->IASetVertexBuffers(0, vbCount_, GetNullPointerArray<ID3D11Buffer>(), nullValues, nullValues);
    vbCount_ = 0;
}

void D3D11BindingTable::EvictAllStreamOutputTargets()
{
    context_->SOSetTargets(0, nullptr, nullptr);

    for_range(slot, soCount_)
        RemoveWholeResourceOutput(so_.locators, D3D11BindingLocator::D3DOutput_SO, slot);
    soCount_ = 0;
}

void D3D11BindingTable::EvictSingleUnorderedAccessViewPS(UINT slot)
{
    if (uavOMRefs_[slot] != nullptr)
    {
        /* Remove individual UAV from cache and bind cache immediately */
        uavOMRefs_[slot] = nullptr;
        BindCachedOutputMergerUAVs();
    }
}

void D3D11BindingTable::EvictSingleUnorderedAccessViewCS(UINT slot)
{
    context_->CSSetUnorderedAccessViews(
        /*StartSlot:*/              slot,
        /*NumUAVs:*/                1,
        /*ppUnorderedAccessViews:*/ GetNullPointerArray<ID3D11UnorderedAccessView>(),
        /*pUAVInitialCounts:*/      nullptr
    );
}

void D3D11BindingTable::EvictAllInputBindings(D3D11BindingLocator* locator, const D3D11SubresourceRange* subresourceRange)
{
    if (locator->inBitmask != 0)
    {
        if (subresourceRange != nullptr)
        {
            if (locator->HasSingleInputBinding())
                EvictSingleSubresourceInputBinding(locator, *subresourceRange);
            else
                EvictMultipleSubresourceInputBindings(locator, *subresourceRange);
        }
        else
        {
            if (locator->HasSingleInputBinding())
                EvictSingleInputBinding(locator);
            else
                EvictMultipleInputBindings(locator);
        }
    }
}

void D3D11BindingTable::BindCachedOutputMergerUAVs()
{
    /* Leading null UAVs must not override RTV slots, so start UAV range at least after number of RTVs */
    const UINT uavStartSlot = std::max<UINT>(rtvCount_, omUAVStartSlot_);
    const UINT uavCount     = omNumUAVs_ - (uavStartSlot - omUAVStartSlot_);
    context_->OMSetRenderTargetsAndUnorderedAccessViews(
        /*NumRTVs:*/                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
        /*ppRenderTargetViews:*/    nullptr,
        /*pDepthStencilView:*/      nullptr,
        /*UAVStartSlot:*/           uavStartSlot,
        /*NumUAVs:*/                uavCount,
        /*ppUnorderedAccessViews:*/ &(uavOMRefs_[uavStartSlot]),
        /*pUAVInitialCounts:*/      &(uavOMInitialCounts_[uavStartSlot])
    );
}


} // /namespace LLGL



// ================================================================================
