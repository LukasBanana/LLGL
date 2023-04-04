/*
 * D3D11ComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMPUTE_PSO_H
#define LLGL_D3D11_COMPUTE_PSO_H


#include "D3D11PipelineState.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


struct ComputePipelineDescriptor;
class D3D11StateManager;

class D3D11ComputePSO final : public D3D11PipelineState
{

    public:

        D3D11ComputePSO(const ComputePipelineDescriptor& desc);

        void Bind(D3D11StateManager& stateMngr) override;

    private:

        ComPtr<ID3D11ComputeShader> cs_;

};


} // /namespace LLGL


#endif



// ================================================================================
