/*
 * D3D11GraphicsPSOBase.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_GRAPHICS_PSO_BASE_H
#define LLGL_D3D11_GRAPHICS_PSO_BASE_H


#include "D3D11PipelineState.h"
#include <LLGL/ForwardDecls.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>
#include <memory>


namespace LLGL
{


class D3D11ShaderProgram;
class D3D11StateManager;
class ByteBufferIterator;

class D3D11GraphicsPSOBase : public D3D11PipelineState
{

    public:

        // Binds the input layout, primitive topology, and all shader stages.
        virtual void Bind(D3D11StateManager& stateMngr) override;

    protected:

        D3D11GraphicsPSOBase(const GraphicsPipelineDescriptor& desc);

        void SetStaticViewportsAndScissors(D3D11StateManager& stateMngr);

        // Returns the primitive toplogy for the 'IASetPrimitiveTopology' function.
        inline D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
        {
            return primitiveTopology_;
        }

        // Returns the stencil reference value used for the 'OMSetDepthStencilState' function.
        inline UINT GetStencilRef() const
        {
            return stencilRef_;
        }

        // Returns true if the stencil reference value must be set independently of the PSO.
        inline bool IsStencilRefDynamic() const
        {
            return stencilRefDynamic_;
        }

        // Returns the pointer to an array of 4 floating-points for the blending factor for the 'OMSetBlendState' function.
        inline const FLOAT* GetBlendFactor() const
        {
            return blendFactor_;
        }

        // Returns true if the blend factor must be set independently of the PSO.
        inline bool IsBlendFactorDynamic() const
        {
            return blendFactorDynamic_;
        }

        // Returns the 32-bit sample mask for the 'OMSetBlendState' function.
        inline UINT GetSampleMask() const
        {
            return sampleMask_;
        }

    private:

        void StoreShaderObjects(const D3D11ShaderProgram& shaderProgramD3D);

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);

    private:

        ComPtr<ID3D11InputLayout>       inputLayout_;

        ComPtr<ID3D11VertexShader>      vs_;
        ComPtr<ID3D11HullShader>        hs_;
        ComPtr<ID3D11DomainShader>      ds_;
        ComPtr<ID3D11GeometryShader>    gs_;
        ComPtr<ID3D11PixelShader>       ps_;

        D3D11_PRIMITIVE_TOPOLOGY        primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        bool                            stencilRefDynamic_  = false;
        UINT                            stencilRef_         = 0;
        bool                            blendFactorDynamic_ = false;
        FLOAT                           blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        UINT                            sampleMask_         = std::numeric_limits<UINT>::max();

        std::unique_ptr<char[]>         staticStateBuffer_;
        UINT                            numStaticViewports_ = 0;
        UINT                            numStaticScissors_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
