/*
 * D3D11VertexShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11VertexShader.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


D3D11VertexShader::D3D11VertexShader(ID3D11Device* device, const ShaderDescriptor& desc) :
    D3D11Shader { desc.type }
{
    if (BuildShader(device, desc))
    {
        /* Build input layout object for vertex shaders */
        BuildInputLayout(device, static_cast<UINT>(desc.vertex.inputAttribs.size()), desc.vertex.inputAttribs.data());

        /* Build optional proxy geometry shader if there are any output attributes */
        if (!desc.vertex.outputAttribs.empty())
            BuildProxyGeometryShader(device, desc, proxyGeomtryShader_);
    }
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}


/*
 * ======= Private: =======
 */

// Converts a vertex attribute to a D3D input element descriptor
static void ConvertInputElementDesc(D3D11_INPUT_ELEMENT_DESC& dst, const VertexAttribute& src)
{
    dst.SemanticName            = src.name.c_str();
    dst.SemanticIndex           = src.semanticIndex;
    dst.Format                  = DXTypes::ToDXGIFormat(src.format);
    dst.InputSlot               = src.slot;
    dst.AlignedByteOffset       = src.offset;
    dst.InputSlotClass          = (src.instanceDivisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA);
    dst.InstanceDataStepRate    = src.instanceDivisor;
}

void D3D11VertexShader::BuildInputLayout(ID3D11Device* device, UINT numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    /* Check if input layout is allowed */
    LLGL_ASSERT(GetType() == ShaderType::Vertex, "cannot build input layout for non-vertex-shader");

    /* Setup input element descriptors */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    inputElements.resize(numVertexAttribs);

    for_range(i, numVertexAttribs)
        ConvertInputElementDesc(inputElements[i], vertexAttribs[i]);

    /* Create input layout */
    HRESULT hr = device->CreateInputLayout(
        inputElements.data(),
        numVertexAttribs,
        GetByteCode()->GetBufferPointer(),
        GetByteCode()->GetBufferSize(),
        inputLayout_.ReleaseAndGetAddressOf()
    );
    DXThrowIfFailed(hr, "failed to create D3D11 input layout");
}


} // /namespace LLGL



// ================================================================================
