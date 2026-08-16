/*
 * D3D11GraphicsPSOBase.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11GraphicsPSOBase.h"
#include "D3D11StateManager.h"
#include "D3D11PipelineLayout.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"
#include "../D3D11ObjectUtils.h"
#include "../Shader/D3D11DomainShader.h"
#include "../Shader/D3D11VertexShader.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/TypeNames.h>
#include <stdexcept>


namespace LLGL
{


void D3D11GraphicsPSOBase::Bind(D3D11StateManager& stateMngr)
{
    /* Set input-assembly states */
    stateMngr.SetPrimitiveTopology(primitiveTopology_);
    stateMngr.SetInputLayout(inputLayout_.Get());

    /* Set shader stages */
    stateMngr.SetVertexShader   (vs_.Get());
    stateMngr.SetHullShader     (hs_.Get());
    stateMngr.SetDomainShader   (ds_.Get());
    stateMngr.SetGeometryShader (gs_.Get());
    stateMngr.SetPixelShader    (ps_.Get());

    /* Set static viewports and scissors */
    SetStaticViewportsAndScissors(stateMngr);

    /* Set static samplers */
    if (const D3D11PipelineLayout* pipelineLayoutD3D = GetPipelineLayout())
        pipelineLayoutD3D->BindGraphicsStaticSamplers(stateMngr);
}


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

void D3D11GraphicsPSOBase::BuildInputLayout(LLGL::ArrayView<VertexAttribute> attributes, LLGL::DynamicVector<D3D11_INPUT_ELEMENT_DESC>& outputAttributes)
{
    const UINT numVertexAttribs = static_cast<UINT>(attributes.size());

    outputAttributes.resize(numVertexAttribs);

    for_range(i, numVertexAttribs)
        ConvertInputElementDesc(outputAttributes[i], attributes[i]);
}

// Converts a vertex attribute to a D3D stream-output entry
static void ConvertSODeclEntry(D3D11_SO_DECLARATION_ENTRY& dst, const VertexAttribute& src)
{
    const char* systemValueSemantic = DXTypes::SystemValueToString(src.systemValue);
    dst.Stream          = 0; //TODO: not sure what Stream refers to here, since OutputSlot is already used for
    dst.SemanticName    = (systemValueSemantic != nullptr ? systemValueSemantic : src.name.c_str());
    dst.SemanticIndex   = src.semanticIndex;
    dst.StartComponent  = 0;
    dst.ComponentCount  = GetFormatAttribs(src.format).components;
    dst.OutputSlot      = src.slot;
}

void D3D11GraphicsPSOBase::BuildStreamOutput(LLGL::ArrayView<VertexAttribute> attributes, LLGL::DynamicVector<D3D11_SO_DECLARATION_ENTRY>& output, UINT bufferStrides[D3D11_SO_BUFFER_SLOT_COUNT], UINT& numBufferStrides)
{
    const UINT numStreamOutputAttribs = static_cast<UINT>(attributes.size());

    /* Initialize output elements for geometry shader with stream-output */
    output.resize(numStreamOutputAttribs);

    for_range(i, numStreamOutputAttribs)
    {
        ConvertSODeclEntry(output[i], attributes[i]);
        LLGL_ASSERT(output[i].OutputSlot < D3D11_SO_BUFFER_SLOT_COUNT); //TODO: replace with error report
        bufferStrides[output[i].OutputSlot] = attributes[i].stride;
        numBufferStrides = std::max<UINT>(numBufferStrides, output[i].OutputSlot + 1);
    }
}

/*
 * ======= Protected: =======
 */

D3D11GraphicsPSOBase::D3D11GraphicsPSOBase(ID3D11Device* device, const GraphicsPipelineDescriptor& desc) :
    D3D11PipelineState { /*isGraphicsPSO:*/ true, desc.pipelineLayout, GetShadersAsArray(desc) }
{
    /* Validate pointers and get D3D shader objects */
    if (auto* vertexShaderD3D = LLGL_CAST(const D3D11VertexShader*, desc.vertexShader))
    {
        /* Take input layout and store optional proxy geometry-shader for stream-output */

        // FIXME: https://github.com/LukasBanana/LLGL/pull/257#discussion_r3767429388

        auto* vertexByteCode = vertexShaderD3D->GetByteCode();

        DynamicVector<D3D11_INPUT_ELEMENT_DESC> inputElements;
        BuildInputLayout(desc.inputVertexAttribs, inputElements);

        if (!inputElements.empty())
        {
            HRESULT hr = device->CreateInputLayout(
                inputElements.data(),
                static_cast<UINT>(inputElements.size()),
                vertexByteCode->GetBufferPointer(),
                vertexByteCode->GetBufferSize(),
                inputLayout_.ReleaseAndGetAddressOf()
            );
            DXThrowIfFailed(hr, "failed to create D3D11 input layout");
        }

        std::vector<D3D11_SO_DECLARATION_ENTRY> outputElements;
        outputElements.resize(desc.outputVertexAttribs.size());

        UINT bufferStrides[D3D11_SO_BUFFER_SLOT_COUNT];
        UINT numBufferStrides = 0;

        if (!outputElements.empty())
        {
            // FIXME: https://github.com/LukasBanana/LLGL/pull/257#discussion_r3784385726
            /* Create geometry shader with stream-output declaration */
            HRESULT hr = device->CreateGeometryShaderWithStreamOutput(
                vertexByteCode->GetBufferPointer(),
                vertexByteCode->GetBufferSize(),
                outputElements.data(),
                static_cast<UINT>(outputElements.size()),
                bufferStrides,
                numBufferStrides,
                0,
                nullptr,
                gs_.ReleaseAndGetAddressOf()
            );
            DXThrowIfCreateFailed(hr, "failed to create proxy geometry shader");
        }

        // Deprecated feature support: Get input layout from the vertex shader if we failed to get it from the pipeline descriptor.
        if (inputLayout_ == nullptr)
        {
            inputLayout_ = vertexShaderD3D->GetInputLayout();
        }

        // Deprecated feature support: Get proxy geometry shader from the vertex shader if we failed to get it from the pipeline descriptor.
        if (gs_ == nullptr)
        {
            gs_ = vertexShaderD3D->GetProxyGeometryShader();
        }
    }
    else
        ResetReport("cannot create D3D graphics PSO without vertex shader", true);

    /* Override proxy geometry shader if the domain shader has one */
    if (auto* domainShaderD3D = LLGL_CAST(const D3D11DomainShader*, desc.tessEvaluationShader))
    {
        if (domainShaderD3D->GetProxyGeometryShader())
            gs_ = domainShaderD3D->GetProxyGeometryShader();
    }

    GetD3DNativeShaders(desc);

    /* Store dynamic pipeline states */
    primitiveTopology_  = DXTypes::ToD3DPrimitiveTopology(desc.primitiveTopology);
    stencilRefDynamic_  = desc.stencil.referenceDynamic;
    stencilRef_         = desc.stencil.front.reference;
    blendFactorDynamic_ = desc.blend.blendFactorDynamic;
    blendFactor_[0]     = desc.blend.blendFactor[0];
    blendFactor_[1]     = desc.blend.blendFactor[1];
    blendFactor_[2]     = desc.blend.blendFactor[2];
    blendFactor_[3]     = desc.blend.blendFactor[3];
    sampleMask_         = desc.blend.sampleMask;

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);
}

void D3D11GraphicsPSOBase::SetStaticViewportsAndScissors(D3D11StateManager& stateMngr)
{
    if (staticStateBuffer_)
    {
        ByteBufferConstIterator byteBufferIter = staticStateBuffer_.GetBufferIterator();
        if (staticStateBuffer_.GetNumViewports() > 0)
        {
            stateMngr.GetContext()->RSSetViewports(
                staticStateBuffer_.GetNumViewports(),
                byteBufferIter.Next<D3D11_VIEWPORT>(staticStateBuffer_.GetNumViewports())
            );
        }
        if (staticStateBuffer_.GetNumScissors() > 0)
        {
            stateMngr.GetContext()->RSSetScissorRects(
                staticStateBuffer_.GetNumScissors(),
                byteBufferIter.Next<D3D11_RECT>(staticStateBuffer_.GetNumScissors())
            );
        }
    }
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPSOBase::GetD3DNativeShaders(const GraphicsPipelineDescriptor& desc)
{
    if (Shader* vs = desc.vertexShader        ) { D3D11CastShader(vs_, LLGL_CAST(D3D11Shader*, vs)->GetNative(), ShaderType::Vertex,         desc.debugName, GetMutableReport()); }
    if (Shader* hs = desc.tessControlShader   ) { D3D11CastShader(hs_, LLGL_CAST(D3D11Shader*, hs)->GetNative(), ShaderType::TessControl,    desc.debugName, GetMutableReport()); }
    if (Shader* ds = desc.tessEvaluationShader) { D3D11CastShader(ds_, LLGL_CAST(D3D11Shader*, ds)->GetNative(), ShaderType::TessEvaluation, desc.debugName, GetMutableReport()); }
    if (Shader* gs = desc.geometryShader      ) { D3D11CastShader(gs_, LLGL_CAST(D3D11Shader*, gs)->GetNative(), ShaderType::Geometry,       desc.debugName, GetMutableReport()); }
    if (Shader* ps = desc.fragmentShader      ) { D3D11CastShader(ps_, LLGL_CAST(D3D11Shader*, ps)->GetNative(), ShaderType::Fragment,       desc.debugName, GetMutableReport()); }
}

void D3D11GraphicsPSOBase::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    ByteBufferIterator byteBufferIter = staticStateBuffer_.Allocate(
        desc.viewports.size(), desc.scissors.size(),
        sizeof(D3D11_VIEWPORT), sizeof(D3D11_RECT),
        GetMutableReport(), D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
    );

    /* Build <D3D11_VIEWPORT> entries */
    for_range(i, staticStateBuffer_.GetNumViewports())
    {
        D3D11_VIEWPORT* dst = byteBufferIter.Next<D3D11_VIEWPORT>();
        {
            dst->TopLeftX   = desc.viewports[i].x;
            dst->TopLeftY   = desc.viewports[i].y;
            dst->Width      = desc.viewports[i].width;
            dst->Height     = desc.viewports[i].height;
            dst->MinDepth   = desc.viewports[i].minDepth;
            dst->MaxDepth   = desc.viewports[i].maxDepth;
        }
    }

    /* Build <D3D11_RECT> entries */
    for_range(i, staticStateBuffer_.GetNumScissors())
    {
        D3D11_RECT* dst = byteBufferIter.Next<D3D11_RECT>();
        {
            dst->left   = static_cast<LONG>(desc.scissors[i].x);
            dst->top    = static_cast<LONG>(desc.scissors[i].y);
            dst->right  = static_cast<LONG>(desc.scissors[i].x + desc.scissors[i].width);
            dst->bottom = static_cast<LONG>(desc.scissors[i].y + desc.scissors[i].height);
        }
    }
}


} // /namespace LLGL



// ================================================================================
