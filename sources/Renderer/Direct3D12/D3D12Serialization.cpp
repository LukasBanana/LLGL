/*
 * D3D12Serialization.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Serialization.h"


namespace LLGL
{

namespace Serialization
{


void D3D12WriteSegmentBlob(Serializer& writer, const D3D12Ident ident, ID3DBlob* blob)
{
    if (blob != nullptr)
        writer.WriteSegment(ident, blob->GetBufferPointer(), blob->GetBufferSize());
}

void D3D12WriteSegmentBytecode(Serializer& writer, const D3D12Ident ident, const D3D12_SHADER_BYTECODE& shaderBytecode)
{
    if (shaderBytecode.pShaderBytecode != nullptr && shaderBytecode.BytecodeLength > 0)
        writer.WriteSegment(ident, shaderBytecode.pShaderBytecode, shaderBytecode.BytecodeLength);
}

void D3D12ReadSegmentBlob(Deserializer& reader, const D3D12Ident ident, D3D12_CACHED_PIPELINE_STATE& cachedBlob)
{
    auto seg = reader.ReadSegment(ident);
    {
        cachedBlob.pCachedBlob              = seg.data;
        cachedBlob.CachedBlobSizeInBytes    = seg.size;
    }
}

void D3D12ReadSegmentBytecode(Deserializer& reader, const D3D12Ident ident, D3D12_SHADER_BYTECODE& shaderBytecode)
{
    auto seg = reader.ReadSegmentOnMatch(ident);
    if (seg.ident == ident)
    {
        shaderBytecode.pShaderBytecode  = seg.data;
        shaderBytecode.BytecodeLength   = seg.size;
    }
    else
    {
        shaderBytecode.pShaderBytecode  = nullptr;
        shaderBytecode.BytecodeLength   = 0;
    }
}


} // /namespace Serialization

} // /namespace LLGL



// ================================================================================
