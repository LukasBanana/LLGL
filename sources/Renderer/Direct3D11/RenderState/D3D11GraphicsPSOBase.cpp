/*
 * D3D11GraphicsPSOBase.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11GraphicsPSOBase.h"
#include "D3D11StateManager.h"
#include "D3D11PipelineLayout.h"
#include "../D3D11Types.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Helper.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
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
    if (const auto* pipelineLayoutD3D = GetPipelineLayout())
        pipelineLayoutD3D->BindGraphicsStaticSamplers(stateMngr);
}


/*
 * ======= Protected: =======
 */

D3D11GraphicsPSOBase::D3D11GraphicsPSOBase(const GraphicsPipelineDescriptor& desc) :
    D3D11PipelineState { /*isGraphicsPSO:*/ true, desc.pipelineLayout }
{
    /* Validate pointers and get D3D shader objects */
    if (auto vertexShaderD3D = LLGL_CAST(const D3D11Shader*, desc.vertexShader))
        inputLayout_ = vertexShaderD3D->GetInputLayout();
    else
        throw std::invalid_argument("cannot create D3D graphics pipeline without vertex shader");

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
        ByteBufferIterator byteBufferIter{ staticStateBuffer_.get() };
        if (numStaticViewports_ > 0)
        {
            stateMngr.GetContext()->RSSetViewports(
                numStaticViewports_,
                byteBufferIter.Next<D3D11_VIEWPORT>(numStaticViewports_)
            );
        }
        if (numStaticScissors_ > 0)
        {
            stateMngr.GetContext()->RSSetScissorRects(
                numStaticScissors_,
                byteBufferIter.Next<D3D11_RECT>(numStaticScissors_)
            );
        }
    }
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPSOBase::GetD3DNativeShaders(const GraphicsPipelineDescriptor& desc)
{
    if (auto vs = desc.vertexShader        ) { vs_ = LLGL_CAST(D3D11Shader*, vs)->GetNative().vs; }
    if (auto hs = desc.tessControlShader   ) { hs_ = LLGL_CAST(D3D11Shader*, hs)->GetNative().hs; }
    if (auto ds = desc.tessEvaluationShader) { ds_ = LLGL_CAST(D3D11Shader*, ds)->GetNative().ds; }
    if (auto gs = desc.geometryShader      ) { gs_ = LLGL_CAST(D3D11Shader*, gs)->GetNative().gs; }
    if (auto ps = desc.fragmentShader      ) { ps_ = LLGL_CAST(D3D11Shader*, ps)->GetNative().ps; }
}

void D3D11GraphicsPSOBase::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    /* Allocate packed raw buffer */
    const std::size_t bufferSize =
    (
        desc.viewports.size() * sizeof(D3D11_VIEWPORT) +
        desc.scissors.size()  * sizeof(D3D11_RECT    )
    );
    staticStateBuffer_ = MakeUniqueArray<char>(bufferSize);

    ByteBufferIterator byteBufferIter{ staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!desc.viewports.empty())
        BuildStaticViewports(desc.viewports.size(), desc.viewports.data(), byteBufferIter);

    /* Build static scissors in raw buffer */
    if (!desc.scissors.empty())
        BuildStaticScissors(desc.scissors.size(), desc.scissors.data(), byteBufferIter);
}

void D3D11GraphicsPSOBase::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
{
    /* Store number of viewports and validate limit */
    numStaticViewports_ = static_cast<UINT>(numViewports);

    if (numStaticViewports_ > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        throw std::invalid_argument(
            "too many viewports in graphics pipeline state (" + std::to_string(numStaticViewports_) +
            " specified, but limit is " + std::to_string(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE) + ")"
        );
    }

    /* Build <D3D11_VIEWPORT> entries */
    for (std::size_t i = 0; i < numViewports; ++i)
    {
        auto dst = byteBufferIter.Next<D3D11_VIEWPORT>();
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

void D3D11GraphicsPSOBase::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
{
    /* Store number of scissors and validate limit */
    numStaticScissors_ = static_cast<UINT>(numScissors);

    if (numStaticScissors_ > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        throw std::invalid_argument(
            "too many viewports in graphics pipeline state (" + std::to_string(numStaticScissors_) +
            " specified, but limit is " + std::to_string(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE) + ")"
        );
    }

    /* Build <D3D11_RECT> entries */
    for (std::size_t i = 0; i < numScissors; ++i)
    {
        auto dst = byteBufferIter.Next<D3D11_RECT>();
        {
            dst->left   = static_cast<LONG>(scissors[i].x);
            dst->top    = static_cast<LONG>(scissors[i].y);
            dst->right  = static_cast<LONG>(scissors[i].x + scissors[i].width);
            dst->bottom = static_cast<LONG>(scissors[i].y + scissors[i].height);
        }
    }
}


} // /namespace LLGL



// ================================================================================
