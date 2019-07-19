/*
 * D3D11ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_COMPUTE_PIPELINE_H
#define LLGL_D3D11_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


struct ComputePipelineDescriptor;
class D3D11StateManager;

class D3D11ComputePipeline final : public ComputePipeline
{

    public:

        D3D11ComputePipeline(const ComputePipelineDescriptor& desc);

        void Bind(D3D11StateManager& stateMngr);

    private:

        ComPtr<ID3D11ComputeShader> cs_;

};


} // /namespace LLGL


#endif



// ================================================================================
