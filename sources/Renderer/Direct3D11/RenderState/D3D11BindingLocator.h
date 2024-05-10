/*
 * D3D11BindingLocator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BINDING_LOCATOR_H
#define LLGL_D3D11_BINDING_LOCATOR_H


#include <LLGL/ResourceFlags.h>
#include <d3d11.h>


namespace LLGL
{


// Ranges to determine overlaps between SRV and UAV subresources of the same parent resource.
struct D3D11SubresourceRange
{
    static inline bool Overlap(const D3D11SubresourceRange& lhs, const D3D11SubresourceRange& rhs)
    {
        return (lhs.begin < rhs.end && lhs.end > rhs.begin);
    }

    UINT begin;
    UINT end;
};

// Each D3D11Buffer and D3D11Texture has one such locator to speed up the lookup within the device-context's binding table.
struct D3D11BindingLocator
{
    enum D3DLocatorBits
    {
        D3DLocatorInRangeBits   = 8,
        D3DLocatorOutRangeBits  = 7,
    };

    enum D3DLocatorTypes : UINT
    {
        D3DLocator_ReadOnly = 0,
        D3DLocator_RWBuffer,    // Buffer resource with BindFlags::Storage/ CopyDst/ StreamOutputBuffer flag
        D3DLocator_RWTexture,   // Texture resource with BindFlags::Storage/ CopyDst/ ColorAttachent/ DepthStencilAttachent flag
    };

    enum D3DInputs
    {
        D3DInput_VB = 0,        // Vertex buffer
        D3DInput_IB,            // Index buffer
        D3DInput_SRV_VS,        // Shader resource view for vertex-shader stage
        D3DInput_SRV_HS,        // Shader resource view for hull-shader stage
        D3DInput_SRV_DS,        // Shader resource view for domain-shader stage
        D3DInput_SRV_GS,        // Shader resource view for geometry-shader stage
        D3DInput_SRV_PS,        // Shader resource view for pixel-shader stage
        D3DInput_SRV_CS,        // Shader resource view for compute-shader stage

        D3DInput_Num,
    };

    enum D3DOutputs
    {
        D3DOutput_SO = 0,       // Stream output
        D3DOutput_UAV_PS,       // Unordered access view for pixel-shader stage
        D3DOutput_UAV_CS,       // Unordered access view for compute-shader stage
        D3DOutput_RTV,          // Render target view
        D3DOutput_DSV,          // Depth stencil view

        D3DOutput_Num,
    };

    static_assert(
        (1 << D3DLocatorInRangeBits) > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
        "Number of bits for 'D3DLocatorInRangeBits' must fit into D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT range"
    );

    static_assert(
        (1 << D3DLocatorOutRangeBits) > D3D11_1_UAV_SLOT_COUNT,
        "Number of bits for 'D3DLocatorOutRangeBits' must fit into D3D11_1_UAV_SLOT_COUNT range"
    );

    D3D11BindingLocator(ResourceType resourceType, long bindFlags);

    D3D11BindingLocator(const D3D11BindingLocator&) = delete;
    D3D11BindingLocator& operator = (const D3D11BindingLocator&) = delete;

    void InsertInputAt(D3DInputs input, UINT slot);
    bool TryRemoveInputAt(D3DInputs input, UINT slot);
    void ClearInput();
    bool RemoveInput(D3DInputs input);

    inline bool HasSingleInputBinding() const
    {
        return (inRangeBegin + 1 == inRangeEnd);
    }

    void InsertOutputAt(D3DOutputs output, UINT slot);
    bool TryRemoveOutputAt(D3DOutputs output, UINT slot);
    void ClearOutput();
    bool RemoveOutput(D3DOutputs output);

    inline bool HasSingleOutputBinding() const
    {
        return (outRangeBegin + 1 == outRangeEnd);
    }

    D3DLocatorTypes type            : 2;
    UINT            inRangeBegin    : D3DLocatorInRangeBits;
    UINT            inRangeEnd      : D3DLocatorInRangeBits;
    UINT            inBitmask       : D3DInput_Num;
    UINT            outRangeBegin   : D3DLocatorOutRangeBits;
    UINT            outRangeEnd     : D3DLocatorOutRangeBits;
    UINT            outBitmask      : D3DOutput_Num;
};


} // /namespace LLGL


#endif



// ================================================================================
