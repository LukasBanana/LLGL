/*
 * D3D11StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STATE_MANAGER_H
#define LLGL_D3D11_STATE_MANAGER_H


#include "../../DXCommon/ComPtr.h"
#include "../Shader/D3D11BuiltinShaderFactory.h"
#include "../Buffer/D3D11IntermediateBufferPool.h"
#include <LLGL/PipelineStateFlags.h>
#include <vector>
#include <cstdint>
#include "../Direct3D11.h"


namespace LLGL
{


class D3D11StateManager
{

    public:

        D3D11StateManager(ID3D11Device* device, ComPtr<ID3D11DeviceContext>& context);

        void SetViewports(std::uint32_t numViewports, const Viewport* viewportArray);
        void SetScissors(std::uint32_t numScissors, const Scissor* scissorArray);

        void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
        void SetInputLayout(ID3D11InputLayout* inputLayout);

        void SetVertexShader(ID3D11VertexShader* shader);
        void SetHullShader(ID3D11HullShader* shader);
        void SetDomainShader(ID3D11DomainShader* shader);
        void SetGeometryShader(ID3D11GeometryShader* shader);
        void SetPixelShader(ID3D11PixelShader* shader);
        void SetComputeShader(ID3D11ComputeShader* shader);

        void SetRasterizerState(ID3D11RasterizerState* rasterizerState);

        void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState);
        void SetDepthStencilState(ID3D11DepthStencilState* depthStencilState, UINT stencilRef);
        void SetStencilRef(UINT stencilRef);

        void SetBlendState(ID3D11BlendState* blendState, UINT sampleMask);
        void SetBlendState(ID3D11BlendState* blendState, const FLOAT* blendFactor, UINT sampleMask);
        void SetBlendFactor(const FLOAT* blendFactor);

        void SetConstantBuffers(
            UINT                    startSlot,
            UINT                    count,
            ID3D11Buffer* const*    buffers,
            long                    stageFlags
        );

        void SetConstantBuffersRange(
            UINT                    startSlot,
            UINT                    count,
            ID3D11Buffer* const*    buffers,
            const UINT*             firstConstants,
            const UINT*             numConstants,
            long                    stageFlags
        );

        void SetShaderResources(
            UINT                                startSlot,
            UINT                                count,
            ID3D11ShaderResourceView* const*    views,
            long                                stageFlags
        );

        void SetSamplers(
            UINT                        startSlot,
            UINT                        count,
            ID3D11SamplerState* const*  samplers,
            long                        stageFlags
        );

        void SetUnorderedAccessViews(
            UINT                                startSlot,
            UINT                                count,
            ID3D11UnorderedAccessView* const*   views,
            const UINT*                         initialCounts,
            long                                stageFlags
        );

        // Binds an intermediate constant buffer and updates its content with the specified data.
        void SetConstants(std::uint32_t slot, const void* data, std::uint16_t dataSize, long stageFlags);

        // Executes the specified builtin compute shader.
        void DispatchBuiltin(const D3D11BuiltinShader builtinShader, UINT numWorkGroupsX, UINT numWorkGroupsY, UINT numWorkGroupsZ);

        // Must be called in D3D11CommandBuffer::Begin
        void ResetIntermediateBufferPools();

        // Returns the ID3D11DeviceContext that this state manager is associated with.
        inline ID3D11DeviceContext* GetContext() const
        {
            return context_.Get();
        }

    private:

        struct D3DInputAssemblyState
        {
            D3D11_PRIMITIVE_TOPOLOGY    primitiveTopology   = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            ID3D11InputLayout*          inputLayout         = nullptr;
        };

        struct D3DShaderState
        {
            ID3D11VertexShader*     vs = nullptr;
            ID3D11HullShader*       hs = nullptr;
            ID3D11DomainShader*     ds = nullptr;
            ID3D11GeometryShader*   gs = nullptr;
            ID3D11PixelShader*      ps = nullptr;
            ID3D11ComputeShader*    cs = nullptr;
        };

        struct D3DRenderState
        {
            ID3D11RasterizerState*      rasterizerState     = nullptr;
            ID3D11DepthStencilState*    depthStencilState   = nullptr;
            UINT                        stencilRef          = 0;
            ID3D11BlendState*           blendState          = nullptr;
            FLOAT                       blendFactor[4]      = { -1.0f, -1.0f, -1.0f, -1.0f };
            UINT                        sampleMask          = 0xffffffff;
        };

    private:

        ComPtr<ID3D11DeviceContext>     context_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3D11DeviceContext1>    context1_;
        #endif

        D3D11IntermediateBufferPool     intermediateCbufferPool_;

        D3DInputAssemblyState           inputAssemblyState_;
        D3DShaderState                  shaderState_;
        D3DRenderState                  renderState_;

};


} // /namespace LLGL


#endif



// ================================================================================
