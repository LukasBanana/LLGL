/*
 * D3D12Serialization.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SERIALIZATION_H
#define LLGL_D3D12_SERIALIZATION_H


#include "../Serialization.h"
#include <LLGL/RenderSystemFlags.h>
#include <d3dcommon.h>
#include <d3d12.h>


namespace LLGL
{

namespace Serialization
{


/* ----- Enumerations ----- */

// Segment identifiers for D3D12 serialization.
enum D3D12Ident : IdentType
{
    D3D12Ident_ReservedD3D12 = (RendererID::Direct3D12 << 8),
    D3D12Ident_GraphicsPSOIdent,
    D3D12Ident_ComputePSOIdent,
    D3D12Ident_RootSignature,       // Serialized ID3D12RootSignature
    D3D12Ident_CachedPSO,           // D3D12_CACHED_PIPELINE_STATE
    D3D12Ident_GraphicsDesc,        // D3D12_GRAPHICS_PIPELINE_STATE_DESC
    D3D12Ident_ComputeDesc,         // D3D12_COMPUTE_PIPELINE_STATE_DESC
    D3D12Ident_StaticState,         // D3D12_VIEWPORT[n]; D3D12_RECT[n]
    D3D12Ident_InputElements,       // D3D12_INPUT_ELEMENT_DESC
    D3D12Ident_InputSemanticNames,  // LPCSTR[n]
    D3D12Ident_SODeclEntries,       // D3D12_SO_DECLARATION_ENTRY[n]
    D3D12Ident_SOSemanticNames,     // LPCSTR[n]
    D3D12Ident_SOBufferStrides,     // UINT[n]
    D3D12Ident_VS,                  // D3D12_SHADER_BYTECODE
    D3D12Ident_PS,                  // D3D12_SHADER_BYTECODE
    D3D12Ident_DS,                  // D3D12_SHADER_BYTECODE
    D3D12Ident_HS,                  // D3D12_SHADER_BYTECODE
    D3D12Ident_GS,                  // D3D12_SHADER_BYTECODE
    D3D12Ident_CS,                  // D3D12_SHADER_BYTECODE
};


/* ----- Functions ----- */

// Writes the specified blob as a serialized segment.
void D3D12WriteSegmentBlob(Serializer& writer, const D3D12Ident ident, ID3DBlob* blob);

// Writes the specified shader bytecode as a serialized segment.
void D3D12WriteSegmentBytecode(Serializer& writer, const D3D12Ident ident, const D3D12_SHADER_BYTECODE& shaderBytecode);

// Reads a blob from the next deserialized segment.
void D3D12ReadSegmentBlob(Deserializer& reader, const D3D12Ident ident, D3D12_CACHED_PIPELINE_STATE& cachedBlob);

// Reads a shader bytecode block from the next deserialized segment.
void D3D12ReadSegmentBytecode(Deserializer& reader, const D3D12Ident ident, D3D12_SHADER_BYTECODE& shaderBytecode);


} // /namespace Serialization

} // /namespace LLGL


#endif



// ================================================================================
