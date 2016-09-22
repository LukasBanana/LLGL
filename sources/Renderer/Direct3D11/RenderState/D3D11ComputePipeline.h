/*
 * D3D11ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_COMPUTE_PIPELINE_H__
#define __LLGL_D3D11_COMPUTE_PIPELINE_H__


#include <LLGL/ComputePipeline.h>
#include "../../ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11ComputePipeline : public ComputePipeline
{

    public:

        D3D11ComputePipeline(const ComputePipelineDescriptor& desc);

        void Bind(ID3D11DeviceContext* context);

    private:

        ComPtr<ID3D11ComputeShader> cs_;

};


} // /namespace LLGL


#endif



// ================================================================================
