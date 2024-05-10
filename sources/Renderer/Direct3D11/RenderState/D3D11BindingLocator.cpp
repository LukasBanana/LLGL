/*
 * D3D11BindingLocator.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11BindingLocator.h"
#include <algorithm>


namespace LLGL
{


static D3D11BindingLocator::D3DLocatorTypes GetBindingLocatorType(ResourceType resourceType, long bindFlags)
{
    constexpr long rwBufferBindFlags = (BindFlags::Storage | BindFlags::CopyDst| BindFlags::StreamOutputBuffer);
    constexpr long rwTextureBindFlags = (BindFlags::Storage | BindFlags::CopyDst | BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment);
    if (resourceType == ResourceType::Buffer && (bindFlags & rwBufferBindFlags) != 0)
        return D3D11BindingLocator::D3DLocator_RWBuffer;
    else if (resourceType == ResourceType::Texture && (bindFlags & rwTextureBindFlags) != 0)
        return D3D11BindingLocator::D3DLocator_RWTexture;
    else
        return D3D11BindingLocator::D3DLocator_ReadOnly;
}

D3D11BindingLocator::D3D11BindingLocator(ResourceType resourceType, long bindFlags) :
    type          { GetBindingLocatorType(resourceType, bindFlags) },
    inRangeBegin  { ~0u                                            },
    inRangeEnd    { 0u                                             },
    inBitmask     { 0u                                             },
    outRangeBegin { ~0u                                            },
    outRangeEnd   { 0u                                             },
    outBitmask    { 0u                                             }
{
}

void D3D11BindingLocator::InsertInputAt(D3DInputs input, UINT slot)
{
    inRangeBegin    = std::min<UINT>(inRangeBegin, slot);
    inRangeEnd      = std::max<UINT>(inRangeEnd, slot + 1u);
    inBitmask       = inBitmask | (1u << input);
}

bool D3D11BindingLocator::TryRemoveInputAt(D3DInputs input, UINT slot)
{
    /* We can only clear the input bitmask if this locator only resides in a single input table at a single binding slot */
    if (inBitmask == (1u << input))
    {
        if (inRangeBegin == slot && inRangeEnd == slot + 1)
        {
            /* This removed the last input binding, so clear the input bitmask */
            ClearInput();
            return true;
        }
        else if (inRangeBegin == slot)
        {
            /* Reduce range if this locator only refers to a single resource table and we remove the first slot */
            ++inRangeBegin;
        }
        else if (inRangeEnd == slot + 1)
        {
            /* Reduce range if this locator only refers to a single resource table and we remove the last slot */
            --inRangeEnd;
        }
    }
    return false;
}

void D3D11BindingLocator::ClearInput()
{
    inRangeBegin    = ~0u;
    inRangeEnd      = 0u;
    inBitmask       = 0u;
}

bool D3D11BindingLocator::RemoveInput(D3DInputs input)
{
    inBitmask &= ~(1u << input);
    if (inBitmask == 0)
    {
        inRangeBegin    = ~0u;
        inRangeEnd      = 0u;
        return true;
    }
    return false;
}

void D3D11BindingLocator::InsertOutputAt(D3DOutputs output, UINT slot)
{
    outRangeBegin   = std::min<UINT>(outRangeBegin, slot);
    outRangeEnd     = std::max<UINT>(outRangeEnd, slot + 1u);
    outBitmask      = outBitmask | (1u << output);
}

bool D3D11BindingLocator::TryRemoveOutputAt(D3DOutputs output, UINT slot)
{
    /* We can only clear the output bitmask if this locator only resides in a single output table at a single binding slot */
    if (outBitmask == (1u << output))
    {
        if (outRangeBegin == slot && outRangeEnd == slot + 1)
        {
            /* This removed the last output binding, so clear the output bitmask */
            ClearOutput();
            return true;
        }
        else if (outRangeBegin == slot)
        {
            /* Reduce range if this locator only refers to a single resource table and we remove the first slot */
            ++outRangeBegin;
        }
        else if (outRangeEnd == slot + 1)
        {
            /* Reduce range if this locator only refers to a single resource table and we remove the last slot */
            --outRangeEnd;
        }
    }
    return false;
}

void D3D11BindingLocator::ClearOutput()
{
    outRangeBegin   = ~0u;
    outRangeEnd     = 0u;
    outBitmask      = 0u;
}

bool D3D11BindingLocator::RemoveOutput(D3DOutputs output)
{
    outBitmask &= ~(1u << output);
    if (outBitmask == 0)
    {
        outRangeBegin   = ~0u;
        outRangeEnd     = 0u;
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
