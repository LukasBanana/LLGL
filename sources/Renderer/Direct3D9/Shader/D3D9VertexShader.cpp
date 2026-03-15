/*
 * D3D9VertexShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9VertexShader.h"
#include "../D3D9Core.h"
#include "../D3D9Types.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/StringUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static bool HasAnyInstanceDivisors(ArrayView<VertexAttribute> vertexAttribs)
{
    for (const VertexAttribute& attrib : vertexAttribs)
    {
        if (attrib.instanceDivisor > 0)
            return true;
    }
    return false;
}

D3D9VertexShader::D3D9VertexShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc) :
    D3D9Shader { ShaderType::Vertex }
{
    if (BuildShader(device, shaderDesc))
    {
        BuildVertexDeclaration(device, shaderDesc.vertex.inputAttribs);
        if (HasAnyInstanceDivisors(shaderDesc.vertex.inputAttribs))
            BuildStreamSourceFreq(shaderDesc.vertex.inputAttribs);
    }
}

bool D3D9VertexShader::Reflect(ShaderReflection& reflection) const
{
    return false; //TODO
}


/*
 * ======= Private: =======
 */

HRESULT D3D9VertexShader::CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode)
{
    return device->CreateVertexShader(static_cast<const DWORD*>(byteCode->GetBufferPointer()), d3dShader_.ReleaseAndGetAddressOf());
}

static D3DDECLUSAGE MapToD3DDeclUsage(const char* semanticsName)
{
    if (StrcmpCi(semanticsName, "POSITION") == 0)
        return D3DDECLUSAGE_POSITION;
    if (StrcmpCi(semanticsName, "NORMAL") == 0)
        return D3DDECLUSAGE_NORMAL;
    if (StrcmpCi(semanticsName, "COLOR") == 0)
        return D3DDECLUSAGE_COLOR;
    return D3DDECLUSAGE_TEXCOORD;
}

static void ConvertD3DVertexAttrib(D3DVERTEXELEMENT9& outVertexElement, const VertexAttribute& inAttrib)
{
    outVertexElement.Stream     = static_cast<WORD>(inAttrib.slot); // Stream index
    outVertexElement.Offset     = static_cast<WORD>(inAttrib.offset); // Offset in the stream in bytes
    outVertexElement.Type       = D3D9Types::ToD3DDeclType(inAttrib.format); // Data type
    outVertexElement.Method     = D3DDECLMETHOD_DEFAULT; // Processing method
    outVertexElement.Usage      = MapToD3DDeclUsage(inAttrib.name.c_str()); // Semantics
    outVertexElement.UsageIndex = static_cast<BYTE>(inAttrib.semanticIndex); // Semantic index
}

void D3D9VertexShader::BuildVertexDeclaration(IDirect3DDevice9* device, ArrayView<VertexAttribute> vertexAttribs)
{
    std::vector<D3DVERTEXELEMENT9> vertexElements;
    vertexElements.resize(vertexAttribs.size() + 1);

    for_range(i, vertexAttribs.size())
        ConvertD3DVertexAttrib(vertexElements[i], vertexAttribs[i]);

    vertexElements.back() = D3DDECL_END();

    HRESULT hr = device->CreateVertexDeclaration(vertexElements.data(), d3dVertexDecl_.ReleaseAndGetAddressOf());
    D3DThrowIfCreateFailed(hr, "IDirect3DVertexDeclaration9");
}

void D3D9VertexShader::BuildStreamSourceFreq(ArrayView<VertexAttribute> vertexAttribs)
{
    for (const VertexAttribute& attrib : vertexAttribs)
    {
        if (attrib.instanceDivisor > 0)
        {
            LLGL_ASSERT(attrib.slot > 0, "per-instance vertex data cannot be set in stream 0 in D3D9");

            /* Try to find existing entry for this stream first */
            auto it = std::find_if(
                streamSourceFreq_.begin(),
                streamSourceFreq_.end(),
                [&attrib](const D3D9StreamSourceFreq& entry) -> bool
                {
                    return (entry.stream == attrib.slot);
                }
            );
            if (it != streamSourceFreq_.end())
            {
                /* Match existing stream soruce frequency with new one. If they occupy the same stream, they must have the same divider */
                const UINT existingDivier = (it->divider ^ D3DSTREAMSOURCE_INSTANCEDATA);
                LLGL_ASSERT(
                    existingDivier == attrib.instanceDivisor,
                    "mismtach between existing instance divisor (%u) and existing one (%u) for attribute '%s'",
                    attrib.instanceDivisor, existingDivier, attrib.name.c_str()
                );
            }
            else
            {
                /* Add new stream source frequency */
                D3D9StreamSourceFreq streamSourceFreq;
                {
                    streamSourceFreq.stream     = attrib.slot;
                    streamSourceFreq.divider    = (D3DSTREAMSOURCE_INSTANCEDATA | attrib.instanceDivisor);
                }
                streamSourceFreq_.push_back(streamSourceFreq);
            }
        }
    }
}


} // /namespace LLGL



// ================================================================================
