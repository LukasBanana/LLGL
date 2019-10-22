/*
 * D3D12GraphicsPSO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12GraphicsPSO.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12RenderPass.h"
#include "D3D12PipelineLayout.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Serialization.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
#include <algorithm>
#include <limits>


namespace LLGL
{


// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPSO::D3D12GraphicsPSO(
    D3D12Device&                        device,
    D3D12PipelineLayout&                defaultPipelineLayout,
    const GraphicsPipelineDescriptor&   desc,
    const D3D12RenderPass*              defaultRenderPass,
    Serialization::Serializer*          writer)
:
    D3D12PipelineState { true, desc.pipelineLayout, defaultPipelineLayout }
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);
    auto shaderProgramD3D = LLGL_CAST(const D3D12ShaderProgram*, desc.shaderProgram);

    /* Use either default render pass or from descriptor */
    const D3D12RenderPass* renderPass = nullptr;
    if (desc.renderPass != nullptr)
        renderPass = LLGL_CAST(const D3D12RenderPass*, desc.renderPass);
    else
        renderPass = defaultRenderPass;

    /* Store dynamic pipeline states */
    primitiveTopology_  = D3D12Types::Map(desc.primitiveTopology);
    scissorEnabled_     = desc.rasterizer.scissorTestEnabled;

    stencilRefEnabled_  = IsStaticStencilRefEnabled(desc.stencil);
    stencilRef_         = desc.stencil.front.reference;

    blendFactorEnabled_ = IsStaticBlendFactorEnabled(desc.blend);
    blendFactor_[0]     = desc.blend.blendFactor.r;
    blendFactor_[1]     = desc.blend.blendFactor.g;
    blendFactor_[2]     = desc.blend.blendFactor.b;
    blendFactor_[3]     = desc.blend.blendFactor.a;

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);

    /* Get D3D pipeline layout */
    const D3D12PipelineLayout* pipelineLayoutD3D = nullptr;
    if (desc.pipelineLayout != nullptr)
        pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
    else
        pipelineLayoutD3D = &defaultPipelineLayout;

    /* Create native graphics PSO */
    CreateNativePSOFromDesc(device, *pipelineLayoutD3D, *shaderProgramD3D, renderPass, desc, writer);
}

D3D12GraphicsPSO::D3D12GraphicsPSO(D3D12Device& device, Serialization::Deserializer& reader) :
    D3D12PipelineState { true, device.GetNative(), reader }
{
    CreateNativePSOFromCache(device, reader);
}

void D3D12GraphicsPSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetGraphicsRootSignature(GetRootSignature());
    commandContext.SetPipelineState(GetNative());

    /* Set dynamic pipeline states */
    auto commandList = commandContext.GetCommandList();

    commandList->IASetPrimitiveTopology(primitiveTopology_);

    if (stencilRefEnabled_)
        commandList->OMSetStencilRef(stencilRef_);
    if (blendFactorEnabled_)
        commandList->OMSetBlendFactor(blendFactor_);

    /* Set static viewports and scissors */
    SetStaticViewportsAndScissors(commandList);
}

UINT D3D12GraphicsPSO::NumDefaultScissorRects() const
{
    return std::max(numStaticViewports_, 1u);
}

static D3D12_CONSERVATIVE_RASTERIZATION_MODE GetConservativeRaster(bool enabled)
{
    return (enabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}

static D3D12_SHADER_BYTECODE GetShaderByteCode(D3D12Shader* shader)
{
    return (shader != nullptr ? shader->GetByteCode() : D3D12_SHADER_BYTECODE{ nullptr, 0 });
}

static UINT8 GetColorWriteMask(const ColorRGBAb& color)
{
    UINT8 mask = 0;

    if (color.r) { mask |= D3D12_COLOR_WRITE_ENABLE_RED;   }
    if (color.g) { mask |= D3D12_COLOR_WRITE_ENABLE_GREEN; }
    if (color.b) { mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;  }
    if (color.a) { mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA; }

    return mask;
}

static void Convert(D3D12_DEPTH_STENCILOP_DESC& dst, const StencilFaceDescriptor& src)
{
    dst.StencilFailOp       = D3D12Types::Map(src.stencilFailOp);
    dst.StencilDepthFailOp  = D3D12Types::Map(src.depthFailOp);
    dst.StencilPassOp       = D3D12Types::Map(src.depthPassOp);
    dst.StencilFunc         = D3D12Types::Map(src.compareOp);
}

static void Convert(D3D12_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil)
{
    dst.DepthEnable         = DXBoolean(srcDepth.testEnabled);
    dst.DepthWriteMask      = (srcDepth.writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO);
    dst.DepthFunc           = D3D12Types::Map(srcDepth.compareOp);
    dst.StencilEnable       = DXBoolean(srcStencil.testEnabled);
    dst.StencilReadMask     = static_cast<UINT8>(srcStencil.front.readMask);
    dst.StencilWriteMask    = static_cast<UINT8>(srcStencil.front.writeMask);

    Convert(dst.FrontFace, srcStencil.front);
    Convert(dst.BackFace, srcStencil.back);
}

static void Convert(D3D12_RENDER_TARGET_BLEND_DESC& dst, const BlendTargetDescriptor& src)
{
    dst.BlendEnable             = DXBoolean(src.blendEnabled);
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12Types::Map(src.srcColor);
    dst.DestBlend               = D3D12Types::Map(src.dstColor);
    dst.BlendOp                 = D3D12Types::Map(src.colorArithmetic);
    dst.SrcBlendAlpha           = D3D12Types::Map(src.srcAlpha);
    dst.DestBlendAlpha          = D3D12Types::Map(src.dstAlpha);
    dst.BlendOpAlpha            = D3D12Types::Map(src.alphaArithmetic);
    dst.LogicOp                 = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = GetColorWriteMask(src.colorMask);
}

static void SetBlendDescToDefault(D3D12_RENDER_TARGET_BLEND_DESC& dst)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

static void SetBlendDescToLogicOp(D3D12_RENDER_TARGET_BLEND_DESC& dst, D3D12_LOGIC_OP logicOp)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = TRUE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = logicOp;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

static void Convert(D3D12_BLEND_DESC& dst, DXGI_FORMAT (&dstColorFormats)[8], const BlendDescriptor& src, UINT numAttachments)
{
    dst.AlphaToCoverageEnable = DXBoolean(src.alphaToCoverageEnabled);

    if (src.logicOp == LogicOp::Disabled)
    {
        /* Enable independent blend states when multiple targets are specified */
        dst.IndependentBlendEnable = DXBoolean(src.independentBlendEnabled);

        for (UINT i = 0; i < 8u; ++i)
        {
            if (i < numAttachments)
            {
                /* Convert blend target descriptor */
                Convert(dst.RenderTarget[i], src.targets[i]);
                dstColorFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(dst.RenderTarget[i]);
                dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
            }
        }
    }
    else
    {
        /* Independent blend states is not allowed when logic operations are used */
        dst.IndependentBlendEnable = FALSE;

        /*
        Special output format required for logic operations
        see https://msdn.microsoft.com/en-us/library/windows/desktop/mt426648(v=vs.85).aspx
        */
        SetBlendDescToLogicOp(dst.RenderTarget[0], D3D12Types::Map(src.logicOp));
        dstColorFormats[0] = DXGI_FORMAT_R8G8B8A8_UINT;

        /* Initialize remaining blend target to default values */
        for (int i = 1; i < 8; ++i)
        {
            SetBlendDescToDefault(dst.RenderTarget[i]);
            dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
}

static void Convert(D3D12_BLEND_DESC& dst, DXGI_FORMAT (&dstColorFormats)[8], const BlendDescriptor& src, const D3D12RenderPass& renderPass)
{
    dst.AlphaToCoverageEnable = DXBoolean(src.alphaToCoverageEnabled);

    if (src.logicOp == LogicOp::Disabled)
    {
        /* Enable independent blend states when multiple targets are specified */
        dst.IndependentBlendEnable = DXBoolean(src.independentBlendEnabled);

        for (UINT i = 0; i < 8u; ++i)
        {
            if (i < renderPass.GetNumColorAttachments())
            {
                /* Convert blend target descriptor */
                Convert(dst.RenderTarget[i], src.targets[i]);
                dstColorFormats[i] = renderPass.GetRTVFormats()[i];
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(dst.RenderTarget[i]);
                dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
            }
        }
    }
    else
    {
        /* Independent blend states is not allowed when logic operations are used */
        dst.IndependentBlendEnable = FALSE;

        /*
        Special output format required for logic operations
        see https://msdn.microsoft.com/en-us/library/windows/desktop/mt426648(v=vs.85).aspx
        */
        SetBlendDescToLogicOp(dst.RenderTarget[0], D3D12Types::Map(src.logicOp));

        if (renderPass.GetNumColorAttachments() > 0)
            dstColorFormats[0] = D3D12Types::ToDXGIFormatUInt(renderPass.GetRTVFormats()[0]);
        else
            dstColorFormats[0] = DXGI_FORMAT_UNKNOWN;

        /* Initialize remaining blend target to default values */
        for (int i = 1; i < 8; ++i)
        {
            SetBlendDescToDefault(dst.RenderTarget[i]);
            dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
}

static void Convert(D3D12_RASTERIZER_DESC& dst, const RasterizerDescriptor& src)
{
    dst.FillMode                = D3D12Types::Map(src.polygonMode);
    dst.CullMode                = D3D12Types::Map(src.cullMode);
    dst.FrontCounterClockwise   = DXBoolean(src.frontCCW);
    dst.DepthBias               = static_cast<INT>(src.depthBias.constantFactor);
    dst.DepthBiasClamp          = src.depthBias.clamp;
    dst.SlopeScaledDepthBias    = src.depthBias.slopeFactor;
    dst.DepthClipEnable         = DXBoolean(!src.depthClampEnabled);
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled);
    dst.AntialiasedLineEnable   = DXBoolean(src.antiAliasedLineEnabled);
    dst.ForcedSampleCount       = 0; // no forced sample count
    dst.ConservativeRaster      = GetConservativeRaster(src.conservativeRasterization);
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveToplogyType(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineLoop:
        case PrimitiveTopology::LineListAdjacency:
        case PrimitiveTopology::LineStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::TriangleListAdjacency:
        case PrimitiveTopology::TriangleStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        default:
            if (topology >= PrimitiveTopology::Patches1 && topology <= PrimitiveTopology::Patches32)
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            break;
    }
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

void D3D12GraphicsPSO::CreateNativePSOFromDesc(
    D3D12Device&                        device,
    const D3D12PipelineLayout&          pipelineLayout,
    const D3D12ShaderProgram&           shaderProgram,
    const D3D12RenderPass*              renderPass,
    const GraphicsPipelineDescriptor&   desc,
    Serialization::Serializer*          writer)
{
    /* Get number of render-target attachments */
    const UINT numAttachments = (renderPass != nullptr ? renderPass->GetNumColorAttachments() : 1);

    /* Initialize D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};
    stateDesc.pRootSignature = GetRootSignature();

    /* Get shader byte codes */
    stateDesc.VS = GetShaderByteCode(shaderProgram.GetVS());
    stateDesc.PS = GetShaderByteCode(shaderProgram.GetPS());
    stateDesc.DS = GetShaderByteCode(shaderProgram.GetDS());
    stateDesc.HS = GetShaderByteCode(shaderProgram.GetHS());
    stateDesc.GS = GetShaderByteCode(shaderProgram.GetGS());

    /* Convert blend state and depth-stencil format */
    if (renderPass != nullptr)
    {
        stateDesc.DSVFormat = renderPass->GetDSVFormat();
        Convert(stateDesc.BlendState, stateDesc.RTVFormats, desc.blend, *renderPass);
    }
    else
    {
        stateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        Convert(stateDesc.BlendState, stateDesc.RTVFormats, desc.blend, numAttachments);
    }

    /* Convert rasterizer state */
    Convert(stateDesc.RasterizerState, desc.rasterizer);

    /* Convert depth-stencil state */
    Convert(stateDesc.DepthStencilState, desc.depth, desc.stencil);

    /* Convert other states */
    stateDesc.InputLayout           = shaderProgram.GetInputLayoutDesc();
    stateDesc.StreamOutput          = shaderProgram.GetStreamOutputDesc();
    stateDesc.IBStripCutValue       = (IsPrimitiveTopologyStrip(desc.primitiveTopology) ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED);
    stateDesc.PrimitiveTopologyType = GetPrimitiveToplogyType(desc.primitiveTopology);
    stateDesc.SampleMask            = desc.blend.sampleMask;
    stateDesc.NumRenderTargets      = numAttachments;
    stateDesc.SampleDesc.Count      = (renderPass != nullptr ? renderPass->GetSampleDesc().Count : 1);
    stateDesc.SampleDesc.Quality    = 0;

    /* Create native PSO */
    SetNative(device.CreateDXGraphicsPipelineState(stateDesc));

    /* Serialize graphics PSO */
    if (writer != nullptr)
    {
        /* Get cached blob from native PSO */
        ComPtr<ID3DBlob> cachedBlob;
        auto hr = GetNative()->GetCachedBlob(cachedBlob.GetAddressOf());
        DXThrowIfFailed(hr, "failed to retrieve cached blob from ID3D12PipelineState");

        /* Get serialized root signature blob */
        auto rootSignatureBlob = pipelineLayout.GetSerializedBlob();
        if (rootSignatureBlob == nullptr)
            DXThrowIfFailed(E_POINTER, "failed to retrieve serialized root signature blob from ID3D12RootSignature");

        /* Serialize entire PSO */
        SerializePSO(*writer, stateDesc, rootSignatureBlob, cachedBlob.Get());
    }
}

void D3D12GraphicsPSO::CreateNativePSOFromCache(
    D3D12Device&                    device,
    Serialization::Deserializer&    reader)
{
    /* Read graphics PSO descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};
    reader.ReadSegment(Serialization::D3D12Ident_GraphicsDesc, &stateDesc, sizeof(stateDesc));

    stateDesc.pRootSignature = GetRootSignature();

    /* Deserialize PSO from cache */
    std::vector<D3D12_INPUT_ELEMENT_DESC>   inputElements;
    std::vector<D3D12_SO_DECLARATION_ENTRY> soDeclEntries;
    std::vector<UINT>                       soBufferStrides;

    DeserializePSO(reader, stateDesc, inputElements, soDeclEntries, soBufferStrides);

    /* Create native PSO */
    SetNative(device.CreateDXGraphicsPipelineState(stateDesc));
}

// Returns the size (in bytes) for the static-state buffer with the specified number of viewports and scissor rectangles
static std::size_t GetStaticStateBufferSize(std::size_t numViewports, std::size_t numScissors)
{
    return (numViewports * sizeof(D3D12_VIEWPORT) + numScissors * sizeof(D3D12_RECT));
}

void D3D12GraphicsPSO::SerializePSO(
    Serialization::Serializer&                  writer,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC&   stateDesc,
    ID3DBlob*                                   rootSignatureBlob,
    ID3DBlob*                                   psoCacheBlob)
{
    /* Write graphics PSO identifier */
    writer.Begin(Serialization::D3D12Ident_GraphicsPSOIdent);
    writer.End();

    /* Write root signature blob */
    Serialization::D3D12WriteSegmentBlob(writer, Serialization::D3D12Ident_RootSignature, rootSignatureBlob);

    /* Write graphics PSO descriptor */
    writer.WriteSegment(Serialization::D3D12Ident_GraphicsDesc, &stateDesc, sizeof(stateDesc));

    /* Write PSO cache blob */
    Serialization::D3D12WriteSegmentBlob(writer, Serialization::D3D12Ident_CachedPSO, psoCacheBlob);

    /* Write shader entries */
    Serialization::D3D12WriteSegmentBytecode(writer, Serialization::D3D12Ident_VS, stateDesc.VS);
    Serialization::D3D12WriteSegmentBytecode(writer, Serialization::D3D12Ident_PS, stateDesc.PS);
    Serialization::D3D12WriteSegmentBytecode(writer, Serialization::D3D12Ident_DS, stateDesc.DS);
    Serialization::D3D12WriteSegmentBytecode(writer, Serialization::D3D12Ident_HS, stateDesc.HS);
    Serialization::D3D12WriteSegmentBytecode(writer, Serialization::D3D12Ident_GS, stateDesc.GS);

    /* Write input layout declarations */
    if (stateDesc.InputLayout.NumElements > 0)
    {
        /* Write input layout entries */
        writer.WriteSegment(
            Serialization::D3D12Ident_InputElements,
            stateDesc.InputLayout.pInputElementDescs,
            stateDesc.InputLayout.NumElements * sizeof(D3D12_INPUT_ELEMENT_DESC)
        );

        /* Write input semantic names */
        writer.Begin(Serialization::D3D12Ident_InputSemanticNames);
        {
            for (UINT i = 0; i < stateDesc.InputLayout.NumElements; ++i)
                writer.WriteCString(stateDesc.InputLayout.pInputElementDescs[i].SemanticName);
        }
        writer.End();
    }

    /* Write stream-output declarations */
    if (stateDesc.StreamOutput.NumEntries > 0)
    {
        /* Write stream-output entries */
        writer.WriteSegment(
            Serialization::D3D12Ident_SODeclEntries,
            stateDesc.StreamOutput.pSODeclaration,
            stateDesc.StreamOutput.NumEntries * sizeof(D3D12_SO_DECLARATION_ENTRY)
        );

        /* Write output semantic names */
        writer.Begin(Serialization::D3D12Ident_SOSemanticNames);
        {
            for (UINT i = 0; i < stateDesc.StreamOutput.NumEntries; ++i)
                writer.WriteCString(stateDesc.StreamOutput.pSODeclaration[i].SemanticName);
        }
        writer.End();
    }

    /* Write buffer strides */
    if (stateDesc.StreamOutput.NumStrides > 0)
    {
        writer.WriteSegment(
            Serialization::D3D12Ident_SOBufferStrides,
            stateDesc.StreamOutput.pBufferStrides,
            stateDesc.StreamOutput.NumStrides * sizeof(UINT)
        );
    }

    /* Write static state */
    writer.Begin(Serialization::D3D12Ident_StaticState);
    {
        writer.WriteTyped(primitiveTopology_);
        writer.WriteTyped(blendFactorEnabled_);
        writer.WriteTyped(blendFactor_);
        writer.WriteTyped(stencilRefEnabled_);
        writer.WriteTyped(stencilRef_);
        writer.WriteTyped(scissorEnabled_);
        writer.WriteTyped(numStaticViewports_);
        writer.WriteTyped(numStaticScissors_);

        if (numStaticViewports_ > 0 || numStaticScissors_ > 0)
        {
            const auto bufferSize = GetStaticStateBufferSize(numStaticViewports_, numStaticScissors_);
            writer.Write(staticStateBuffer_.get(), bufferSize);
        }
    }
    writer.End();
}

void D3D12GraphicsPSO::DeserializePSO(
    Serialization::Deserializer&                reader,
    D3D12_GRAPHICS_PIPELINE_STATE_DESC&         stateDesc,
    std::vector<D3D12_INPUT_ELEMENT_DESC>&      inputElements,
    std::vector<D3D12_SO_DECLARATION_ENTRY>&    soDeclEntries,
    std::vector<UINT>&                          soBufferStrides)
{
    /* Read PSO cache blob */
    Serialization::D3D12ReadSegmentBlob(reader, Serialization::D3D12Ident_CachedPSO, stateDesc.CachedPSO);

    /* Read shader byte codes */
    Serialization::D3D12ReadSegmentBytecode(reader, Serialization::D3D12Ident_VS, stateDesc.VS);
    Serialization::D3D12ReadSegmentBytecode(reader, Serialization::D3D12Ident_PS, stateDesc.PS);
    Serialization::D3D12ReadSegmentBytecode(reader, Serialization::D3D12Ident_DS, stateDesc.DS);
    Serialization::D3D12ReadSegmentBytecode(reader, Serialization::D3D12Ident_HS, stateDesc.HS);
    Serialization::D3D12ReadSegmentBytecode(reader, Serialization::D3D12Ident_GS, stateDesc.GS);

    /* Read input layout declarations */
    if (stateDesc.InputLayout.NumElements > 0)
    {
        inputElements.resize(stateDesc.InputLayout.NumElements);

        /* Read input layout entries */
        reader.ReadSegment(
            Serialization::D3D12Ident_InputElements,
            inputElements.data(),
            inputElements.size() * sizeof(D3D12_INPUT_ELEMENT_DESC)
        );

        /* Write input semantic names */
        reader.Begin(Serialization::D3D12Ident_InputSemanticNames);
        {
            for (UINT i = 0; i < stateDesc.InputLayout.NumElements; ++i)
                inputElements[i].SemanticName = reader.ReadCString();
        }
        reader.End();

        /* Patch descritpor field */
        stateDesc.InputLayout.pInputElementDescs = inputElements.data();
    }

    /* Read stream-output declarations */
    if (stateDesc.StreamOutput.NumEntries > 0)
    {
        soDeclEntries.resize(stateDesc.StreamOutput.NumEntries);

        /* Read stream-output entries */
        reader.ReadSegment(
            Serialization::D3D12Ident_SODeclEntries,
            soDeclEntries.data(),
            soDeclEntries.size() * sizeof(D3D12_SO_DECLARATION_ENTRY)
        );

        /* Write semantic names */
        reader.Begin(Serialization::D3D12Ident_SOSemanticNames);
        {
            for (UINT i = 0; i < stateDesc.StreamOutput.NumEntries; ++i)
                soDeclEntries[i].SemanticName = reader.ReadCString();
        }
        reader.End();

        /* Patch descritpor field */
        stateDesc.StreamOutput.pSODeclaration = soDeclEntries.data();
    }

    if (stateDesc.StreamOutput.NumStrides > 0)
    {
        soBufferStrides.resize(stateDesc.StreamOutput.NumStrides);

        /* Read buffer strides */
        reader.ReadSegment(
            Serialization::D3D12Ident_SOBufferStrides,
            soBufferStrides.data(),
            soBufferStrides.size() * sizeof(UINT)
        );

        /* Patch descriptor field */
        stateDesc.StreamOutput.pBufferStrides = soBufferStrides.data();
    }

    /* Write static state */
    reader.Begin(Serialization::D3D12Ident_StaticState);
    {
        reader.ReadTyped(primitiveTopology_);
        reader.ReadTyped(blendFactorEnabled_);
        reader.ReadTyped(blendFactor_);
        reader.ReadTyped(stencilRefEnabled_);
        reader.ReadTyped(stencilRef_);
        reader.ReadTyped(scissorEnabled_);
        reader.ReadTyped(numStaticViewports_);
        reader.ReadTyped(numStaticScissors_);

        if (numStaticViewports_ > 0 || numStaticScissors_ > 0)
        {
            const auto bufferSize = GetStaticStateBufferSize(numStaticViewports_, numStaticScissors_);
            staticStateBuffer_ = MakeUniqueArray<char>(bufferSize);
            reader.Read(staticStateBuffer_.get(), bufferSize);
        }
    }
    reader.End();
}

void D3D12GraphicsPSO::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    /* Allocate packed raw buffer */
    const auto bufferSize = GetStaticStateBufferSize(desc.viewports.size(), desc.scissors.size());
    staticStateBuffer_ = MakeUniqueArray<char>(bufferSize);

    ByteBufferIterator byteBufferIter { staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!desc.viewports.empty())
        BuildStaticViewports(desc.viewports.size(), desc.viewports.data(), byteBufferIter);

    /* Build static scissors in raw buffer */
    if (!desc.scissors.empty())
        BuildStaticScissors(desc.scissors.size(), desc.scissors.data(), byteBufferIter);
}

void D3D12GraphicsPSO::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
{
    /* Store number of viewports and validate limit */
    numStaticViewports_ = static_cast<UINT>(numViewports);

    if (numStaticViewports_ > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        throw std::invalid_argument(
            "too many viewports in graphics pipeline state (" + std::to_string(numStaticViewports_) +
            " specified, but limit is " + std::to_string(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE) + ")"
        );
    }

    /* Build <D3D12_VIEWPORT> entries */
    for (std::size_t i = 0; i < numViewports; ++i)
    {
        auto dst = byteBufferIter.Next<D3D12_VIEWPORT>();
        {
            dst->TopLeftX   = viewports[i].x;
            dst->TopLeftY   = viewports[i].y;
            dst->Width      = viewports[i].width;
            dst->Height     = viewports[i].height;
            dst->MinDepth   = viewports[i].minDepth;
            dst->MaxDepth   = viewports[i].maxDepth;
        }
    }
}

void D3D12GraphicsPSO::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
{
    /* Store number of scissors and validate limit */
    numStaticScissors_ = static_cast<UINT>(numScissors);

    if (numStaticScissors_ > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        throw std::invalid_argument(
            "too many viewports in graphics pipeline state (" + std::to_string(numStaticScissors_) +
            " specified, but limit is " + std::to_string(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE) + ")"
        );
    }

    /* Build <D3D12_RECT> entries */
    for (std::size_t i = 0; i < numScissors; ++i)
    {
        auto dst = byteBufferIter.Next<D3D12_RECT>();
        {
            dst->left   = static_cast<LONG>(scissors[i].x);
            dst->top    = static_cast<LONG>(scissors[i].y);
            dst->right  = static_cast<LONG>(scissors[i].x + scissors[i].width);
            dst->bottom = static_cast<LONG>(scissors[i].y + scissors[i].height);
        }
    }
}

void D3D12GraphicsPSO::SetStaticViewportsAndScissors(ID3D12GraphicsCommandList* commandList)
{
    if (staticStateBuffer_)
    {
        ByteBufferIterator byteBufferIter { staticStateBuffer_.get() };
        if (numStaticViewports_ > 0)
        {
            commandList->RSSetViewports(
                numStaticViewports_,
                byteBufferIter.Next<D3D12_VIEWPORT>(numStaticViewports_)
            );
        }
        if (numStaticScissors_ > 0)
        {
            commandList->RSSetScissorRects(
                numStaticScissors_,
                byteBufferIter.Next<D3D12_RECT>(numStaticScissors_)
            );
        }
    }
}


} // /namespace LLGL



// ================================================================================
