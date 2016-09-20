/*
 * D3D11GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_GRAPHICS_PIPELINE_H__
#define __LLGL_D3D11_GRAPHICS_PIPELINE_H__


#include <LLGL/GraphicsPipeline.h>
#include "../../ComPtr.h"
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class D3D11RenderSystem;
class D3D11ShaderProgram;

class D3D11GraphicsPipeline : public GraphicsPipeline
{

    public:

        D3D11GraphicsPipeline(
            ID3D11Device* device,
            const GraphicsPipelineDescriptor& desc
        );

        void Bind(ID3D11DeviceContext* context);

        inline ID3D11VertexShader*      GetVS() const { return vs_.Get(); }
        inline ID3D11PixelShader*       GetPS() const { return ps_.Get(); }
        inline ID3D11HullShader*        GetHS() const { return hs_.Get(); }
        inline ID3D11DomainShader*      GetDS() const { return ds_.Get(); }
        inline ID3D11GeometryShader*    GetGS() const { return gs_.Get(); }
        inline ID3D11ComputeShader*     GetCS() const { return cs_.Get(); }

        inline ID3D11DepthStencilState* GetDepthStencilState() const
        {
            return depthStencilState_.Get();
        }

        inline ID3D11RasterizerState* GetRasterizerState() const
        {
            return rasterizerState_.Get();
        }

        inline ID3D11BlendState* GetBlendState() const
        {
            return blendState_.Get();
        }

        inline D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
        {
            return primitiveTopology_;
        }

    private:

        void GetShaderObjects(D3D11ShaderProgram& shaderProgramD3D);

        void CreateDepthStencilState(ID3D11Device* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc);
        void CreateBlendState(ID3D11Device* device, const BlendDescriptor& desc);

        ComPtr<ID3D11VertexShader>      vs_;
        ComPtr<ID3D11PixelShader>       ps_;
        ComPtr<ID3D11HullShader>        hs_;
        ComPtr<ID3D11DomainShader>      ds_;
        ComPtr<ID3D11GeometryShader>    gs_;
        ComPtr<ID3D11ComputeShader>     cs_;

        ComPtr<ID3D11DepthStencilState> depthStencilState_;
        ComPtr<ID3D11RasterizerState>   rasterizerState_;
        ComPtr<ID3D11BlendState>        blendState_;

        D3D11_PRIMITIVE_TOPOLOGY        primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        UINT                            stencilRef_         = 0;
        FLOAT                           blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        UINT                            sampleMask_         = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
