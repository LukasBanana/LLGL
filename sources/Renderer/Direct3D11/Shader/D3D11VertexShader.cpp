/*
 * D3D11VertexShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11VertexShader.h"
#include "../RenderState/D3D11PipelineLayout.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/Assertion.h"
#include <LLGL/VertexAttribute.h>
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
            BuildProxyGeometryShader(device, desc, proxyGeometryShader_);
    }
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}


/*
 * ======= Private: =======
 */

void D3D11VertexShader::BuildInputLayout(ID3D11Device* device, UINT numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    /* Check if input layout is allowed */
    LLGL_ASSERT(GetType() == ShaderType::Vertex, "cannot build input layout for non-vertex-shader");

    /* Setup input element descriptors */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    LLGL::D3D11PipelineLayout::BuildInputLayout({vertexAttribs, numVertexAttribs}, inputElements);

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
